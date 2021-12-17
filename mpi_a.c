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

    // Check if processes are a power of 2
    if(ceil(log2(comm_size)) != floor(log2(comm_size))){
        printf("Processes given(%d) are not a power of 2.\n", comm_size);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

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

    float *points = (float *) malloc(dims * pointsNum * sizeof(float));
    float *pivot = (float *) malloc(dims * sizeof(float));

    // Make a new process struct, to pass the most important values to functions.
    process proc = {comm_size, comm_rank, dims, pointsNum, pivot, mpi_stat101};

    MPI_Barrier(MPI_COMM_WORLD);

    // Split the data from the binary file into processes.
    split_into_processes(file, &proc, points);

    // Select and broadcast pivot.
    bcast_pivot(&proc, pivot, points);
    for(int i = 0; i < dims; i++) {
        proc.pivot[i] = pivot[i];
    }

    //Calculate distances from pivot
    float *distances = (float *) malloc(pointsNum * sizeof(float));
    for (int i = 0; i < pointsNum; i++) {
        distances[i] = calculateDistanceArray(points, dims * i, pivot, dims);
    }


//////////////////// From this point on everything must be integrated in the recursive function //////////////////////////////////////


    // Initialize the array that will keep all the distances for the master to find the median.
    // It only needs to be initialized in the case of the master, to conserve memory.
    float *dist_arr = NULL;
    if (comm_rank == 0) {
        dist_arr = malloc(pointsNum * comm_size * sizeof(float));
    }
    MPI_Gather(distances, pointsNum, MPI_FLOAT, dist_arr, pointsNum, MPI_FLOAT, 0, MPI_COMM_WORLD);
    
    if (comm_rank == 0) {
        printf("\n All distances: \n");
        for(int i = 0 ; i < pointsNum * comm_size ; i++){
            printf("%f ", dist_arr[i]);
        }

        median = quickselect(dist_arr, pointsNum * comm_size - 1);
        //printf("\nMedian distance is %f\n\n", median);
    }

    // Broadcast median.
    MPI_Bcast(&median, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    // Calculate number of unwanted points and gather all data to all processes.
    // This is the least amount of information needed to complete the transfers.
    int *unwantedMat = (int *) malloc(comm_size * sizeof(int));
    int *sortedByMedian = sortByMedian(distances, points, median, &proc);

    int unwantedNum = sortedByMedian[0];  

    MPI_Allgather(&unwantedNum, 1, MPI_INT, unwantedMat, 1, MPI_INT, MPI_COMM_WORLD);

    if (comm_rank == comm_size - 2) {
        // printf("Distances before sortByMedian for process #%d\n", comm_rank);
        // for (int i = 0; i < pointsNum; i++) {
        //     printf("%f ", distances[i]);
        // }
        // printf("\n\n");

        printf("\nUnwanted matrix\n");
        for (int i = 0; i < comm_size; i++) {
            printf("%d ", unwantedMat[i]);    
        }    
        printf("\n");  
    }

    // ---------- START TESTING DISRIBUTEBYMEDIAN ---------- //

    distributeByMedian(unwantedMat, points, distances, &proc, median, 0, comm_size);

    if (comm_rank == 2 || comm_rank == comm_size - 2) {
        printf("Distances after sortByMedian for process #%d\n", comm_rank);
        for (int i = 0; i < pointsNum; i++) {
            printf("%f ", distances[i]);
        }
        printf("\n\n");
    }
    


    

	MPI_Finalize();
	return 0;
}
