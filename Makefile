MPICC = mpicc
GCC = gcc
MATH = -lm
INCLUDES = helpers.c mpihelp.c

default: mpi_a

mpi_a:
	$(MPICC) mpi_a.c -o mpi_a.o $(INCLUDES) $(MATH)

linear:
	$(GCC) linear.c -o linear.o $(MATH)

suppress_errors:
	export OMPI_MCA_btl_vader_single_copy_mechanism=none

.PHONY: clean

times_mpi:
	for i in 2 4 8 16 32 64; do for j in $(shell seq 10); do mpiexec -np $$i ./mpi_a.o; done; done

times_linear:
	for i in 2 4 8 16 32 64; do for j in $(shell seq 10); do ./linear.o $$i; done; done

test_echo:
	for i in $(shell seq 10); do echo $$i; done 

clean:
	rm -f mpi_a.o linear.o 
