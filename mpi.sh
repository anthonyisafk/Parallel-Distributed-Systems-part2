#!/bin/bash
#SBATCH --partition=batch
#SBATCH --ntasks-per-node=16
#SBATCH --nodes=2
#SBATCH --time=1:00:00

module load gcc openmpi

mpicc mpi_a.c -o mpi_a.o helpers.c mpihelp.c -lm

srun ./mpi_a.o

