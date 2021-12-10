#ifndef HELPERS_H
#define HELPERS_H

float calculateDistanceArray(float *p, int start, float *ref, uint dims);

uint partition(float *arr, uint low, uint high);
float kthSmallest(float *array, uint start, uint end, uint k);
float quickselect(float *distances, uint end);
void swap(float *array, uint x, uint y, long len);
void sortByMedian(float *array, float *points, float median, long size, long dims);

#endif