#include <stdlib.h>
#include <iostream>
#include <random>
#include <chrono>
#include <iomanip>
#include <mpi/mpi.h>

// Create our random number seed, and initialize it to the "time since the epoch"
unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
// For the Monte Carlo simulation, we need to generate numbers in the range [0, 1]
std::uniform_real_distribution<double> dist(0, 1);
// We are using a 64-bit Mersenne Twister engine to actually generate the numbers.
std::mt19937_64 rng(seed);

// Define some constants
const int DEFAULT_NUM_ITERATIONS = 1000;
const unsigned long long int MAX_ULL_VAL = std::numeric_limits<unsigned long long>::max();
const int COORDINATOR_RANK = 0;

/**
 * This function performs a simple test to see if a point lies inside of the unit circle.
 * @param x The x-coordinate
 * @param y The y-coordinate
 * @return bool True if the point lies inside or on the circle, false otherwise
 */
bool insideUnitCircle(double x, double y) {
    return ((x * x) + (y * y)) <= 1;
}

int main(int argc, char * argv[]) {
    int commSize;
    int numWorkers = 0;
    int myRank;
    double startTime = 0.0;

    unsigned long long numIterations = 0;
    unsigned long long numHits = 0;

    // Initialize MPI, and get the size of the communicator, and the current process's rank
    // It is important to note that the actual of workers is n - 1. One process is dedicated
    // to performing coordination tasks.
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &commSize);
    numWorkers = commSize - 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

    if (myRank == 0) {
        if (argc < 2) {
            std::cout << "You must provide at least one argument." << std::endl;
            std::cout << "\tUsage: " << argv[0] << " numIterations" << std::endl;
            return 0;
        }

        if (argv[1][0] == '-') {
            std::cout << "Number of iterations must be a positive number." << std::endl;
            std::cout << "Setting the number of iterations to the default value of " << DEFAULT_NUM_ITERATIONS << "." << std::endl;
            numIterations = DEFAULT_NUM_ITERATIONS;
        } else {
            numIterations = strtoull(argv[1], &argv[1], 10);
        }

        if (errno == ERANGE) {
            std::cout << "Number of iterations must be in the range [1, " << MAX_ULL_VAL << "]." << std::endl;
            std::cout << "Setting the number of iterations to the max value of " << MAX_ULL_VAL << "." << std::endl;
        }

        // COORDINATION
        /*
         * It is likely that we will have more iterations to perform than there are processors
         * available, thus we must split things up into blocks.
         * The method used to split things up is very simple:
         * The number of iterations are divided up evenly among the processors in the communicator.
         * Any leftover iterations are assigned to the last processor in the communicator.
         * Since the computations DO NOT get increasingly more complex, dynamic scheduling is not
         * needed. Each process will have the same amount of work to do, with the exception of the
         * last one, which may have a little additional work, however this is marginal at best.
         */
        startTime = MPI_Wtime();
        unsigned long long iterationsPerThread = 0;
        unsigned long long remainingIterations = 0;

        iterationsPerThread = numIterations / numWorkers;
        if ((numIterations % numWorkers) != 0)
            remainingIterations = numIterations % numWorkers;

        // Send each process (except coordinator) the number of computations it must perform.
        for (int i = 1; i < commSize; i++) {
            MPI_Request req;
            if (i == (commSize - 2))
                iterationsPerThread += remainingIterations;
            /*
             * Messages are sent with the non-blocking version of MPI_Send because we want each process to
             * begin its work without waiting for a process to call MPI_Receive.
             */
            MPI_Isend(&iterationsPerThread, 1, MPI_UNSIGNED_LONG_LONG, i, 0, MPI_COMM_WORLD, &req);
        }

        /*
         * The coordinator receives the "hits" from each process (in no particular order), and adds
         * them to the total number of hits.
         */
        unsigned long long totalHits = 0;
        for (int i = 0; i < numWorkers; i++) {
            unsigned long long hits;
            MPI_Recv(&hits, 1, MPI_UNSIGNED_LONG_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            totalHits += hits;
        }

        // To finish things up we divide the total hits by the number of iterations, multiply by 4,
        // and voila - an approximation of Pi!
        double ratio = (double)totalHits / (double)numIterations;
        double pi = ratio * 4;
        double timeTaken = MPI_Wtime() - startTime;
        std::cout << "Calculation took " << std::setprecision(10) << timeTaken << " seconds" << std::endl;
        std::cout << "Pi is approximately equal to " << std::setprecision(15) << pi << std::endl;
    } else {
        /*
         * Everything here is executed in parallel on each process that is not the coordinator.
         *
         * A process receives its workload from the coordinator. Which workload it receives does
         * not make a difference in this case since there is no data dependency. Each process is just
         * calculating a bunch of random numbers, and incrementing a counter for each point that lies
         * inside of the unit circle.
         */
        MPI_Request req;
        unsigned long long numIterations;
        MPI_Recv(&numIterations, 1, MPI_UNSIGNED_LONG_LONG, COORDINATOR_RANK, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        unsigned long long hits = 0;
        for (unsigned long long i = 0; i < numIterations; i++) {
            if (insideUnitCircle(dist(rng), dist(rng)))
                hits++;
            // Reseed the random number generator with the current seed + i after each iteration of the loop
            rng.seed(seed + i);
        }
        // Send the number of "hits" back to the coordinator.
        MPI_Send(&hits, 1, MPI_UNSIGNED_LONG_LONG, COORDINATOR_RANK, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}