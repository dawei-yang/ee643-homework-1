#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <string.h>
#include <math.h>
#include <time.h>


static void program_abort(char *message) {
  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  if (my_rank == 0) {
    if (message) {
      fprintf(stderr,"%s",message);
    }
  }
  MPI_Abort(MPI_COMM_WORLD, 1);
  exit(1);
}

static int isPerfectSquare(int number) {
	int iVar;
    float fVar;
    fVar = sqrt((double)number);
    iVar = fVar;
    if(iVar==fVar) return 1;
    else return 0;
}

int main(int argc, char *argv[]) {
    int N;
    MPI_Init(&argc, &argv);
    if(argc < 2) {
		program_abort("Missing Dimension of Matrix <N>\n");
	}

	N = strtol(argv[1], NULL, 10);
	if(N <= 0) {
        program_abort("Invalid argument <N>\n");
 
    }	
    // MPI_Comm fox_comm;
    MPI_Status status_B;
    MPI_Request request_send, request_recv;
    int rank, size, step, i, j, k;

    double *A, *B, *C, *TmpA, *TmpB;
    double start, end;
    double send_data = 0.0;
	double recv_data = 0.0;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(isPerfectSquare(size) == 0) program_abort("Number of processes is not a perfect number\n");
    int p = (int)sqrt(size);
    int tile_size = N/p;
	int row_num = rank/p;
	int col_num = rank%p;

    A = (double *)malloc(tile_size * tile_size * sizeof(double));
    B = (double *)malloc(tile_size * tile_size * sizeof(double));
    C = (double *)malloc(tile_size * tile_size * sizeof(double));
    TmpA = (double *)malloc(tile_size * tile_size * sizeof(double));
    TmpB = (double *)malloc(tile_size * tile_size * sizeof(double));



    for (i = 0; i < tile_size; i++) {
        for (j = 0; j < tile_size; j++) {
            // use for test: for a 4x4: result should be 231,161
            // A[i * tile_size + j] = B[i * tile_size + j] = rand()%100;

            A[i * tile_size + j] = i + row_num * tile_size;
            B[i * tile_size + j] = i + row_num * tile_size + j + col_num * tile_size;
            C[i * tile_size + j] = 0.0;
        }
    }

    // Establish row and col sub communicators
	MPI_Comm new_comm_row;
	MPI_Comm new_comm_col;
	MPI_Comm_split(MPI_COMM_WORLD, row_num, col_num, &new_comm_row);
	MPI_Comm_split(MPI_COMM_WORLD, col_num, row_num, &new_comm_col);

    if (rank == 0) {
        start = MPI_Wtime();
    }
    for (step = 0; step < p; step++) {

        // up shift B, nonblocking  
        for(i = 0; i < tile_size; i++) {
            for(j=0; j < tile_size; j++) {
                // TmpB[i* tile_size + j] = B[i* tile_size + j];
                TmpA[i* tile_size + j] = A[i* tile_size + j];
            }
        }

        MPI_Isend(B, tile_size* tile_size, MPI_DOUBLE, (row_num-1 + p)%p, 3, new_comm_col, &request_send);

        // Broadcase Diagonal el -> row
        int root = (step + row_num)%p;
       
        MPI_Bcast(TmpA, tile_size* tile_size, MPI_DOUBLE, root, new_comm_row);
        
        // Matrix multiplication
        for (i = 0; i < tile_size; i++) {
            for (k = 0; k < tile_size; k++) {
                for (j = 0; j < tile_size; j++) {
                    C[i * tile_size + j] += TmpA[i * tile_size + k] * B[k * tile_size + j];
                }
            }
        }
        // receive B
        MPI_Recv(B, tile_size* tile_size, MPI_DOUBLE, (row_num+1)%p, 3, new_comm_col, MPI_STATUS_IGNORE);
    }

    // calculate sum of C for each process
	for(i=0; i<tile_size; i++) {
		for(j=0; j<tile_size; j++) {
			send_data += C[i * tile_size + j];
		}
	}

    MPI_Reduce(&send_data, &recv_data, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

	// Result validation in rank 0
	if(rank == 0) {
		double expected_sum = (pow(N, 3)*pow(N-1, 2))/2;
        fprintf(stdout, "Time Elipsed: %lf\n", MPI_Wtime() - start);
		fprintf(stderr, "Expected sum of C: \t%lf\n", expected_sum);
		fprintf(stderr, "Calculate sum of C: \t%lf\n", recv_data);
		if(abs(expected_sum-recv_data) < 0.000000001) fprintf(stderr,"Multiplication Correct\n");
		else fprintf(stderr,"Multiplication Incorrect\n");
	}

    free(A);
    free(B);
    free(C);
    free(TmpA);
    free(TmpB);
    MPI_Comm_free(&new_comm_col);
    MPI_Comm_free(&new_comm_row);

    MPI_Finalize();
    return 0;
}