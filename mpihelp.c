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
        // Only read the closest power of 2, not all of them.
        int totalPoints = pow(2, maxPower(info[1], 2, 0));
        info[1] = totalPoints / comm_size;
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
    } else {
        MPI_Recv(points, p->dims * p->pointsNum, MPI_FLOAT, 0, 101, MPI_COMM_WORLD, p->mpi_stat101);
    }
}


// Let the master select and broadcast the pivot point.
void bcast_pivot(process *p, float *pivot, float *points) {

    // Pick a pivot and broadcast it 
    if (p->comm_rank == 0) {
        int pivotIndex = rand() % p->pointsNum;
        // int pivotIndex = 238; // check for indices that are known to have broken the algo.
        printf("Pivot index is %d\n", pivotIndex);

        for (int i = 0; i < p->dims; i++) {
            pivot[i] = points[i + pivotIndex * p->dims];
		}  
    }
    MPI_Bcast(pivot, p->dims, MPI_FLOAT, 0, MPI_COMM_WORLD);
}


/**
 * Sorts an array depending on the median value.
 * The algorithm basically sorts the left side of the array,
 * while the right side takes care of itself during the execution.
 * Also swap the values between the helping array points.
 * **************************************************************
 * The algorithm now shifts all the median values to the rightmost part
 * of the array, to be sent last, prioritizing getting rid of the 
 * greater values first.
 */ 

int *sortByMedian(float *array, float *points, float median, process *p) {
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
            if (right_half * array[right - 1] < right_half * median) {
                swapFloat(array, i, right - 1, 1);
                swapFloat(points, i * p->dims, (right - 1) * p->dims, p->dims);
            }
            right--;
        } else {
            // Experimental !!!
            // If median is found move it to the end
            center++;
            i++;
        }
    }

    // Swap the last median with the last right 
    // Just a playa playing
    if (center + 1 - left != 0) {
        printf("dist[right-1] = %f\n", array[right-1]);
        printf("median = %f\n", median);
    }

    // Gradually shift every median to the end.
    for (int i = 0 ; i < center + 1 - left; i++) {
        swapFloat(array, p->pointsNum - i - 1, right - i - 1, 1);
        swapFloat(points, (p->pointsNum - i - 1) * p->dims, (right - i - 1) * p->dims, p->dims);
    }

    int *result = (int *) malloc(3 * sizeof(int));
    // Return the number of unwanted points, aka unwantedNum.
    // Also including elements that are equal to the median for now.
    result[0] = p->pointsNum - left;
    // Return the number of points equal to the median.
    result[1] = center + 1 - left;
    // Return the index of the first median.
    result[2] = (result[1]) ? left : -1;

    return result;
}


// Splits a group of processes to two halves.
void splitGroup(MPI_Comm *comm, MPI_Comm *new_comm, int *my_new_comm_rank, int *my_new_comm_size,
    int colour, int key, process *p) 
{
    // 0 if in left half 1 if in right half.
    colour = ((p->comm_rank + 1) * 2 <= p->comm_size) ? 0 : 1; 
    // Assign the same keys to processes that lie in the same position of each half.
    // key = ((p->comm_rank + 1) * 2 <= p->comm_size) ? p->comm_rank : p->comm_rank - p->comm_size / 2;
    key = p->comm_rank;
    
    MPI_Comm_split(*comm, colour, key, new_comm);
 
    // Get my rank in the new communicator. Update the comm_rank and comm_size placeholders,
    // to be used in the next recursive call of the function.
    MPI_Comm_rank(*new_comm, my_new_comm_rank);
    MPI_Comm_size(*new_comm, my_new_comm_size);
    p->comm_rank = *my_new_comm_rank;
    p->comm_size = *my_new_comm_size;
}


// Finds the new median after a group of processes has been sorted and split.
void findNewMedian(float *points, int *unwantedMat, float *distances, float *dist_array, bool *sortedMat,
    float median, MPI_Comm new_comm, process *p) 
{
    for (int i = 0; i < p->pointsNum; i++) {
        distances[i] = calculateDistanceArray(points, p->dims * i, p->pivot, p->dims);
    }

    if (p->comm_rank == 0) {
        dist_array = (float *) malloc(p->pointsNum * p->comm_size * sizeof(float));
    }
    MPI_Gather(distances, p->pointsNum, MPI_FLOAT, dist_array, p->pointsNum, MPI_FLOAT, 0, new_comm);

    if (p->comm_rank == 0) {
        median = quickselect(dist_array, p->pointsNum * p->comm_size - 1);
        //printf("\nMedian distance is %f\n\n", median);
    }
    // Broadcast median.
    MPI_Bcast(&median, 1, MPI_FLOAT, 0, new_comm);

    sortedMat = realloc(sortedMat, p->comm_size * sizeof(bool));
    unwantedMat = (int *) realloc(unwantedMat ,p->comm_size * sizeof(int));
    int *newSortedByMedian = sortByMedian(distances, points, median, p);

    int newUnwantedNum = newSortedByMedian[0];  

    MPI_Allgather(&newUnwantedNum, 1, MPI_INT, unwantedMat, 1, MPI_INT, new_comm);
}


void distributeByMedian(int *unwantedMat, float *points, float *distances, process *p,
    float median, MPI_Comm comm, bool *sortedMat) 
{
    // End of recursion.
    if (p->comm_size == 1) {
        printf("All points are sorted! \n");
        return;
    }

    // Find the side on which the process is on. As we already know, the right half
    // contains the larger values.
    bool left_half = p->comm_rank < p->comm_size / 2;

    // The start and end of the indices each process scans for a peer.
    int peerScanStart = (left_half) ? p->comm_size / 2 : 0; 
    int peerScanEnd = (left_half) ? p->comm_size : p->comm_size / 2;
    
    // The start and end of the indices each process scans for its position
    // in the side it is on.
    int posScanStart = (left_half) ? 0 : p->comm_size / 2; 
    int posScanEnd = p->comm_rank + 1;

    // "Are my points sorted?"
    bool sorted = false;
    // "Are everyone's points sorted?"
    bool allsorted = false;
    
    int round = 0;
    while(!sorted) {
        if (unwantedMat[p->comm_rank] != 0) {
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
                    toTrade = (unwantedMat[p->comm_rank] <= unwantedMat[i]) ? unwantedMat[p->comm_rank] : unwantedMat[i]; 
                    // printf("Proc %d paired with proc %d to trade %d elements\n", p->comm_rank, i, toTrade);
                    break;
                }
            }

            // If peer pos is less than process position then process will not participate in
            // this parallel round
            if (peer_pos == my_pos) {
                MPI_Sendrecv_replace(&(points[p->dims * p->pointsNum - p->dims * unwantedMat[p->comm_rank]]), p->dims * toTrade, 
                    MPI_FLOAT, peer, 110, peer, 110, comm, MPI_STATUS_IGNORE);

                // Update how many points the process has to get rid of now.
                unwantedMat[p->comm_rank] -= toTrade;
            }
        }

        MPI_Allgather(&unwantedMat[p->comm_rank], 1, MPI_INT, unwantedMat, 1, MPI_INT, comm);

        for (int i = 0; i < p->comm_size ; i++) {
            if (unwantedMat[i] != 0) {
                sorted = false;
                break;
            } else {
                sorted = true;
            }
        }

        // Print the matrix of unwanted points in case a group of processes 
        // has taken too long to get sorted.
        if (round > 1000 && unwantedMat[p->comm_rank] != 0) {
            printf("\n\nINFINITE LOOOP\n\n");
            
            printf("\nUnwantedMat round %d:\n", round);
            for (int i = 0; i < p->comm_size; i++) {
                printf("%d ", unwantedMat[i]);
            }
            printf("\n");
        } 

        // If I am the process that is unsorted, trick the algorithm into
        // thinking it's done with.
        if (round > 1000) {
            sorted = true;
        }
        round++;
    }

    // The process that 'lied' turns the sorted value to false again,
    // in order to broadcast it to the rest of the group.
    if (round > 1000) {
        sorted = false;
    }

    MPI_Allgather(&sorted, 1, MPI_C_BOOL, sortedMat, 1, MPI_C_BOOL, comm);
    for (int i = 0; i < p->comm_size; i++) {
        if (!sortedMat[i]) {
            allsorted = false;
            break;
        } else {
            allsorted = true;
        }
    }

    // See if everyone is sorted. If yes, split into two groups and move on.
    // Elsewise, sort points again and re-enter distributeByMedian.
    if (allsorted) {
        // --------------- SPLIT INTO TWO HALVES --------------- //

        MPI_Comm new_comm;
        int my_new_comm_rank, my_new_comm_size;
        int colour, key;

        splitGroup(&comm, &new_comm, &my_new_comm_rank, &my_new_comm_size, colour, key, p);

        // --------------- RECALCULATE DISTANCES AND UNWANTED PONTS --------------- //

        float *dist_array = NULL;
        if (p->comm_rank == 0) {
            dist_array = (float *) malloc(p->pointsNum * p->comm_size * sizeof(float));
        }
        findNewMedian(points, unwantedMat, distances, dist_array, sortedMat, median, new_comm, p);

        // --------------- CALL THE RECURSION --------------- //

        distributeByMedian(unwantedMat, points, distances, p, median, new_comm, sortedMat);
    } else {
        float *dist_array = NULL;
        if (p->comm_rank == 0) {
            dist_array = (float *) malloc(p->pointsNum * p->comm_size * sizeof(float));
        }

        findNewMedian(points, unwantedMat, distances, dist_array, sortedMat, median, comm, p);
        distributeByMedian(unwantedMat, points, distances, p, median, comm, sortedMat);
    }
}

#endif