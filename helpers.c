#ifndef HELPERS_H
#define HELPERS_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "headers/point.h"
#include "headers/helpers.h"


// Calculates the distance of p from a reference point, given in the form of an array.
double calculateDistanceArray(double *p, int start, double *ref, uint dims) {
	double distance = 0;
	for (int i = 0; i < dims; i++) {
		distance += pow(p[start + i] - ref[i], 2);
	}

	return distance;
}


// Standard Lomuto partition function.
// Taken from https://www.geeksforgeeks.org/quickselect-a-simple-iterative-implementation/
uint partition(double *arr, uint low, uint high) {
	uint pivot = arr[high];

	uint i = low - 1;
	for (uint j = low; j <= high - 1; j++) {
		if (arr[j] <= pivot) {
			i++;

			// Swap arr[i] and arr[j].
			double tmp = arr[i];
			arr[i] = arr[j];
			arr[j] = tmp;
		}
	}

	double tmp = arr[i + 1];
	arr[i + 1] = arr[high];
	arr[high] = tmp;

	return i + 1;
}


// Finds the k-th smallest element of an array. Kept outside of quickselect,
// since the distances array will probably consist of an even number of floats,
// which means we need to find the semi-sum of the values in the middle.
double kthSmallest(double *array, uint start, uint end, uint k) {
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
double quickselect(double *distances, uint end) {
	// The index where the median is supposed to be.
	uint mid_index = end / 2;
	printf("MID INDEX = %d\n", mid_index);

	// The median is calculated depending on whether the population is even or odd.
	if ((end + 1) % 2 == 0) {
		double mid1 = kthSmallest(distances, 0, end, mid_index + 1);
		double mid2 = kthSmallest(distances, 0, end, mid_index + 2);

		printf("mid1 = %f, mid2 = %f\n", mid1, mid2);
		return (double) ((mid1 + mid2) / 2);
	} else {
		return kthSmallest(distances, 0, end, mid_index);
	}
}


#endif