/**
 * @file: mpi.c
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

#include "headers/point.h"

#define DIMS 5
#define PROCS 10
#define POINTS_NUM 10


// Calculates the distance of p from a reference point.
float calculateDistance(point p, point ref) {
	int dims_p = p.dims;
	int dims_ref = ref.dims;

	if (dims_ref != dims_p) {
		printf("\nDon't know how to calculate distance between points of different dimensions.\n");
		return -1;
	}

	float distance = 0;
	for (int i = 0; i < dims_ref; i++) {
		distance += pow(p.coord[i] - ref.coord[i], 2);
	}

	return distance;
}

// Standard Lomuto partition function.
// Taken from https://www.geeksforgeeks.org/quickselect-a-simple-iterative-implementation/
uint partition(float *arr, uint low, uint high) {
	uint pivot = arr[high];

	uint i = low - 1;
	for (uint j = low; j <= high - 1; j++) {
		if (arr[j] <= pivot) {
			i++;

			// Swap arr[i] and arr[j].
			float tmp = arr[i];
			arr[i] = arr[j];
			arr[j] = tmp;
		}
	}

	float tmp = arr[i + 1];
	arr[i + 1] = arr[high];
	arr[high] = tmp;

	return i + 1;
}


// Finds the k-th smallest element of an array. Kept outside of quickselect,
// since the distances array will probably consist of an even number of floats,
// which means we need to find the semi-sum of the values in the middle.
float kthSmallest(float *array, uint start, uint end, uint k) {
	while (start <= end) {
		uint pivotIndex = partition(array, start, end);

		if (pivotIndex == k - 1) {
			return array[pivotIndex];
		} 
		else if (pivotIndex > k - 1) {
			end = pivotIndex - 1;
		} else {
			start = pivotIndex + 1;
		}
	}

	// Return -1 if the function fails.
	return -1;
}


// Finds the median in an array of distances.
float quickselect(float *distances, uint end) {
	// The index where the median is supposed to be.
	uint mid_index = end / 2;
	printf("MID INDEX = %d\n", mid_index);

	// The median is calculated depending on whether the population is even or odd.
	if ((end + 1) % 2 == 0) {
		float mid1 = kthSmallest(distances, 0, end, mid_index + 1);
		float mid2 = kthSmallest(distances, 0, end, mid_index + 2);

		printf("mid1 = %f, mid2 = %f\n", mid1, mid2);
		return (float) ((mid1 + mid2) / 2);
	} else {
		return kthSmallest(distances, 0, end, mid_index);
	}
}


int main(int argc, char **argv) {
	float *ref_point = (float *) malloc(DIMS * sizeof(float));
	point reference = {DIMS, ref_point};

	point **points = (point **) malloc(PROCS * sizeof(point *));
	for (int i = 0; i < PROCS; i++) {
		points[i] = (point *) malloc(POINTS_NUM * sizeof(reference));
	}

	// for (int i = 0; i < PROCS; i++) {
	// 	for (int j = 0; j < POINTS_NUM; j++) {
	// 		points[i][j] = random()



	// 	}
	// }



	return 0;
}
