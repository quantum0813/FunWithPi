# FunWithPi

This is my final project for Intro to Parallel Processing. The goal of the project was to implement an algorithm using 2 different parallel frameworks. For this project I have implemented 2 different methods of calculating Pi (Monte Carlo and Chudnovsky) using 2 different parallel frameworks (OpenMP and MPI). 

### About

The Chudnovsky algorithm was published by the Chudnovsky brothers in 1989, and is a fast method for calculating Pi. It has been used to calculate up to 12.1 trillion digits of the number! Since the formula is an infinite series with no data dependencies, it can be considered embarassingly parallel. My Chudnovsky implementation uses OpenMP to speed up the calculation. The implementation also uses the GMP library along with OpenMP and MPI. Since we are dealing with very large numbers, the GMP library is needed. The GMP library has very good performance, and has been used in some of the record-breaking calculations of Pi. OpenMP is used to increase performance by running multiple threads, each calculating one iteration of the formula. In addition, dynamic scheduling is used to achieve load balancing.


The Monte Carlo method is a simple way to estimate the value of Pi by generating random points, and determining if they lie inside or outside of a unit circle. If the point is inside of the unit circle, it is counted as a "hit". The "hits" are then added up, and divided by the number of "simulations" or "iterations" performed. The resulting fraction is then multiplied by 4 to give you a rough estimate of Pi. My implementation of this method uses MPI to perform the calculations in parallel.

### Compiling

Compilation is simple. On Linux-based machines, the following commands can be executed:

```
mkdir build && cd build
cmake ..
make
```

### Usage

Using FunWithPi is simple! There are three required arguments: number of threads, number of iterations and precision in bytes.

```./FunWithPi nThreads nIters precisionInBytes [-c] [-o outFile]```

#### Required Arguments
---
**Number of threads**: Here you can specify the number of threads to use in computing Pi. There is no upper limit to this number, but it must be greater than 0.

**Number of iterations**: Here you can specify how many iterations of the formula to calculate. The more iterations, the more accurate the answer will be, but the longer the computation will take.

**Precision in bytes**: Here you can specify how precise you want the answer to be (i.e., how much of Pi you would like to calculate). This value is in bytes, NOT digits. For example, a precision of 100 bytes will give you 251 digits.

#### Optional Arguments
---
**Accuracy checking (-c)**: If this option is specified, the accuracy of the calculated digits of Pi will be compared against a file containing a million digits of Pi. For this to work, the file named *"pi_one_mil.txt"* must be placed in the same directory as the executable.

**Output redirection (-o filename)**: If this option is specified with a valid filename, the digits of Pi will be written to this location instead of standard output.
