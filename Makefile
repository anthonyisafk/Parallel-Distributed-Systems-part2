MPICC=mpicc
MATH=-lm
INCLUDES=helpers.c

default: all

mpi_a:
	$(MPICC) mpi_a.c -o mpi_a.o $(INCLUDES) $(MATH)

all: mpi_a 

.PHONY: clean

clean:
	rm -f mpi_a.o 
