#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main() {
    int fd1;
    char c;
    char buffer[6];

    fd1 = open("output.txt", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    write(fd1, "00000", 5);

    for (int i = 1; i < 10; i++) {
        if (!fork()) {            
            /* Child */
            close(fd1);
            fd1 = open("output.txt", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
            lseek(fd1, i * 6, SEEK_SET);

            sprintf(buffer, "%d", i * 11111);
            write(fd1, buffer, 5);

            close(fd1);
            exit(0);
        }
    }

	// wait for all childs to finish
    while (wait(NULL) != -1);

    lseek(fd1, 0, SEEK_SET);

    printf("File contents are:\n");
    while (read(fd1, &c, 1) > 0) {
        printf("%c", (char)c);
    }
    printf("\n");

    close(fd1);
    exit(0);
}
