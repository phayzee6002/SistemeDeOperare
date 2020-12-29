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

struct output
{
	int **permutations;
	char **encryptedArray;
};

struct word
{
	int *scrambledArray;
	char *encryptedWord;
};

void randomisation(char *word)
{
	int *arr  = (int *)malloc(strlen(word) * sizeof(int)),
		*freq = (int *)calloc(strlen(word),  sizeof(int));
	int i = 0, num;
	
	srand(time(NULL));										

	while (i != strlen(word))
	{
		num = (rand() % (strlen(word)));

		while (freq[num] == 1)
		{
			num = (rand() % (strlen(word)));
		}

		arr[i] = num;
		freq[num] = 1;
		i += 1;
	}

	char wordCpy[strlen(word)];
	strcpy(wordCpy, word);

	for (int i = 0; i < strlen(word); i++)
	{
		wordCpy[i] = word[arr[i]];
	}
	puts(wordCpy);
}

void *permutation(void *srcDat)
{
	struct data *threadDat = (struct data *)srcDat;

	for (int i = threadDat->inf; i < threadDat->supr; i++)
	{
		char *word = (threadDat->wordsArray[i]);

		randomisation(word);
	}

	return NULL;
}

int* threadDivider(int wordsCount, int nrThreads)
{
	int *wordsThreads = (int *)malloc(nrThreads * sizeof(int));
	int i = 0;

	for (; i < wordsCount % nrThreads; i++)
	{
		wordsThreads[i] = wordsCount / nrThreads + 1;
	}

	for (; i < nrThreads; i++)
	{
		wordsThreads[i] = wordsCount / nrThreads;
	}

	return wordsThreads;
}

struct data srcDat_allocate(char string[])
{
	int wordsCount = 0, counter = 0;
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
		printf("Eroare maapare fisier");
	}

	return (map_ptr);
}

int encrypt(char src[], char dstPerm[], char dst[], int nrThreads)
{
	struct stat src_struct,
				dst_struct,
				dstPerm_struct;
	struct data srcDat;
	
	int fd_src, 
		fd_dst, 
		fd_dstPerm;
	int *wordsThreads = (int *)malloc(nrThreads * sizeof(int));

	pthread_t threadID;


	// Getting the file descriptor for the necessary files
	fd_src     = openSrc  (src);
	fd_dst     = createDst(dst);
	fd_dstPerm = createDst(dstPerm);

	// Initialising the structs containing file data
	stat(src	, &src_struct);
	stat(dst	, &dst_struct);
	stat(dstPerm, &dstPerm_struct);

	// Copying the source file content into rawDat 
	char 		rawDat[src_struct.st_size];
	const char* map_ptr = mapping(fd_src, src_struct.st_size);
	strcpy(rawDat, map_ptr);

	// Formatting the rawDat string
	srcDat 		 = srcDat_allocate(rawDat);
	wordsThreads = threadDivider  (srcDat.wordsCount, nrThreads);

	// Creating threads
	srcDat.inf  = 0;
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
		srcDat.supr = srcDat.inf + wordsThreads[i + 1];
	}

	return 1;
}

int main(int argc, char *argv[])
{
	encrypt("in/test.txt", "out/iesire1.txt", "out/iesire2.txt", 3);

	return 0;
}
