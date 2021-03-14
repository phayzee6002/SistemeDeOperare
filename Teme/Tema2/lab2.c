#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#define SIRWXU 0000700

int mycp(char copy[], char paste[]) {
    int errno, read_copy = 2, bucla_count = 0;
    struct stat copy_struct, paste_struct;
    stat(copy, &copy_struct);
    char text[4096];

    // #region Deschidere Fisier Origine
    int fd_copy = open(copy, O_RDONLY, SIRWXU);
    if (fd_copy == -1) {
        perror("Eroare deschidere origine");

        return errno;
    }

	// #region Deschidere/ Creare Fisier Destinatie
    int fd_paste = open(paste, O_WRONLY | O_CREAT, SIRWXU);
    if (fd_paste == -1)
    {
        perror("Eroare deschidere/ creare destinatie");

        return errno;
    }
    // #endregion

    // #endregion

    // #region Citire Fisier Origine
	while (read_copy!=0) {
		bucla_count += 1;
		read_copy = read(fd_copy, text, 4096);

		if (read_copy < 0) {			
			perror("Eroare citire origine");
			return errno;
		}
    	
		write(fd_paste, text, read_copy);
	}
	
    // #endregion

    close(fd_copy);
    close(fd_paste);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        perror("Command usage: <src> <dst>");
        return -1;
    }

    mycp(argv[1], argv[2]);

    return 0;
}