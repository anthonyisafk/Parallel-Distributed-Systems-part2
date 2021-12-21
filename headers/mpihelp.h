#ifndef MPIHELP_H
#define MPIHELP_H

#include <stdio.h>

void bcast_dims_points(FILE *file, long *info, int comm_rank, int comm_size);
void split_into_processes(FILE *file, process *p, float *points);
void bcast_pivot(process *p, float *pivot, float *points);

int *sortByMedian(float *array, float *points, float median, process *p);

void findNewMedian(float *points, int *unwantedMat, float *distances, float *dist_array, bool *sortedMat,
    float median, MPI_Comm new_comm, process *p);
void splitGroup(MPI_Comm *comm, MPI_Comm *new_comm, int *my_new_comm_rank, int *my_new_comm_size,
    int colour, int key, process *p);
void distributeByMedian(int *unwantedMat, float *points, float *distances,
    process *p, float median, MPI_Comm comm, bool *sortedMat, int inft); 

#endif