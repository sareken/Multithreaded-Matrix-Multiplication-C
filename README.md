
#  Multithreaded Matrix Multiplication in C

This project demonstrates **matrix shifting and multiplication using multithreading (pthreads)** in C. It reads a square matrix from a file, performs **dynamic right and upward shifting based on the current system time**, and multiplies the resulting matrices using parallel threads.

---

## Features

 Reads matrix `A` from `inputA.txt`  
 Dynamically computes shift value using current system second  
 Performs:
-  **Right shift** (horizontal) on matrix A
-  **Up shift** (vertical) on the result  
 Writes the up-shifted matrix to `inputB.txt`  
 Multiplies matrix A and shifted matrix B using multithreading  
 Writes multiplication steps and final result to `outputC.txt`

---

## File Structure

| File            | Description                                  |
|-----------------|----------------------------------------------|
| `main.c`        | Source code implementing all operations      |
| `inputA.txt`    | Input matrix A                               |
| `inputB.txt`    | Generated matrix B (right + up shifted A)    |
| `outputC.txt`   | Output matrix C and multiplication steps     |

---

##  Compile & Run

```bash
gcc -pthread main.c -o matrix_project
./matrix_project
