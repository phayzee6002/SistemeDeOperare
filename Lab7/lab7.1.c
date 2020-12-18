#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>

#define MAX_RESOURCES 5;
int available_resources = MAX_RESOURCES;

pthread_mutex_t mtx;

int decreaseCount(int count)
{
	pthread_mutex_lock(&mtx);
	printf("count - %d - available %d", count, available_resources);

	if (available_resources < count)
	{
		printf("\nNo Resources");
		pthread_mutex_unlock(&mtx);

		return -1;
	}
	else
	{
		available_resources = available_resources - count;
		printf("\n Got %d Resources, %d Remaining ", count, available_resources);

		pthread_mutex_unlock(&mtx);

		return 0;
	}
}

int increaseCount(int count)
{
	pthread_mutex_lock(&mtx);

	available_resources = available_resources + count;
	printf("\n Released %d Resources %d Remaining ", count, available_resources);

	pthread_mutex_unlock(&mtx);

	return 0;
}

void* aux(void* args)
{
	int count = *(int*) args;
	printf("\n Count - %d - Available - %d", count, available_resources);

	if (decreaseCount(count)!=-1)
	{
		increaseCount(count);
	}
	
	return NULL;
}

int main() 
{
	if(pthread_mutex_init(&mtx, NULL))
	{
		perror(NULL);
		return errno;
	}

	pthread_t threads[6];
	int count[6] = {2, 2, 1, 3, 2 ,5};
	
	for (int i = 0; i < 6; i++)
	{
		if (pthread_create(&threads[i], NULL, aux, &count[i]))
		{
			perror(NULL);
			return errno;
		}	
	}
	
	for (int i = 0; i < 6; i++)
	{
		if (pthread_join(threads[i], NULL))
		{
			perror(NULL);
			return errno;
		}	
	}

	pthread_mutex_destroy(&mtx);
}