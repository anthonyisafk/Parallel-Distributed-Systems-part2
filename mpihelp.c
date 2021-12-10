#ifndef MPIHELP_H
#define MPIHELP_H

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h>

#include "headers/mpihelp.h"
#include "headers/process.h"


// Broadcast the dimensions of each point and how many points each process will have.
void bcast_dims_points(FILE *file, long *info, int comm_rank, int comm_size) {
    if (comm_rank == 0) {
        fread(info, sizeof(long), 2, file);

        // Split the points evenly between each process.
        // info[1] /= comm_size;

        info[1] = 10; // set the points per process to 200.
    } 

	MPI_Bcast(info, 2, MPI_LONG, 0, MPI_COMM_WORLD);
}


// Read the binary file in easier-to-handle chunks and send them out to the processes.
void split_into_processes(FILE *file, process *p, float *points) {
    if (p->comm_rank == 0) {
        // Send the first batch of floats back to master.
        fread(points, sizeof(float), p->dims * p->pointsNum , file);
        MPI_Sendrecv(points, p->dims * p->pointsNum, MPI_FLOAT, 0, 101, points,
            p->dims * p->pointsNum, MPI_FLOAT, 0, 101, MPI_COMM_WORLD, p->mpi_stat101);

        // Keep reading and send to the other processes.
        for (int i = 1; i < p->comm_size; i++) {
            fread(points, sizeof(float), p->dims * p->pointsNum , file);
            MPI_Send(points, p->dims * p->pointsNum, MPI_FLOAT, i, 101, MPI_COMM_WORLD);
        }
    } else{
        MPI_Recv(points, p->dims * p->pointsNum, MPI_FLOAT, 0, 101, MPI_COMM_WORLD, p->mpi_stat101);
    }
}


// Let the master select and broadcast the pivot point.
void bcast_pivot(process *p, float *pivot, float *points) {
    // Pick a pivot and broadcast it 
    if (p->comm_rank == 0) {
        int pivotIndex = rand() % p->pointsNum;
        printf("Pivot index is %d\n", pivotIndex);

        for (int i = 0; i < p->dims; i++) {
            pivot[i] = points[i + pivotIndex * p->dims];
		}  
    }
    MPI_Bcast(pivot, p->dims, MPI_FLOAT, 0, MPI_COMM_WORLD);
}


#endif