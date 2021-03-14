#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

int n = 4, crossings = 0;
sem_t sem;
pthread_mutex_t mtx;

void* tfun(void* v)
{
    int tid = *(int*)v;

    printf("%d reached barrier\n", tid);

    pthread_mutex_lock(&mtx);
    crossings+=1;

    if (crossings==4)
    {
        pthread_mutex_unlock(&mtx);
        
        for (int i = 0; i < 4; i++)
        {
            if (sem_post(&sem))
            {
                perror(NULL);
                return errno;
            }   
        }  
    }

    else
    {
        pthread_mutex_unlock(&mtx);
        sem_wait(&mtx);
    }
    
    printf("%d crossed barrier\n", tid);
    
    return 0;
}

int main(int argc, char* argv[])
{
    if (pthread_mutex_init(&mtx, NULL))
    {
        perror(NULL);
        return errno;
    }

    if(sem_init(&sem, 0, 0))
    {
        perror(NULL);
        return errno;
    }

    pthread_t threads[4];
    int val[4] = {0, 1, 2, 3};

    for (int i = 0; i < 4; i++)
    {
        if(pthread_create(&threads[i], NULL, tfun, &val[i]))
        {
            perror(NULL);
            return errno;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        if(pthread_join(&threads[i], NULL))
        {
            perror(NULL);
            return errno;
        }
    }

    return 0;
}