CXX=mpicc

all: mpi1 mpi2 mpi3 mpi4 mpi5 mpi6 mpi7 ssort

mpi1: mpi_solved1.c
	$(CXX) mpi_solved1.c -o mpi1

mpi2: mpi_solved2.c
	$(CXX) mpi_solved2.c -o mpi2

mpi3: mpi_solved3.c
	$(CXX) mpi_solved3.c -o mpi3

mpi4: mpi_solved4.c
	$(CXX) mpi_solved4.c -o mpi4

mpi5: mpi_solved5.c
	$(CXX) mpi_solved5.c -o mpi5

mpi6: mpi_solved6.c
	$(CXX) mpi_solved6.c -o mpi6	

mpi7: mpi_solved7.c
	$(CXX) mpi_solved7.c -o mpi7

ssort: ssort.c	
	$(CXX) ssort.c -o ssort

clean:
	rm -rf *o mpi1 mpi2 mpi3 mpi4 mpi5 mpi6 mpi7 ssort