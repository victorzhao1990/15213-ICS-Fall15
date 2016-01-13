// Xinyun (Victor) Zhao Andrew Id: xinyunzh
/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"
#include "contracts.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. The REQUIRES and ENSURES from 15-122 are included
 *     for your convenience. They can be removed if you like.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    REQUIRES(M > 0);
    REQUIRES(N > 0);

    int i, j, k, l, m, n;

    switch (N) {
      case 32:
	for (i = 0; i < N; i = i + 8) {
	  for (j = 0; j < M; j = j + 8) {
	    for (k = i; k < i + 8; k++) {
	      for (l = j; l < j + 8; l++) {
		B[l][k] = A[k][l];
  	      }
  	    }
      	  }
	}
	break;

      case 64:
	for (i = 0; i < N; i = i + 8) {
	  for (j = 0; j < M; j = j + 8) {
	    for (k = i; k < i + 8; k = k + 4) {
	      for (l = j; l < j + 8; l = l + 4) {
		for (m = k; m < k + 4; m++) {
		  for (n = l; n < l + 4; n++) {		
		    B[n][m] = A[m][n];
		  }
		}
	      }
	    }
	  }
	}
        break;


      case 67:
        for (i = 0; i < N; i = i + 21) {
          for (j = 0; j < M; j = j + 21) {
  	    for (k = i; k < i + 21; k++) {
  	      for (l = j; l < j + 21; l++) {
  	        if ((k < N) && (l < M)) {
  	          B[l][k] = A[k][l];
                }
              }
            }
          }
        }
        break;

      default:
        return;
      }

    ENSURES(is_transpose(M, N, A, B));
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    REQUIRES(M > 0);
    REQUIRES(N > 0);

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

    ENSURES(is_transpose(M, N, A, B));
}


/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
