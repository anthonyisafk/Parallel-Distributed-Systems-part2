MPICC=mpicc
MATH=-lm

default: all

mpi_a:
	$(MPICC) mpi_a.c -o mpi_a.o $(MATH)

mpi:
	$(MPICC) mpi.c -o mpi.o $(MATH)

all: mpi_a mpi

.PHONY: clean

clean:
	rm -f mpi_a mpi
