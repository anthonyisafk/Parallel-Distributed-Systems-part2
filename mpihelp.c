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

        info[1] = 4; // set the points per process to 200.
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

 //TO DO: add communicator parameters since it will be different for each recursive step 
int findUnwantedPoints (int *isUnwanted, float *distances, process *p, float median) {
    // Multiply by -1 if the process is looking for small elements to send out.
    int right_half = (p->comm_rank + 1 > p->comm_size / 2) ? -1 : 1;  
    int unwantedNum = 0;
    for (long i = 0; i < p->pointsNum; i++) {
        if (right_half * distances[i] < right_half * median) {
            isUnwanted[i] = 1;
        } 
        else if (distances[i] == median) {
            isUnwanted[i] = 0;
            unwantedNum++;
        }
        else {
            isUnwanted[i] = -1;
            unwantedNum++;
        }
    }

    return unwantedNum;
}


/**
 * Sorts an array depending on the median value.
 * The algorithm basically sorts the left side of the array,
 * while the right side takes care of itself during the execution.
 * Also swap the values between the helping array points.
 */ 

 //TO DO: add communicator parameters since it will be different for each recursive step 
void sortByMedian(float *array, float *points, float median, process *p) {
    // Multiply by -1 if the process is looking for small elements to send out.
    int right_half = (p->comm_rank + 1 > p->comm_size / 2) ? -1 : 1; 

    // Keeps track of the value we are now checking.
	long i = 0;
    // The index of the rightmost side that is sorted.
	long right = p->pointsNum;
    // The index of the leftmost side that is sorted.
	long left = 0;
    // Keeps track of the last median encountered.
	long center = -1;

    while (i != right) {
        if (right_half * array[i] < right_half * median) {
            swapFloat(array, i, left, 1);
            swapFloat(points, i * p->dims, left * p->dims, p->dims);
            
            left++;
            center++;
            i++;
        }
        else if (right_half * array[i] > right_half * median) {
            // Only do the swap if value doesn't belong in the
            // rightmost set.
            if (right_half * array[right - 1] <= right_half * median) {
                swapFloat(array, i, right - 1, 1);
                swapFloat(points, i * p->dims, (right - 1) * p->dims, p->dims);
            }
            right--;
        } else {
            center++;
            i++;
        }
    }
}


void distributeByMedian(int *unwantedMat, int unwantedNum, float *points, float *distances,
    process *p, float median, int start, int end) 
{
    // Find the side on which the process is on. As we already know, the right half
    // contains the larger values.
    bool left_half = p->comm_rank < (end - start) / 2;

    // The start and end of the indices each process scans for a peer.
    int peerScanStart = (left_half) ? (end - start) / 2 : start; 
    int peerScanEnd = (left_half) ? end : (end - start) / 2;
    
    // The start and end of the indices each process scans for its position
    // in the side it is on.
    int posScanStart = (left_half) ? 0 : (end - start) / 2; 
    int posScanEnd = p->comm_rank + 1;

    // Keep how many points the process had to give in the previous round.
    int previousRound = unwantedNum;

    bool sorted = false;
    int round = 0;
    while(!sorted) {
        if (unwantedNum != 0) {
            // The process's position in regards to the number of the elements to be sent out.
            int my_pos = 0;
            // The position of the process to which the points will be sent.
            int peer_pos = 0;
            int peer = 0;
            // The number of points that will finally be sent.
            int toTrade = 0;

            // Find how many procs before me have unwanted elements
            // My_pos > 0
            for (int i = posScanStart; i < posScanEnd; i++) {
                if(unwantedMat[i] != 0) {
                    my_pos++;
                } 
            }

            // Look at the other side for peer
            for (int i = peerScanStart; i < peerScanEnd; i++) {
                if (unwantedMat[i] != 0) {
                    peer_pos++;
                }
                
                if (peer_pos == my_pos) {
                    peer = i;

                    // Send only as many points as both processes can handle.
                    toTrade = (unwantedNum <= unwantedMat[i]) ? unwantedNum : unwantedMat[i]; 
                    printf("Proc %d paired with proc %d to trade %d elements\n", p->comm_rank, i, toTrade);
                    break;
                }
            }

            // If no peer is found after the for loop, wait for next parallel round.
            // Don't bother sending anything if toTrade == 0.
            if (peer_pos != 0 && peer_pos < end && toTrade > 0) {
                MPI_Sendrecv_replace(&(points[p->dims * p->pointsNum - p->dims * unwantedNum]), p->dims * toTrade, 
                    MPI_FLOAT, peer, 110, peer, 110, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                // Update how many points the process has to get rid of now.
                unwantedNum -= toTrade;
            }
        }

        // Calculate distances for the new points. Then see how many unwanted points each process has.
        MPI_Barrier(MPI_COMM_WORLD);
        for (int i = 0; i < p->pointsNum; i++) {
            distances[i] = calculateDistanceArray(points, p->dims * i, p->pivot, p->dims);
        }

        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Allgather(&unwantedNum, 1, MPI_INT, unwantedMat, 1, MPI_INT, MPI_COMM_WORLD);

        // Check if the algoritm is finished. Otherwise, everybody participates again.
        for (int i = 0; i < end - start; i++) {
            if (unwantedMat[i] != 0) {
                sorted = false;
                break;
            } else {
                sorted = true;
            }
        }

        if (p->comm_rank == 0) {
            printf("\nUnwantedMat round %d:\n", round);
            for (int i = 0; i < end - start; i++) {
                printf("%d ", unwantedMat[i]);
            }
        }
        round++;
    }

}

#endif