/**
 * @file: mpi_a.c
 * ******************** 
 * @authors: Antonios Antoniou, Polydoros Giannouris
 * @emails: aantonii@ece.auth.gr, polydoros@ece.auth.gr
 * ********************
 * @description: p processes, controlled by MPI, possess N/p points each. The master
 * process gives them a pivot point, out of the ones it owns. The processes calculate
 * the distance of each of their points from the pivot and inform the master. Then
 * the first p/2 processes are given the points that are closer to the point than the
 * median distance, which means the rest of the p/2 processes get the rest of those points.
 * NOTE: for each of the processes, with ID t > 0, the process "t-1" must have points that are
 * closer to the pivot, the process "t+1" must have points that are further from the pivot and so on. 
 * The points of the process itself need not be sorted.
 * ********************
 * 2021 Aristotle University Thessaloniki
 * Parallel and Distributed Systems - Electrical and Computer Engineering
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

#include "headers/process.h"
#include "headers/helpers.h"
#include "headers/mpihelp.h"


int main(int argc, char **argv) {
	int comm_size, comm_rank;
    MPI_Status *mpi_stat101;
    MPI_Request *mpi_req101;
    srand((unsigned) time(NULL));

	// --------------- START OF TESTING MPI --------------- //
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);

    long *info = (long *) calloc(2, sizeof(long));
    long dims, pointsNum;
    float median;
    FILE *file;
    if (comm_rank == 0) {
        file = fopen("data/mnist.bin", "rb");
    }

    // Assign the info[] values to new variables to make the code more coherent.
    bcast_dims_points(file, info, comm_rank, comm_size);
    dims = info[0];
    pointsNum = info[1];

    // Make a new process struct, to pass the most important values to functions.
    process proc = {comm_size, comm_rank, dims, pointsNum, mpi_stat101};

    float *points = malloc(dims * pointsNum * sizeof(float));
    float *pivot = malloc(dims * sizeof(float));
    MPI_Barrier(MPI_COMM_WORLD);

    // Split the data from the binary file into processes.
    split_into_processes(file, &proc, points);

    // Select anc broadcast pivot.
    bcast_pivot(&proc, pivot, points);

    //Calculate distances from pivot
    float *distances = (float *) malloc(pointsNum * sizeof(float));
    for (int i = 0; i < pointsNum; i++) {
        distances[i] = calculateDistanceArray(points, dims * i, pivot, dims);
    }

    // Initialize the array that will keep all the distances for the master to find the median.
    // It only needs to be initialized in the case of the master, to conserve memory.
    float *dist_arr = NULL;
    if (comm_rank == 0) {
        dist_arr = malloc(pointsNum * comm_size * sizeof(float));
    }
    MPI_Gather(distances, pointsNum, MPI_FLOAT, dist_arr, pointsNum, MPI_FLOAT, 0, MPI_COMM_WORLD);
    
    if (comm_rank == 0) {
        median = quickselect(dist_arr, pointsNum*comm_size-1);
        printf("\nMedian distance is %f\n", median);
    }

    // Broadcast median.
    MPI_Bcast(&median, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    // The table each process uses to see whether a point it possesses has a distance
    // from the pivot point that is larger than the median.
    int *isLargerThanMedian = (int *) malloc(pointsNum * sizeof(int));
    
    // The integer that is to be gathered by the master from each process.
    long largerThanMedian = 0;
    long smallerThanMedian = 0;

    // The array that stores the largerThanMedian values.
    long *largerForEach = NULL;
    if (comm_rank == 0) {
        largerForEach = (long *) calloc(comm_size, sizeof(long));
    }

    /**
     * isLargerThanMedian:
     * 1 if distance is larger than median,
     * 0 if distance is equal to median,
     * -1 if distamce is less than median.
     */
    for (long i = 0; i < pointsNum; i++) {
        if (distances[i] > median) {
            isLargerThanMedian[i] = 1;
            largerThanMedian++;
        } 
        else if (distances[i] == median) {
            isLargerThanMedian[i] = 0;
        }
        else {
            isLargerThanMedian[i] = -1;
            smallerThanMedian++;
        }
    }
    MPI_Gather(&largerThanMedian, 1, MPI_LONG, largerForEach, 1, MPI_LONG, 0, MPI_COMM_WORLD);

    // if (comm_rank == 0) {
    //     printf("Process #0 has gathered how many distances are larger than the median for each process.\n");
    //     for (int i = 0; i < comm_size; i++) {
    //         printf("%ld ", largerForEach[i]);
    //     }
    //     printf("\n");
    // }

    if (comm_rank == 4) {
        printf("Distances before sortByMedian\n");
        for (int i = 0; i < pointsNum; i++) {
            printf("%f ", distances[i]);
        }

        printf("\nPoints before sortByMedian\n");
        for (int i = 0; i < pointsNum; i++) {
            for (int j = 700; j < 715; j++) {
                printf("%f ", points[dims * i + j]);
            }
            printf("\n");
        }
    }

    sortByMedian(distances, points, median, pointsNum, dims);

    MPI_Barrier(MPI_COMM_WORLD);

    if (comm_rank == 4) {
        printf("\nAfter sortByMedian\n");
        for (int i = 0; i < pointsNum; i++) {
            printf("%f ", distances[i]);
        }

        printf("\nPoints after sortByMedian\n");
        for (int i = 0; i < pointsNum; i++) {
            for (int j = 700; j < 715; j++) {
                printf("%f ", points[dims * i + j]);
            }
            printf("\n");
        }    
    }





	MPI_Finalize();
	return 0;
}
