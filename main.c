// SARE EKEN
// 22100011016
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#define MATRIX_MAX_SIZE 20 // Maksimum matris boyutu

// Global değişkenler
int originalMatrix[MATRIX_MAX_SIZE][MATRIX_MAX_SIZE];      // Orijinal matris (inputA.txt'den okunur)
int rightShiftedMatrix[MATRIX_MAX_SIZE][MATRIX_MAX_SIZE];  // Sağa kaydırma için geçici matris
int upShiftedMatrix[MATRIX_MAX_SIZE][MATRIX_MAX_SIZE];     // Yukarı kaydırılmış matris (inputB.txt)
int resultMatrix[MATRIX_MAX_SIZE][MATRIX_MAX_SIZE];        // Çarpım sonucu matris (outputC.txt)
pthread_mutex_t matrixMutex;                              // Mutex kilidi (paylaşılan kaynakları korur)
pthread_cond_t shiftCompletedCondition = PTHREAD_COND_INITIALIZER; // Koşul değişkeni
int tempSum = 0;                   // Geçici toplam değişkeni (çarpım işlemi için)
int isRightShiftCompleted = 0;                            // Sağa kaydırma tamamlandı mı? (1 = tamamlandı)
int matrixSize;// Matris boyutu (n x n)

// matrisi ekrana yazdırır
void print_matrix(const char *label, int matrix[MATRIX_MAX_SIZE][MATRIX_MAX_SIZE]) {
    printf("%s\n", label);
    for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < matrixSize; j++) {
            printf("%d ", matrix[i][j]); // Matris elemanını ekrana yaz
        }
        printf("\n");
    }
    printf("\n");
}

//dosyadan matris okur
void read_matrix(const char *filename, int matrix[MATRIX_MAX_SIZE][MATRIX_MAX_SIZE]) {
    FILE *file = fopen(filename, "r");
    if (!file) { // Dosya açılamazsa hata ver
        perror("Dosya acma hatasi");
        exit(EXIT_FAILURE);
    }

    char line[256];
    int rows = 0, cols = 0;

    while (fgets(line, sizeof(line), file)) {
        cols = 0;
        char *token = strtok(line, " \t\n");
        while (token != NULL) {
            matrix[rows][cols++] = atoi(token);
            token = strtok(NULL, " \t\n");
        }
        if (cols > MATRIX_MAX_SIZE) {
            fprintf(stderr, "Maksimum sÜtun boyutunu asiyor!\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }
        rows++;
        if (rows > MATRIX_MAX_SIZE) {
            fprintf(stderr, "Maksimum satir boyutunu asiyor!\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);

    if (rows != cols) {
        fprintf(stderr, "Kare matris gereklidir.Satır ve sütun sayısı esit degil.\n");
        exit(EXIT_FAILURE);
    }

    matrixSize = rows; // Kare matris boyutu
}

//matrisi dosyaya yazar
void write_matrix(const char *filename, int matrix[MATRIX_MAX_SIZE][MATRIX_MAX_SIZE]) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Dosya acma hatasi");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < matrixSize; j++) {
            fprintf(file, "%d ", matrix[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

//Sağa kaydırma işlemi
void *shift_right(void *arg) {
    int shift = *(int *)arg;

    for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < matrixSize; j++) {
            rightShiftedMatrix[i][(j + shift) % matrixSize] = originalMatrix[i][j];
        }
    }

    pthread_mutex_lock(&matrixMutex);
    isRightShiftCompleted = 1; // Sağa kaydırma tamamlandı
    pthread_cond_signal(&shiftCompletedCondition); // Yukarı kaydırmaya sinyal gönder
    pthread_mutex_unlock(&matrixMutex);

    pthread_exit(NULL);
}

// Yukarı kaydırma işlemi
void *shift_up(void *arg) {
    pthread_mutex_lock(&matrixMutex);
    while (!isRightShiftCompleted) { // Sağa kaydırmanın tamamlanmasını bekle
        pthread_cond_wait(&shiftCompletedCondition, &matrixMutex);
    }
    pthread_mutex_unlock(&matrixMutex);

    int shift = *(int *)arg;

    for (int j = 0; j < matrixSize; j++) {
        for (int i = 0; i < matrixSize; i++) {
            upShiftedMatrix[(i + matrixSize - shift) % matrixSize][j] = rightShiftedMatrix[i][j];
        }
    }

    pthread_exit(NULL);
}

// Matris çarpım işlemi
void *matrix_multiply(void *arg) {
    int *data = (int *)arg;
    int row = data[0]; // Satır indeksi
    int col = data[1]; // Sütun indeksi

    pthread_mutex_lock(&matrixMutex);
    tempSum = 0; // tempSum sıfırlanır

    for (int k = 0; k < matrixSize; k++) {
        tempSum += originalMatrix[row][k] * upShiftedMatrix[k][col];
    }

    resultMatrix[row][col] = tempSum;

    FILE *output = fopen("outputC.txt", "a");
    if (output == NULL) {
        perror("Dosya açma hatası");
        pthread_mutex_unlock(&matrixMutex);
        pthread_exit(NULL);
    }
    fprintf(output, "C[%d][%d] = %d\n", row, col, tempSum);
    fclose(output);

    pthread_mutex_unlock(&matrixMutex);

    free(data);
    pthread_exit(NULL);
}

int main() {
    FILE *output = fopen("outputC.txt", "w");
    if (output == NULL) {
        perror("Dosya açma hatası");
        exit(EXIT_FAILURE);
    }
    fclose(output);

    // Orijinal matrisi oku
    read_matrix("inputA.txt", originalMatrix);
    print_matrix("Input A:", originalMatrix);

    // Zaman bilgisine göre kaydırma değeri belirle
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int shift = tm.tm_sec % matrixSize;

    printf("Şu anki zaman : %02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
    printf("Shift degeri : %d birim\n\n", shift);

    // Sağa ve yukarı kaydırma işlemleri için thread'ler oluştur
    pthread_t thread1, thread2;
    pthread_create(&thread1, NULL, shift_right, &shift);
    pthread_create(&thread2, NULL, shift_up, &shift);

    // Thread'lerin tamamlanmasını bekle
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // Sağa kaydırılmış matrisi yazdır
    print_matrix("-----------------Saga Kaydirilmis Matris------------------", rightShiftedMatrix);

    // Yukarı kaydırılmış matrisi yazdır ve dosyaya kaydet
    print_matrix("----------Yukari Kaydirilmis Matris (Input B)-------------", upShiftedMatrix);
    write_matrix("inputB.txt", upShiftedMatrix);

    // Matris çarpımı için thread'ler oluştur
    pthread_t threads[MATRIX_MAX_SIZE * MATRIX_MAX_SIZE];
    pthread_mutex_init(&matrixMutex, NULL);

    int threadCount = 0;
    for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < matrixSize; j++) {
            int *data = (int *)malloc(2 * sizeof(int));
            data[0] = i;
            data[1] = j;
            pthread_create(&threads[threadCount++], NULL, matrix_multiply, data);
        }
    }

    // Tüm çarpım thread'lerinin tamamlanmasını bekle
    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&matrixMutex);
    pthread_cond_destroy(&shiftCompletedCondition);

    // Sonuç matrisini yazdır
    print_matrix("--------------------Sonuc Matrisi(outputC)------------------", resultMatrix);
    write_matrix("outputC.txt", resultMatrix); // Matris formatinda sonuÁ yaziliyor


    return 0;
}
