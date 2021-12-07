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

    // Change rank to check validity of transfers 
    // if (comm_rank == 1) {
    //     printf("Process #%d contains:\n", comm_rank);
    //     for (int i = 0; i < pointsNum * dims; i++) {
    //         if(i%dims == 0)printf("\n");
    //         printf("%f ", points[i]);
    //     }
    //     printf("\n");
    // }

    // Select anc broadcast pivot.
    bcast_pivot(&proc, pivot, points);

    // Uncomment to test pivot transfer
    if (comm_rank == 2) {
        printf("P#%d GMTPS VALE TO ARISTERO LAY UP:\n", comm_rank);
        for (int i = 0; i < dims; i++) {
            pivot[i] = pivot[i];
			printf("pivot[%d] = %.3f\n", i, pivot[i]);
		} 
        printf("\n");
    }

    //Calculate distances from pivot
    float *distances = (float *) malloc(pointsNum * sizeof(float));
    for (int i = 0; i < pointsNum; i++) {
        distances[i] = calculateDistanceArray(points, dims * i, pivot, dims);
    }

    for (int i = 0; i < pointsNum; i++) {
        printf("D%d,%d:%f ", comm_rank, i, distances[i]);
    }
    printf("\n");

    // Uncomment for distance array of 0 
    // Should be 0 at pivot index
    // First 7 points should always have the same value
    // if (comm_rank == 0) {
    //     printf("Dist array for %d is:\n", comm_rank);
    //     for (int i = 0; i < pointsNum; i++) {
	// 		printf("distances[%d] = %1.3f\n", i, distances[i]);
	// 	} 
    //     printf("\n");
    // }

    // Initialize the array that will keep all the distances for the master to find the median.
    // It only needs to be initialized in the case of the master, to conserve memory.
    float *dist_arr = NULL;
    if (comm_rank == 0) {
        dist_arr = malloc(pointsNum * comm_size * sizeof(float));
    }
    MPI_Gather(distances, pointsNum, MPI_FLOAT, dist_arr, pointsNum, MPI_FLOAT, 0, MPI_COMM_WORLD);
    
    if (comm_rank == 0) {
        // printf("\nProcess #0 has gathered the distances:\n");
        // for (int i = 0; i < pointsNum * comm_size; i++) {
        //     printf("%f ", dist_arr[i]);
        // }
        // printf("\n");

        median = quickselect(dist_arr, pointsNum*comm_size-1);
        printf("\nMedian distance is %f\n", median);
    }
    // Broadcast median.
    MPI_Bcast(&median, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    printf("Process #%d says: median = %f\n", comm_rank, median);





	MPI_Finalize();
	return 0;
}
