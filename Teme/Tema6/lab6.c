#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct args
{
	int lin;
	int col;
	int nrCol;
	int **A;
	int **B;
	int **C;
};

void *printReverse(void *str)
{
	char *string = (char *)str;

	int i;
	for (i = strlen(string) - 1; i >= 0; i--)
	{
		printf("%c", string[i]);
	}
	printf("\n");

	return NULL;
}

void *passThread(void* str)
{
	struct args* matrix = (struct args*)str;
	int m = matrix->nrCol;
	int sum = 0;

	for (int  k = 0; k < m; k++)
	{
		sum = sum + (matrix->A[matrix->lin][k]) * (matrix->B[k][matrix->col]);
	}
	matrix->C[matrix->lin][matrix->col] = sum;

	return NULL;
}

void alocareMatrix(int ***A, int n, int m)
{
	*A = (int **)malloc(n * sizeof(int*));

	for (int i = 0; i < n; i++)
	{
		(*A)[i] = (int *)malloc(m * sizeof(int));
	}
}

void readMatrix(int **A, int n, int m)
{
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < m; j++)
		{
			A[i][j] = rand()%10;
		}
	}
}

void printMatrix(int **A, int n, int m)
{
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < m; j++)
		{
			printf("%4d ", A[i][j]);
		}

		printf("\n");
	}
}

int main(int argc, char *argv[])
{
	printf("wtf");
	char *result;
	struct args matrix;

	if (strcmp(argv[1], "printReverse") == 0)
	{
		pthread_t threadID;

		if (pthread_create(&threadID, NULL, printReverse, argv[2]))
		{
			perror(NULL);
			return errno;
		}

		if (pthread_join(threadID, &result))
		{
			perror(NULL);
			return errno;
		}

		return 0;
	}


	// ./[].exe calculMatrix A n m B x y
	else if (strcmp(argv[1], "calculMatrix") == 0)
	{
		printf("aici");

		int n = atoi(argv[3]);
		int m = atoi(argv[4]);
		int x = atoi(argv[6]);
		int y = atoi(argv[7]);

		matrix.nrCol = m;

		pthread_t threadID;


		if (m == x)
		{
			printf("inainte");
			alocareMatrix(&matrix.A, n, m);
			alocareMatrix(&matrix.B, x, y);
			alocareMatrix(&matrix.C, n, y);
			printf("dupa alocare");
			readMatrix(matrix.A, n, m);
			readMatrix(matrix.B, x, y);
			printf("dupa citire");
			/*
			! calculMatrix(matrix.A, matrix.B, matrix.C, n, m, y, &matrix);
			*/
			for (int i = 0; i < n; i++)
			{
				for(int j = 0; j < y; j++)
				{
					// printf("in calcul\n");
					matrix.col = i;
					matrix.lin = j;

					if(pthread_create(&threadID, NULL, passThread, &matrix)) 
					{
						perror(NULL);
						return errno;
					}

					if(pthread_join(threadID, &result))
					{
						perror(NULL);
						return errno;
					}
				}
			}
			// printf("dupa calcul\n");			

			printMatrix(matrix.A, n, m);
			printf("\n");
			printMatrix(matrix.B, x, y);
			printf("\n");
			printMatrix(matrix.C, n, y);

			for (int i = 0; i < n*y; i++)
			{
				pthread_join(threadID, result);
			}


			free(matrix.A);
			free(matrix.B);
			free(matrix.C);
			
			return 0;
		}

		else
		{
			printf("Matricele nu sunt compatibile\n");
			return 0;
		}
	}
}