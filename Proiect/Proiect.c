#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>

int global = 0;

struct data
{
	char **wordsArray;
	int wordsCount;
	int supr;
	int inf;
	int fd_src;
	int fd_dst;
	int fd_dstPerm;
};

void *permutation(void *srcDat)
{
	struct data *threadDat = (struct data *)srcDat;

	for (int i = threadDat->inf; i < threadDat->supr; i++)
	{
		char* cpy = (threadDat->wordsArray[i]);
		char cp[strlen(threadDat->wordsArray[i])];
		strcpy(cp, cpy);

		printf("%s ", cp[0]);
	}

	return NULL;
}

struct data allocate(char string[])
{
	int wordsCount = 0, init_sz = strlen(string), counter = 0;
	char delim[] = "\n";
	char **wordsArray;
	struct data srcDat;

	for (int i = 0; i < strlen(string); i++)
	{
		if (string[i] == '\n')
		{
			wordsCount += 1;
		}
	}

	wordsCount += 1;
	wordsArray = (char **)malloc(wordsCount * sizeof(char *));

	char *ptr = strtok(string, delim);
	while (ptr != NULL)
	{
		wordsArray[counter] = ptr;
		ptr = strtok(NULL, delim);
		counter += 1;
	}

	srcDat.wordsArray = wordsArray;
	srcDat.wordsCount = wordsCount;

	return srcDat;
}

int openSrc(char src[])
{
	int fd_src = open(src, O_RDWR, S_IRWXU);
	if (fd_src == -1)
	{
		perror("Eroare deschidere fisier sursa");
		return errno;
	}

	return fd_src;
}

int createDst(char dst[])
{
	int fd_dst = open(dst, O_RDWR | O_CREAT, S_IRWXU);
	if (fd_dst == -1)
	{
		perror("Eroare creare/ deschidere destinatie");
		return errno;
	}

	return fd_dst;
}

const char *mapping(int fd_src, int sz)
{
	const char map_name[] = "proiect_shm";

	const size_t map_size = sz;

	const char *map_ptr = mmap(0, map_size, PROT_READ, MAP_SHARED, fd_src, 0);
	if (map_ptr == MAP_FAILED)
	{
		perror(NULL);
		shm_unlink(map_name);
		return errno;
	}

	return (map_ptr);
}

void encrypt(char src[], char dstPerm[], char dst[], int nrThreads)
{
	struct stat src_struct, dst_struct, dstPerm_struct;
	struct data srcDat;
	int fd_src, fd_dst, fd_dstPerm;

	const char *map_ptr;
	pthread_t threadID;

	fd_src = openSrc(src);
	fd_dst = createDst(dst);
	fd_dstPerm = createDst(dstPerm);

	stat(src, &src_struct);
	stat(dst, &dst_struct);
	stat(dstPerm, &dstPerm_struct);

	map_ptr = mapping(fd_src, src_struct.st_size);

	char string[src_struct.st_size];
	strcpy(string, map_ptr);

	srcDat = allocate(string);
 
	int *wordsThreads = (int *)malloc(nrThreads * sizeof(int));
	int i = 0;

	for (; i < srcDat.wordsCount % nrThreads; i++)
	{
		wordsThreads[i] = srcDat.wordsCount / nrThreads + 1;
	}

	for (; i < nrThreads; i++)
	{
		wordsThreads[i] = srcDat.wordsCount / nrThreads;
	}

	srcDat.inf = 0;
	srcDat.supr = wordsThreads[0];

	for (int i = 0; i < nrThreads; i++)
	{
		if (pthread_create(&threadID, NULL, permutation, &srcDat))
		{
			perror(NULL);
			return errno;
		}

		if (pthread_join(threadID, NULL))
		{
			perror(NULL);
			return errno;
		}

		srcDat.inf = srcDat.supr;
		srcDat.supr = srcDat.inf + wordsThreads[i+1];
	}
}

int main(int argc, char *argv[])
{
	encrypt("in/test.txt", "out/iesire1.txt", "out/iesire2.txt", 3);

	return 0;
}
