#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Librería para poder usar funciones como read()
#include <unistd.h>

#define BUF_SIZE 512

void copy(int fdo, int fdd) {
	char buffer[BUF_SIZE];
	ssize_t ret;

	while ((ret = read(fdo, buffer, BUF_SIZE)) > 0) {
		if (write(fdd, buffer, ret) == -1) {
			close(fdo);
			close(fdd);
			perror("Error on writing to destination file");
			exit(-1);
		}
	}

	// Si se salio del bucle anterior por un error en el read
	if (ret == -1) {
		close(fdo);
		close(fdd);
		perror("Error on reading source file");
		exit(-1);
	}
}

int main(int argc, char* argv[]) {
	int fdo, fdd;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <orig_file_name>, %s <dest_filename>\n", argv[1], argv[2]);
		return 1;
	}

	if ((fdo = open(argv[1], O_RDONLY)) == -1) {
		perror("Source file couldn't be opened");
		return -1;
	}

	
	if ((fdd = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0660)) == -1) {
		close(fdo);
		perror("Destination file couldn't be opened");
		return -1;
	}

	copy(fdo, fdd);

	close(fdo);
	close(fdd);

	return 0;
}