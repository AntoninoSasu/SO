#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define BLOCK_SIZE 512

void copy(int fdo, int fdd)
{

	char buffer[BLOCK_SIZE];
    ssize_t leidos, escritos;
    while((leidos = read(fdo, buffer, sizeof(buffer))) > 0){
		escritos = write(fdd, buffer, leidos);
		if (escritos != leidos) {
			perror("Error writing to file");
			exit(1);
        }
	}

}

int main(int argc, char *argv[])
{
	int fdo = open(argv[1], O_RDONLY, 0644);
	int fdd = open(argv[2], O_WRONLY | O_CREAT | O_APPEND, 0644);
	copy(fdo, fdd);
	close(fdo);
	close(fdd);

	return 0;
}
