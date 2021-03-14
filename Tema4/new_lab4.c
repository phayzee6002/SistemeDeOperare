#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

// Lab4.1
void forkls()
{
	pid_t pid = fork();

	if (pid < 0)
	{
		return errno;
	}
	else if (pid == 0)
	{
		char *argv[] = {"ls", NULL};
		execve("/usr/bin/ls", argv, NULL);
		perror(NULL);
	}
}

// Lab4.2
void collatz(int nr)
{
	pid_t pid = fork();

	if (pid < 0)
	{
		return errno;
	}

	else if (pid == 0)
	{
		while (nr > 1)
		{
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
		}
		printf("\nParent %d Me %d \n", getppid(), getpid());
	}

	else
	{
		wait(NULL);
	}

	printf("\n");
}

// Lab4.3
void ncollatz(int argc, char *argv[])
{
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

			while (nr > 1)
			{
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
}

int main(int argc, char *argv[])
{
	if (strcmp(argv[1], "forkls") == 0)
		forkls();

	else if (strcmp(argv[1], "collatz") == 0)
		collatz(atoi(argv[2]));

	else if (strcmp(argv[1], "ncollatz") == 0)
		ncollatz(argc, argv);
	return 0;
}