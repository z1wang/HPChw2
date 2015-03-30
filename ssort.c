/* Parallel sample sort
 */
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include "util.h"

static int compare(const void *a, const void *b)
{
  int *da = (int *)a;
  int *db = (int *)b;

  if (*da > *db)
    return 1;
  else if (*da < *db)
    return -1;
  else
    return 0;
}

int main( int argc, char *argv[])
{
  int rank;
  int i, N;
  int *vec;
  int nprocs;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  /* Number of random numbers per processor (this should be increased
   * for actual tests or could be passed in through the command line */
  N = 100;

  vec = calloc(N, sizeof(int));
  /* seed random number generator differently on every core */
  srand((unsigned int) (rank + 393919));

  /* fill vector with random integers */
  for (i = 0; i < N; ++i) {
    vec[i] = rand();
  }


  /* sort locally */
  qsort(vec, N, sizeof(int), compare);



  int space = floor(N/nprocs);
  int jdex = space;
  int *sample = calloc((nprocs -  1), sizeof(int));

  /* randomly sample s entries from vector or select local splitters,
   * i.e., every N/P-th entry of the sorted vector */
  int index = 0;
  for(i = jdex; i < N; i = i + space){
    sample[index] = vec[i];
    index ++;
  }


  /* every processor communicates the selected entries
   * to the root processor; use for instance an MPI_Gather */
  int *rBuf;
  int rSize = (nprocs - 1) * nprocs;
  if(rank == 0){
    rBuf = calloc(rSize, sizeof(int));
  }
  MPI_Gather(sample, (nprocs - 1), MPI_INT, rBuf, (nprocs - 1), MPI_INT, 0, MPI_COMM_WORLD);

  /* root processor does a sort, determinates splitters that
   * split the data into P buckets of approximately the same size */
  int *sp = calloc(nprocs, sizeof(int));
  if(rank == 0){
    qsort(rBuf, rSize, sizeof(int), compare);
    index = nprocs - 1;
    for(i = 0; i < nprocs - 1; i++){
      sp[i] = rBuf[index];
      index = index + nprocs - 1;
    }
  }
  sp[nprocs - 1] = RAND_MAX;
  /* root process broadcasts splitters */
  MPI_Bcast(sp, nprocs, MPI_INT, 0, MPI_COMM_WORLD);


  /* every processor uses the obtained splitters to decide
   * which integers need to be sent to which other processor (local bins) */

  /* send and receive: either you use MPI_AlltoallV, or
   * (and that might be easier), use an MPI_Alltoall to share
   * with every processor how many integers it should expect,
   * and then use MPI_Send and MPI_Recv to exchange the data */
   int *sendCounts = calloc(nprocs, sizeof(int)); 

   int *recCounts = calloc(nprocs, sizeof(int)); // This array stores the number of elements from each processor

   int count = 0;
   index = 0;
   for(i = 0; i < N; i++){
    if(vec[i] <= sp[index]){
      sendCounts[index]++;
    }else{
      while(vec[i] > sp[index]){
        index ++;
      }
      sendCounts[index] ++;
    }
   }


   MPI_Alltoall(sendCounts, 1, MPI_INT, recCounts, 1, MPI_INT, MPI_COMM_WORLD);

   int *sDisp = calloc(nprocs, sizeof(int));
   int *rDisp = calloc(nprocs, sizeof(int));

   for(i = 1; i < nprocs; i++){
    sDisp[i] = sDisp[i - 1] + sendCounts[i - 1];
    rDisp[i] = rDisp[i - 1] + recCounts[i - 1];
   }

   int mySize = rDisp[nprocs - 1] + recCounts[nprocs - 1];

   int *result = calloc(mySize, sizeof(int));

    MPI_Alltoallv(vec, sendCounts, sDisp, MPI_INT, result, recCounts, rDisp, MPI_INT, MPI_COMM_WORLD);


  /* do a local sort */

    qsort(result, mySize, sizeof(int), compare);

  /* every processor writes its result to a file */

    FILE* fd = NULL;
    char filename[256];
    snprintf(filename, 256, "output%d.txt", rank);
    fd = fopen(filename,"w+");

    if(NULL == fd)
    {
      printf("Error opening file \n");
      return 1;
    }

    for(i = 0; i < mySize; i++)
      fprintf(fd, " %d ", result[i]);
    fclose(fd);

  /* free allocated memory */
  free(result);
  free(recCounts);
  free(sendCounts);
  free(vec);
  free(sp);
  if(rank==0)
    free(rBuf);
  free(sample);
  MPI_Finalize();
  return 0;
}
