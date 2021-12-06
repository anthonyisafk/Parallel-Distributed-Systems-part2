#ifndef HELPERS_H
#define HELPERS_H

double calculateDistanceArray(double *p, int start, double *ref, uint dims);

uint partition(double *arr, uint low, uint high);
double kthSmallest(double *array, uint start, uint end, uint k);
double quickselect(double *distances, uint end);


#endif