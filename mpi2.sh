#!/bin/bash
#SBATCH --partition=batch
#SBATCH --ntasks-per-node=2
#SBATCH --nodes=1
#SBATCH --time=1:00:00

module load gcc openmpi

mpicc mpi_a.c -o mpi_a.o helpers.c mpihelp.c -lm

for i in {1..10}; do srun ./mpi_a.o; done