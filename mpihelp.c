#ifndef MPIHELP_H
#define MPIHELP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <math.h>
#include <time.h>

#include "headers/mpihelp.h"
#include "headers/helpers.h"
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


/**
 * 1 if the point should be kept,
 * 0 if distance is equal to median,
 * -1 if the point is to be gotten rid of.
 * *******************
 * @param right_half: Updated the function to sort the elements based on what each
 * process wants to get rid of. The first half of the processes sort elements so that
 * the values larger than the median are first and the other half prioritizes
 * having the smaller values first.
 */
void findUnwantedPoints (int *isUnwanted, float *distances, process *p, float median) {
    // Multiply by -1 if the process is looking for small elements to send out.
    int right_half = (p->comm_rank + 1 > p->comm_size / 2) ? -1 : 1;  

    for (long i = 0; i < p->pointsNum; i++) {
        if (right_half * distances[i] < right_half * median) {
            isUnwanted[i] = 1;
        } 
        else if (distances[i] == median) {
            isUnwanted[i] = 0;
        }
        else {
            isUnwanted[i] = -1;
        }
    }
}


/**
 * Sorts an array depending on the median value. Values greater than the median
 * go to the right, smaller values to the left and values equal to the median 
 * remain in the middle. The algorithm basically sorts the left side of the array,
 * while the right side takes care of itself during the execution.
 * Also swap the values between the helping arrays: points and isLargerThanMedian.
 */ 
void sortByMedian(float *array, float *points, int *largerThanMedian, float median, process *p) {
    // Multiply by -1 if the process is looking for small elements to send out.
    int right_half = (p->comm_rank + 1 > p->comm_size / 2) ? -1 : 1;

    // Keeps track of the value we are now checking.
	long i = 0;
    // The last value of the rightmost side that is sorted.
	long right = p->pointsNum;
    // The last value of the leftmost side that is sorted.
	long left = 0;
    // Keeps track of the last median encountered.
	long center = -1;

    while (i != right) {
        if (right_half * array[i] < right_half * median) {
            swapFloat(array, i, left, 1);
            swapFloat(points, i * p->dims, left * p->dims, p->dims);
            swapInt(largerThanMedian, i, left, 1);
            
            left++;
            center++;
            i++;
        }
        else if (right_half * array[i] > right_half * median) {
            // Only do the swap the value if it doesn't belong in the
            // rightmost set.
            if (right_half * array[right - 1] <= right_half * median) {
                swapFloat(array, i, right - 1, 1);
                swapFloat(points, i * p->dims, (right - 1) * p->dims, p->dims);
                swapInt(largerThanMedian, i, right - 1, 1);
            }
            right--;
        } else {
            center++;
            i++;
        }
    }
}


#endif