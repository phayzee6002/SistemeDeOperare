#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void shmcollatz(int argc, char* argv[]) {
	const char shm_name[] = "lab5_shm";
	const int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (shm_fd < 0) {
		perror(NULL);
		return errno;
	}

	const size_t shm_size = getpagesize() * (argc-2);
	if(ftruncate(shm_fd, shm_size)==-1) {
		perror(NULL);
		shm_unlink(shm_name);
		return errno;
	}

	const void* shm_ptr = mmap(0, shm_size, PROT_READ, MAP_SHARED, shm_fd,0);
	if(shm_ptr == MAP_FAILED) {
		perror(NULL);
		shm_unlink(shm_name);
		return errno;
	}

	int index = 2;
	pid_t pid;

	for (int i = 2; i < argc; i++)
	{
		pid = fork();

		if (pid < 0)
		{
			return errno;
		}

		else if (pid == 0)
		{
			int nr = atoi(argv[i]);
			int* sharedMem = mmap(0, shm_size, PROT_READ, MAP_SHARED, shm_fd, (i-2)*getpagesize());

			

			while (nr > 1)
			{
				int counter = 0;
				if (nr % 2 == 0)
				{
					nr /= 2;
				}

				else if (nr % 2 == 1)
				{
					nr = nr * 3 + 1;
				}
				printf("%d", nr);
				printf(" ");
				// sharedMem[counter] = nr;
				counter += 1;
			}

			printf("Parent %d Me %d \n", getppid(), getpid());
			exit(0);
		}
	}

	for (int i = 2; i < argc; i++)
	{
		if (pid>0)
		{
			wait(NULL);
		}
	}

	// shm_unlink(shm_name);
	// munmap(shm_ptr,shm_size);
}



int main(int argc, char *argv[]) {
	int index = 2;

	if (strcmp(argv[1], "shmcollatz") == 0)
	{
		shmcollatz(argc, argv);
    }

  return 0;
}