/**
 * @file 	Proiect.c
 * @author 	Leonard Vlaicu
 * @brief 	Program for encryption/ decryption using pseudo-random numbers
 * @version 0.1
 * @date 2021-01-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */

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
#include <stdbool.h>

/**
 * @brief Struct for holding info about file data
 * 
 */
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

/**
 * @brief Struct for holding info about decryption data
 * 
 */
struct decrypt
{
	struct data srcDat;
	struct data srcPermDat;
};

/**
 * @brief Struct for holding info about encryption data
 * 
 */
struct encrypt
{
	struct data srcDat;
};

/**
 * @brief Generates the pseudo-random permutation for each word in the file
 * 
 * @param srcDat holds the information about the source file
 * @return void* 
 */
void *permutation(void *srcDat)
{
	struct encrypt *threadDat = (struct encrypt *)srcDat;

	// Iterating through every word from the thread
	for (int i = threadDat->srcDat.inf; i < threadDat->srcDat.supr; i++)
	{
		char *word = (threadDat->srcDat.wordsArray[i]);
		char wordCpy[strlen(word)];
		strcpy(wordCpy, word);

		// wordKey is the key for a word decryption
		// tmpFreq is a temporary buffer
		int *wordKey = (int *)malloc(strlen(word) * sizeof(int)),
			*tmpFreq = (int *)calloc(strlen(word), sizeof(int));
		int i = 0, num;

		srand(time(NULL));

		// Generating a random sequence of integers from 0-strlen(word)
		while (i != strlen(word))
		{
			num = (rand() % (strlen(word)));

			while (tmpFreq[num] == 1)
			{
				num = (rand() % (strlen(word)));
			}

			wordKey[i] = num;
			tmpFreq[num] = 1;
			i += 1;
		}

		//Encrypting the word
		for (int i = 0; i < strlen(word); i++)
		{
			wordCpy[i] = word[wordKey[i]];
		}

		printf("%s ", wordCpy);
		for (int i = 0; i < strlen(word); i++)
		{
			printf("%d ", wordKey[i]);
		}
		printf(" \n");
	}
}

/**
 * @brief Decryps the words using the generated permutations
 * 
 * @param decryptDat Holds information about the source files
 * @return void* 
 */
void *unlock(void *decryptDat)
{
	struct decrypt *threadDat = (struct decrypt *)decryptDat;

	for (int i = threadDat->srcDat.inf; i < threadDat->srcDat.supr; i++)
	{
		int k = 0;
		char *permDat = threadDat->srcPermDat.wordsArray[i];
		char *word 	  = threadDat->srcDat.wordsArray[i];
		char wordCpy[strlen(threadDat->srcDat.wordsArray[i])];
		strcpy(wordCpy, word);
		char *pch;
		char **Array;
		pch = strtok(permDat, " ");
		while (pch != NULL)
		{
			Array[k] = pch;
			pch = strtok(NULL, " ");
			puts(Array[k]);
			k += 1;
		}

		for (int j = 0; j < strlen(wordCpy); j++)
		{
			for (int k = 0; k < strlen(wordCpy); k++)
			{
				if (atoi(Array[k]) == j)
				{
					wordCpy[atoi(Array[k])] = word[k];
				}
			}
		}
		puts(wordCpy);
		puts("\n");
	}
}
/*
	!word = "cihr"  si Array= ['2' '1' '3' '0'] si newWord = [r i c h]
*/

/**
 * @brief Divides the number of words to the number of threads accordingly
 * 
 * @param wordsCount holds the number of words
 * @param nrThreads hold the number of threads
 * @return int* which contains the array with the number of words per thread
 */
int *threadDivider(int wordsCount, int nrThreads)
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

/**
 * @brief formats the source data into separate words
 * 
 * @param rawDat holds the raw Data from the source file
 * @return struct data which holds the number of words and the words
 */
struct data allocateDat(char rawDat[])
{
	int wordsCount = 0, counter = 0;
	char delim[] = "\n";
	char **wordsArray;
	struct data srcDat;

	for (int i = 0; i < strlen(rawDat); i++)
	{
		if (rawDat[i] == '\n')
		{
			wordsCount += 1;
		}
	}

	wordsCount += 1;
	wordsArray = (char **)malloc(wordsCount * sizeof(char *));

	char *ptr = strtok(rawDat, delim);
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

/**
 * @brief opens source file
 * 
 * @param src holds the PATH to the source file
 * @return int which holds the file_descriptor
 */
int openSrc(char src[])
{
	int fd_src = open(src, O_RDWR, S_IRWXU);
	if (fd_src == -1)
	{
		perror("Eroare deschidere fisier sursa");
		exit(0);
	}

	return fd_src;
}

/**
 * @brief opens/ creates destination file
 * 
 * @param dst hold the PATH to the file to be opened
 * @return int which holds the file_descriptor
 */
int createDst(char dst[])
{
	int fd_dst = open(dst, O_RDWR | O_CREAT, S_IRWXU);
	if (fd_dst == -1)
	{
		perror("Eroare creare/ deschidere destinatie");
		exit(0);
	}

	return fd_dst;
}

/**
 * @brief maps the source file into memory for faster I/O operations
 * 
 * @param fd_src holds the file_descriptor of the source file
 * @param sz holds the size of the file to be mapped
 * @return const char* which holds the mapped content of the file
 */
const char *mapping(int fd_src, int sz)
{
	const char map_name[] = "proiect_shm";

	const size_t map_size = sz;

	const char *map_ptr = mmap(0, map_size, PROT_READ, MAP_SHARED, fd_src, 0);
	if (map_ptr == MAP_FAILED)
	{
		perror(NULL);
		shm_unlink(map_name);
		fprintf(stderr, "Eroare mapare fisier");

		exit(0);
	}

	return (map_ptr);
}

int encrypt(char src[], char dstPerm[], char dst[], int nrThreads)
{
	struct stat src_struct,
				dst_struct,
				dstPerm_struct;
	struct data srcDat;
	struct encrypt encryptDat;

	int fd_src,
		fd_dst,
		fd_dstPerm;
	int *wordsThreads = (int *)malloc(nrThreads * sizeof(int));

	pthread_t threadID;

	// Getting the file descriptor for the necessary files
	fd_src = openSrc(src);

	// Initialising the structs containing file data
	stat(src	, &src_struct);
	stat(dst	, &dst_struct);
	stat(dstPerm, &dstPerm_struct);

	// Copying the source file content into rawDat
	char rawDat[src_struct.st_size];
	const char *map_ptr = mapping(fd_src, src_struct.st_size);

	strcpy(rawDat, map_ptr);

	// Formatting the rawDat string
	srcDat			= allocateDat(rawDat);
	wordsThreads	= threadDivider(srcDat.wordsCount, nrThreads);

	encryptDat.srcDat		= srcDat;
	encryptDat.srcDat.inf	= 0;
	encryptDat.srcDat.supr	= wordsThreads[0];

	for (int i = 0; i < nrThreads; i++)
	{
		pthread_create	(&threadID, NULL, permutation, &encryptDat.srcDat);
		pthread_join	( threadID, NULL);

		encryptDat.srcDat.inf	= encryptDat.srcDat.supr;
		encryptDat.srcDat.supr	= encryptDat.srcDat.inf + wordsThreads[i + 1];
	}

	return 1;
}

int decrypt(char src[], char srcPerm[], char dst[], int nrThreads)
{
	struct stat src_struct,
				srcPerm_struct,
				dst_struct;
	struct data srcDat, srcPermDat;
	struct decrypt decryptDat;

	int fd_src,
		fd_srcPerm,
		fd_dst;
	int *wordThreads = (int *)malloc(nrThreads * sizeof(int));

	pthread_t threadID;

	fd_src		= openSrc(src);
	fd_srcPerm	= openSrc(srcPerm);

	stat(src	, &src_struct);
	stat(srcPerm, &srcPerm_struct);
	stat(dst	, &dst_struct);

	char src_rawDat[src_struct.st_size];
	char srcPerm_rawDat[srcPerm_struct.st_size];

	char *src_ptr		= mapping(fd_src, src_struct.st_size);
	char *srcPerm_ptr	= mapping(fd_srcPerm, srcPerm_struct.st_size);

	// srcPerm_ptr = trimWhitespace(srcPerm_rawDat);
	strcpy(src_rawDat	 , src_ptr);
	strcpy(srcPerm_rawDat, srcPerm_ptr);

	srcDat		= allocateDat(src_rawDat);
	srcPermDat	= allocateDat(srcPerm_rawDat);
	wordThreads	= threadDivider(decryptDat.srcDat.wordsCount, nrThreads);

	decryptDat.srcDat		= srcDat;
	decryptDat.srcPermDat	= srcPermDat;

	decryptDat.srcDat.inf	= 0;
	decryptDat.srcDat.supr	= wordThreads[0];
	printf("inainte de thread\n");

	for (int i = 0; i < nrThreads; i++)
	{
		pthread_create	(&threadID, NULL, unlock, &decryptDat);
		pthread_join	( threadID, NULL);

		decryptDat.srcDat.inf	= decryptDat.srcDat.supr;
		decryptDat.srcDat.supr	= decryptDat.srcDat.inf + wordThreads[i + 1];
	}

	return 1;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("ERROR\nWrong Command\nCommand usage: ./exec <encrypt> / <decrypt>\n");

		return -1;
	}

	if (strcmp(argv[1], "encrypt") == 0)
	{
		char *src = "encrypt/src";
		char *dst = "encrypt/dst";
		char *dstPerm = "encrypt/dstPerm";

		encrypt(src, dst, dstPerm, 3);
	}

	else if (strcmp(argv[1], "decrypt") == 0)
	{
		printf("decrypt\n");

		char *src = "decrypt/src";
		char *srcPerm = "decrypt/srcPerm";
		char *dst = "decrypt/dst";

		decrypt(src, srcPerm, dst, 3);
	}

	else
	{
		fprintf(stderr, "ERROR\nWrong Command\nCommand usage: ./exec <encrypt> / <decrypt>\n");
	}

	return 0;
}
