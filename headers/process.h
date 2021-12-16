/**
 * @file: process.h
 * ********************
 * @description: Describes a process struct, that is used to shorten 
 * the arguments list for the mpihelp header file. 
 */ 

#ifndef PROCESS_H
#define PROCESS_H

#include <mpi.h>

typedef struct {
    int comm_size;
    int comm_rank;
    long dims;
    long pointsNum;
    float *pivot;

    MPI_Status *mpi_stat101;
} process;

#endif