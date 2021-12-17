MPICC = mpicc
MATH = -lm
INCLUDES = helpers.c mpihelp.c

default: mpi_a

mpi_a:
	$(MPICC) mpi_a.c -o mpi_a.o $(INCLUDES) $(MATH)

suppress_errors:
	export OMPI_MCA_btl_vader_single_copy_mechanism=none

.PHONY: clean

clean:
	rm -f mpi_a.o 
