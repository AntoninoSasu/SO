#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      
#include <unistd.h>     
#include <sys/types.h>  
#include <sys/stat.h>   
#include <errno.h>    

void mostrar_contenido(int fd) {
    char buffer;
    ssize_t leidos;

    while ((leidos = read(fd, &buffer, 1)) > 0) {
        write(STDOUT_FILENO, &buffer, 1); 
    }

    if (leidos < 0) {
        perror("Error leyendo el archivo");
        close(fd);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    int opt;
    int n = 0;             
    int flag_e = 0;        
    char *filename = NULL; 

    
    while ((opt = getopt(argc, argv, "n:e")) != -1) {
        switch (opt) {
            case 'n':
                n = atoi(optarg); 
                break;
            case 'e':
                flag_e = 1; 
                break;
            default:
                fprintf(stderr, "Uso: %s [-n N] [-e] <archivo>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        filename = argv[optind];
    } else {
        fprintf(stderr, "Error: Se debe especificar un archivo.\n");
        exit(EXIT_FAILURE);
    }

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error abriendo el archivo");
        exit(EXIT_FAILURE);
    }

    if (flag_e) {
        if (lseek(fd, -n, SEEK_END) < 0) {
            perror("Error posicionando el marcador");
            close(fd);
            exit(EXIT_FAILURE);
        }
    } else {
        if (lseek(fd, n, SEEK_SET) < 0) {
            perror("Error posicionando el marcador");
            close(fd);
            exit(EXIT_FAILURE);
        }
    }

    mostrar_contenido(fd);
    close(fd);

    return 0;
}