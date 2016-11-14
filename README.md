# FunWithPi

This is my final project for Intro to Parallel Processing. The goal of the project was to implement an algorithm using 2 different parallel frameworks. For this project I am using OpenMP and MPI.

### About

For this project, I have chosen to implement the Chudnovsky algorithm. The Chudnovsky algorithm was published by the Chudnovsky brothers in 1989, and is a fast method for calculating Pi. It has been used to calculate up to 12.1 trillion digits of the number! Since the formula is an infinite series with no data dependencies, it can be considered embarassingly parallel. 

My implementation uses the GMP library along with OpenMP and MPI. Since we are dealing with very large numbers, the GMP library is needed to deal with them. The GMP library has very good performance, and has been used in some of the record-breaking calculations of Pi. OpenMP is used to increase performance by running multiple threads, each calculating one iteration of the formula. In addition, dynamic scheduling is used to achieve load balancing.

### Compiling

Compilation is simple. On Linux-based machines, the following commands can be executed:

```
mkdir build && cd build
cmake ..
make
```
