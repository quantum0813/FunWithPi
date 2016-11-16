#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include <gmp.h>
#include <omp.h>

const bool ENABLE_DEBUG = true;

/*
 * Calculates the factorial of a number
 * The GMP library has no built in factorial function, so one is defined
 * here for convenience
 *
 * @param ret A placeholder for the result
 * @param n The number to take the factorial of
 * @return void
 */
inline void mpz_fact(mpz_t ret, mpz_t n) {
    int cmp;
    mpz_t result, m;
    mpz_init(result);

    mpz_set_ui(result, 1);
    if (mpz_cmp_ui(n, 2u) < 0) {
        mpz_set(ret, result);
        return;
    }

    mpz_init_set(m, n);
    while (mpz_sgn(m)) {
        mpz_mul(result, result, m);
        mpz_sub_ui(m, m, 1);
    }

    mpz_set(ret, result);
}

/**
 * Calculates a single iteration of the Chudnovsky formula
 *
 * @param rop The "return operator". This is where the result of the calculation is stored.
 * @param precision The precision to calculate (in bytes)
 * @param i The iteration value to calculate. Must be in the range [0, inf).
 * @return void
 */
void chudnovsky(mpf_t rop, unsigned long precision, unsigned long i) {
    // The Chudnovsky formula is the following:

    /*
     * Sum from k = 0 to infinity:
     *
     *  1               (-1)^k * (6k)! * (545140134k + 13591409)
     * --    =  12 *  Σ  ----------------------------------------
     * pi               (3k)! * (k!)^3 * (640320^3)^k + (1/2)
     */
    unsigned long k1;
    mpz_t k2;
    mpf_t sum, fraction;
    mpz_t numerator, denominatorPart1;
    mpf_t numeratorF, denominatorPart1F;
    mpz_t tokenNeg1PowK, token1359, token5451, token6403Pow3, token6403;
    mpf_t token6403NegSqrt;

    k1 = i;
    // Initialize and set the value of "k2" to the value of "i"
    mpz_init_set_ui(k2, i);

    // Initialize the value of "sum" to "precision"
    mpf_init2(sum, precision);
    // Initialize the value of "fraction" to "precision"
    mpf_init2(fraction, precision);

    // Initialize the value of "numerator" to 0
    mpz_init(numerator);
    // Initialize the value of "numeratorF" to "precision"
    mpf_init2(numeratorF, precision);
    // Initialize the value of "denominatorPart1" to 0
    mpz_init(denominatorPart1);
    // Initialize the value of "denominatorPart1F" to "precision"
    mpf_init2(denominatorPart1F, precision);

    // Initialize and set the value of "tokenNeg1PowK" to either 1 or -1 based on parity of i
    mpz_init_set_si(tokenNeg1PowK, (i % 2 == 0) ? 1 : -1);
    // Initialize and set the value of "token1359" to 1,3591,409
    mpz_init_set_ui(token1359, 13591409u);
    // Initialize and set the value of "token5451" to 545,140,134
    mpz_init_set_ui(token5451, 545140134u);
    // Initialize and set the value of "token6403Pow3" to the value
    // 26,2537,412,640,768,000. The value is passed in as a string, and
    // then parsed as a base 10 integer value.
    mpz_init_set_str(token6403Pow3, "262537412640768000", 10);
    // Initialize and set the value of "token6403" to 640,320
    mpz_init_set_ui(token6403, 640320u);

    // Initialize the value of "token6403NegSqrt" to "precision"
    mpf_init2(token6403NegSqrt, precision);
    // Set the value of "token6403NegSqrt" to 640,320
    mpf_set_ui(token6403NegSqrt, 640320u);
    // Take the square root of "token6403NegSqrt" and place the result back in "token6403NegSqrt"
    mpf_sqrt(token6403NegSqrt, token6403NegSqrt);
    // Divide 1 by "token6403NegSqrt", and place the result back in "token6403NegSqrt"
    mpf_ui_div(token6403NegSqrt, 1u, token6403NegSqrt);

    mpz_t kMul6Fact, kMul3Fact, kFactCube;

    // Initialize the value of "kMul6Fact" to 0
    mpz_init(kMul6Fact);
    // Initialize the value of "kMul3Fact" to 0
    mpz_init(kMul3Fact);
    // Initialize the value of "kFactCube" to 0
    mpz_init(kFactCube);

    // Multiply "k2" by 6, and place the result in "kMul6Fact"
    mpz_mul_ui(kMul6Fact, k2, 6u);
    // Take the factorial of the value "kMul6Fact", and place the result back in "kMul6Fact"
    mpz_fact(kMul6Fact, kMul6Fact);

    // Multiply "k2" by 3, and place the result in "kMul3Fact"
    mpz_mul_ui(kMul3Fact, k2, 3u);
    // Take the factorial of the value "kMul3Fact", and place the result back in "kMul3Fact"
    mpz_fact(kMul3Fact, kMul3Fact);

    // Take the factorial of the value "k2", and place the result in "kFactCube"
    mpz_fact(kFactCube, k2);
    // Raise the value of "kFactCube" to the third power, and place the result back in "kFactCube"
    mpz_pow_ui(kFactCube, kFactCube, 3u);

    // Numerator
    // Multiply "token5451" by "k2", and place the result in "numerator"
    mpz_mul(numerator, token5451, k2);
    // Add the values "numerator" and "token1359", and place the result back in "numerator"
    mpz_add(numerator, numerator, token1359);

    // Multiply "numerator" by "kMul6Fact", and place the result back in "numerator"
    mpz_mul(numerator, numerator, kMul6Fact);

    // Multiply "numerator" by "tokenNeg1PowK", and place the result back in "numerator"
    mpz_mul(numerator, numerator, tokenNeg1PowK);

    // Denominator
    // Raise "token6403Pow3" to the "k1" power, and place the result in "denominatorPart1"
    mpz_pow_ui(denominatorPart1, token6403Pow3, k1);

    // Multiply "denominatorPart1" by "token6403", and place the result back in "denominatorPart1"
    mpz_mul(denominatorPart1, denominatorPart1, token6403);

    // Multiply "denominatorPart1" by "kFactCube", and place the result back in "denominatorPart1"
    mpz_mul(denominatorPart1, denominatorPart1, kFactCube);

    // Multiply "denominatorPart1" by "kMul3Fact", and place the result back in "denominatorPart1"
    mpz_mul(denominatorPart1, denominatorPart1, kMul3Fact);

    // Large division
    // Set the value of "numeratorF" to "numerator"
    mpf_set_z(numeratorF, numerator);
    // Set the value of "denominatorPart1F" to "denominatorPart1"
    mpf_set_z(denominatorPart1F, denominatorPart1);

    // Do the large division
    // Divide "numeratorF" by "denominatorPart1F", and place the result in "fraction"
    mpf_div(fraction, numeratorF, denominatorPart1F);

    // Multiply "fraction" by "token6403NegSqrt", and place the result back in "fraction"
    mpf_mul(fraction, fraction, token6403NegSqrt);

    // Update the global sum
    // Add the values "sum" and "fraction", and place the result back in "sum"
    mpf_add(sum, sum, fraction);

    // Set the value of "rop" to "sum"
    mpf_set(rop, sum);
}

/**
 * This is the functions that performs the calculation of pi using the given number of threads,
 * iterations and precision.
 *
 * @param pi A pointer to the variable where the result should be stored
 * @param numThreads The number of threads to use in the calculation
 * @param numIterations The number of iterations of the formula to calculate
 * @param precision The precision of pi to calculate (in bytes)
 * @return void
 */
void calculatePi(mpf_t * pi, int numThreads, unsigned long numIterations, unsigned long precision) {
    /**
     * Here we use OpenMP to help speed up the computation. The algorithm used here is embarassingly
     * parallel, thus it lends itself well to parallelism.
     * In this case, dynamic scheduling is used to achieve load balancing. Since, each iteration gets
     * increasingly larger, we suffer from poor performance when using static scheduling.
     */
    #pragma omp parallel for schedule(dynamic) num_threads(numThreads)
    for (int i = 0; i < numIterations; i++) {
        mpf_t sum;
        mpf_init2(sum, precision);
        chudnovsky(sum, precision, i);
        /*
         * Because of the use of dynamic scheduling, race conditions can occur when
         * adding to the global sum, thus we must declare a critical section below.
         */
        #pragma omp critical
        mpf_add(*pi, *pi, sum);
    }
    /*
     * A barrier is placed here so that we can wait for all threads to complete their
     * computations before we perform computations that deal with the global sum.
     */
    #pragma omp barrier

    // The global sum is multiplied by 12
    // Multiply the value of "pi" by 12, and place the result back in "pi"
    mpf_mul_ui(*pi, *pi, 12u);
    // Lastly we must take the inverse of the final result, since the formula calculates
    // the inverse of pi.
    mpf_ui_div(*pi, 1, *pi);
}

int main(int argc, char * argv[]) {
    unsigned long threadCount;
    unsigned long iterCount;
    unsigned long precision;
    char * outFile = NULL;
    bool checkAccuracy = false;

    static char usage[] = "nThreads nIters precisionInBytes [-c] [-o outFile]";

    if (argc < 4) {
        fprintf(stderr, "You must provide at least 3 arguments:\n");
        fprintf(stderr, "\tUsage: %s %s\n", argv[0], usage);
        return -1;
    }

    threadCount = atoi(argv[1]);
    iterCount = atoi(argv[2]);
    precision = 8 * atoi(argv[3]);

    int c;
    while ((c = getopt(argc, argv, "co:")) != -1) {
        switch (c) {
            case 'c':
                checkAccuracy = true;
                break;
            case 'o':
                outFile = optarg;
                break;
        }
    }

    mpf_t pi;
    // Initialize the value of "pi" to "precision"
    mpf_init2(pi, precision);

    double startTime = omp_get_wtime();
    calculatePi(&pi, threadCount, iterCount, precision);
    double totalTime = omp_get_wtime() - startTime;

    printf("Computation took %.5f seconds\n", totalTime);

    char * piStr;
    gmp_asprintf(&piStr, "%.Ff", pi);

    if (outFile != NULL) {
        FILE * fp;
        fp = fopen(outFile, "w");
        if (fp != NULL) {
            fprintf(fp, "%s", piStr);
            fclose(fp);
            unsigned int significantDigits = strlen(piStr) - 2;
            printf("Num digits: %u\n", significantDigits);
        } else {
            printf("Error opening specified file, defaulting to stdout.\n");
            unsigned int significantDigits = strlen(piStr) - 2;
            printf("Num digits: %u\n%s\n", significantDigits, piStr);
        }
    } else {
        unsigned int significantDigits = strlen(piStr) - 2;
        printf("Num digits: %u\n%s\n", significantDigits, piStr);
    }

    if (checkAccuracy) {
        printf("\nChecking accuracy of calculation against 1 million digits of Pi...\n");
        FILE * refFile = fopen("pi_one_mil.txt", "r");

        if (refFile == NULL) {
            printf("Cannot find reference file \"pi_one_mil.txt\". Make sure this file is placed in the same directory as the executable.\n");
        } else {
            int curPos = 0;
            for (int i = 0; i < strlen(piStr); i++) {
                curPos = i;
                if ((int)piStr[i] != fgetc(refFile))
                    break;
            }
            fclose(refFile);

            int accuracy = curPos - 1;
            int digitsCalculated = strlen(piStr) - 2;
            printf("Calculation is accurate to %i digits (%i%% accuracy).\n", accuracy, (int)(round(100 * ((float)accuracy / digitsCalculated))));
        }
    }

    return 0;
}