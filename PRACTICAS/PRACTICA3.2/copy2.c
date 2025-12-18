#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
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

	if (ret == -1) {
		close(fdo);
		close(fdd);
		perror("Error on reading source file");
		exit(-1);
	}
}

void copy_regular(char* orig, char* dest) {
	int fdo, fdd;

	if ((fdo = open(orig, O_RDONLY)) == -1) {
		perror("Source file couldn't be opened");
		exit(-1);
	}

	if ((fdd = open(dest, O_WRONLY | O_TRUNC | O_CREAT, 0660)) == -1) {
		close(fdo);
		perror("Destination file couldn't be opened");
		exit(-1);
	}

	copy(fdo, fdd);

	close(fdo);
	close(fdd);
}

// Path del origen y del destino
void copy_link(char* orig, char* dest, off_t size) {
	size++;
	char* buffer = malloc(size);

	if (readlink(orig, buffer, size) == -1) {
		free(buffer);
		perror("Source link couldn't be read");
		exit(-1);
	}

	buffer[size - 1] = '\0';
	
	if (symlink(buffer, dest) == -1) {
		free(buffer);
		perror("Destination link couldn't be created");
		exit(-1);
	}

	free(buffer);
}

int main(int argc, char* argv[]) {

	//Guardo todo aqui sobre el lstat
	struct stat fdo_stats;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <orig_file_name>, %s <dest_filename>\n", argv[1], argv[2]);
		return 1;
	}

	if (lstat(argv[1], &fdo_stats) == -1) {
		perror("Couldn't retrieve file information from source file");
		return -1;
	}

	//Compruebo si es simbolico o duro
	if (S_ISREG(fdo_stats.st_mode)) {
		copy_regular(argv[1], argv[2]);
	} else if (S_ISLNK(fdo_stats.st_mode)) {
		copy_link(argv[1], argv[2], fdo_stats.st_size);
	} else {
		printf("Source file must be a POSIX regular file or symbolic link");
		return 1;
	}

	return 0;
}