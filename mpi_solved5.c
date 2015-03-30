/******************************************************************************
* FILE: mpi_bug5.c
* DESCRIPTION: 
*   This is an "unsafe" program. It's behavior varies depending upon the
*   platform and MPI library
* AUTHOR: Blaise Barney 
* LAST REVISED: 01/24/09
* Hint: If possible, try to run the program on two different machines,
* which are connected through a network. You should see uneven timings;
* try to understand/explain them.
******************************************************************************/


/* When running on network, the time will blows up every four or five rounds when using
MPI_Send and MPI_Recv. The reason for this is because MPI_Send has an internal buffer.
When the buffer is full, it has to wait MPI_Recv to consume all the data in the buffer
so MPI_send could continue to work. In this case, we could use MPI_Isend to MPI_Irecv 
instead.  */




#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define MSGSIZE 2000

int main (int argc, char *argv[])
{
int        numtasks, rank, i, tag=111, dest=1, source=0, count=0;
char       data[MSGSIZE];
double     start, end, result;
MPI_Status status;




MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);

if (rank == 0) {
  printf ("mpi_bug5 has started...\n");
  if (numtasks > 2) 
    printf("INFO: Number of tasks= %d. Only using 2 tasks.\n", numtasks);
  }

/******************************* Send task **********************************/
if (rank == 0) {

  /* Initialize send data */
  for(i=0; i<MSGSIZE; i++)
     data[i] =  'x';

  start = MPI_Wtime();
  while (1) {
    MPI_Send(data, MSGSIZE, MPI_BYTE, dest, tag, MPI_COMM_WORLD);
    count++;
    if (count % 10 == 0) {
      end = MPI_Wtime();
      printf("Count= %d  Time= %f sec.\n", count, end-start);
      start = MPI_Wtime();
      }
    }
  }

/****************************** Receive task ********************************/

if (rank == 1) {
  while (1) {
    MPI_Recv(data, MSGSIZE, MPI_BYTE, source, tag, MPI_COMM_WORLD, &status);
    /* Do some work  - at least more than the send task */
    result = 0.0;
    for (i=0; i < 1000000; i++) 
      result = result + (double)random();
    }
  }

MPI_Finalize();
}


