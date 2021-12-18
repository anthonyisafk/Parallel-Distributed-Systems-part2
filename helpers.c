#ifndef HELPERS_H
#define HELPERS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "headers/process.h"
#include "headers/helpers.h"

#define SWAP(x, y) { float temp = x; x = y; y = temp; }


// Calculates the max power of base that's closer to num.
int maxPower(int num, int base, int rep) {
	if (pow(base, rep) > num) {
		return -1;
	} 
	else if (pow(base, rep) == num) {
		return 0;
	} else {
		return 1 + maxPower(num, base, rep+1);
	}
}


// Calculates the distance of p from a reference point, given in the form of an array.
float calculateDistanceArray(float *p, int start, float *ref, uint dims) {
	float distance = 0;
	for (int i = 0; i < dims; i++) {
		distance += pow(p[start + i] - ref[i], 2);
	}

	return distance;
}


// Partition using Lomuto partition scheme
int partition(float* a, int left, int right, int pIndex)
{
	// pick `pIndex` as a pivot from the array
	float pivot = a[pIndex];
 
	// Move pivot to end
	SWAP(a[pIndex], a[right]);
 
	// elements less than the pivot will be pushed to the left of `pIndex`;
	// elements more than the pivot will be pushed to the right of `pIndex`;
	// equal elements can go either way
	pIndex = left;
 
	// each time we find an element less than or equal to the pivot, `pIndex`
	// is incremented, and that element would be placed before the pivot.
	for (int i = left; i < right; i++)
	{
		if (a[i] <= pivot)
		{
			SWAP(a[i], a[pIndex]);
			pIndex++;
		}
	}
 
	// move pivot to its final place
	SWAP(a[pIndex], a[right]);
 
	// return `pIndex` (index of the pivot element)
	return pIndex;
}
 
// Returns the k'th smallest element in the list within `left…right`
// (i.e., left <= k <= right). The search space within the array is
// changing for each round – but the list is still the same size.
// Thus, `k` does not need to be updated with each round.
float kthSmallest(float* nums, int left, int right, int k)
{
	// If the array contains only one element, return that element
	if (left == right) {
		return nums[left];
	}
 
	// select `pIndex` between left and right
	int pIndex = left + rand() % (right - left + 1);
 
	pIndex = partition(nums, left, right, pIndex);
 
	// The pivot is in its final sorted position
	if (k == pIndex) {
		return nums[k];
	}
 
	// if `k` is less than the pivot index
	else if (k < pIndex) {
		return kthSmallest(nums, left, pIndex - 1, k);
	}
 
	// if `k` is more than the pivot index
	else {
		return kthSmallest(nums, pIndex + 1, right, k);
	}
}

float quickselect(float *distances, uint end) {
	// The index where the median is supposed to be.
	uint mid_index = (end + 1) / 2;

	// The median is calculated depending on whether the population is even or odd.
	if ((end + 1) % 2 == 0) {
	
		float mid1 = kthSmallest(distances, 0, end, mid_index - 1);
		float mid2 = kthSmallest(distances, 0, end, mid_index);
		
		printf("mid1 = %f, mid2 = %f\n", mid1, mid2);

	
		return (float) ((mid1 + mid2) / 2);
	} else {
		printf("To kalo to palikari paei apo allo monopati\n");
		return kthSmallest(distances, 0, end, mid_index);
	}
	
}


/**
 * Swaps two values in an array. 
 * @param len: Used if we want to swap chunks of data, rather than just a single value.
 * len is set to dims when transfering points in a process.
 * The float version of the function.
 */ 
void swapFloat(float *array, long x, long y, long len) {
	for (long i = 0; i < len; i++) {
		float temp = array[x+i];
		array[x+i] = array[y+i];
		array[y+i] = temp;
	}
}


// Swap for integer arrays. 
void swapInt(int *array, long x, long y, long len) {
	for (long i = 0; i < len; i++) {
		float temp = array[x+i];
		array[x+i] = array[y+i];
		array[y+i] = temp;
	}
}



#endif