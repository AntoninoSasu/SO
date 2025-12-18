#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
    int i, fd;
    char buffer[6];

    // Abrir el archivo en modo lectura/escritura, truncándolo si ya existe
    fd = open("output.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Escribir los primeros 5 ceros desde el proceso principal
    write(fd, "00000", 5);

    // Crear 9 procesos hijo
    for (i = 1; i < 10; i++) {
        if (fork() == 0) {
            /* Código del proceso hijo */
            // Cada hijo abre su propio descriptor para el archivo
            int fd_child = open("output.txt", O_RDWR);
            if (fd_child == -1) {
                perror("open (child)");
                exit(EXIT_FAILURE);
            }

            // Calcular el desplazamiento en el archivo para escribir
            off_t pos = i * 5;  // Cada proceso escribe en bloques de 5 caracteres
            lseek(fd_child, pos, SEEK_SET);

            // Escribir el número correspondiente repetido 5 veces
            sprintf(buffer, "%d%d%d%d%d", i, i, i, i, i);
            write(fd_child, buffer, 5);

            // Cerrar el archivo y terminar el proceso hijo
            close(fd_child);
            exit(EXIT_SUCCESS);
        }
    }

    // Esperar a que todos los procesos hijo terminen
    while (wait(NULL) != -1);

    // Leer el contenido del archivo y mostrarlo
    lseek(fd, 0, SEEK_SET);
    printf("File contents are:\n");
    char c;
    while (read(fd, &c, 1) > 0) {
        putchar(c);
    }
    printf("\n");

    // Cerrar el archivo y salir
    close(fd);
    exit(EXIT_SUCCESS);
}