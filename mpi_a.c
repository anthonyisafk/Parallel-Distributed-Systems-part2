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

#include "headers/point.h"
#include "headers/helpers.h"

void findMedian(){
	


}




int main(int argc, char **argv) {
	int DIMS = 2;
	int POINTS_NUM = 3;
  
	int comm_size;
	int comm_rank;

	// --------------- START OF TESTING MPI --------------- //
	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);

	float *pivotCoord = (float *)malloc(DIMS*sizeof(float));
	point pivot = {DIMS, pivotCoord};

	float *distances = (float *)malloc(POINTS_NUM*sizeof(float));

	point *points = (point *) malloc(POINTS_NUM * sizeof(point));
	for (int i = 0; i < POINTS_NUM; i++) {
		points[i].coord = (float *) malloc(DIMS * sizeof(float)); 
	}

	time_t t;
	srand((unsigned) time(&t));

	for (int i = 0; i < POINTS_NUM; i++) {
		points[i].dims = DIMS;
		for (int j = 0; j < DIMS; j++) {
			points[i].coord[j] = (comm_rank+0.5) * (float)rand() / ((float)RAND_MAX);
		}
	}

	printProcess(points, comm_rank, POINTS_NUM, DIMS);	

	// The master picks a pivot, then broadcasts the index (for testing's sake)
	// then copies the coordinates to the array it's going to broadcast.
	if (comm_rank == 0) {
		int pivotIndex = rand() % POINTS_NUM;

		for (int i = 0; i < DIMS; i++) {
			pivot.coord[i] = points[pivotIndex].coord[i];
			printf("pivot.coord[%d] = %f\n", i, pivot.coord[i]);
		}
	}

	MPI_Bcast(pivotCoord, DIMS, MPI_FLOAT, 0, MPI_COMM_WORLD);

	// For each point calculate distance from pivot
	for(int i=0; i<POINTS_NUM; i++){
		distances[i] = calculateDistancePoint(points[i], pivot);
	}

	// Print distance matrices
	MPI_Barrier(MPI_COMM_WORLD);
	for(int i=0; i<POINTS_NUM; i++){
		printf("distances[%d] = %f\n", i, distances[i]);
	}

	//Call recursive equation


	MPI_Finalize();

	return 0;
}
