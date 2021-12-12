#ifndef HELPERS_H
#define HELPERS_H

float calculateDistanceArray(float *p, int start, float *ref, uint dims);

uint partition(float *arr, uint low, uint high);
float kthSmallest(float *array, uint start, uint end, uint k);
float quickselect(float *distances, uint end);

void swapFloat(float *array, uint x, uint y, long len);
void swapInt(int *array, uint x, uint y, long len);

#endif