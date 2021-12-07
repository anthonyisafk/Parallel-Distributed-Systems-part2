#ifndef MPIHELP_H
#define MPIHELP_H

#include <stdio.h>

void bcast_dims_points(FILE *file, long *info, int comm_rank, int comm_size);
void split_into_processes(FILE *file, process *p, float *points);
void bcast_pivot(process *p, float *pivot, float *points);

#endif