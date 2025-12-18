#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#define BLOCK_SIZE 512 // Tamaño de bloque para archivos regulares
void copy(int fdo, int fdd) {
    int bytesRead, bytesWritten;
    char buffer[BLOCK_SIZE];

    while ((bytesRead = read(fdo, buffer, BLOCK_SIZE)) > 0) {
        bytesWritten = write(fdd, buffer, bytesRead);
        if (bytesWritten != bytesRead) {
            perror("Error escribiendo en el archivo destino");
            close(fdo);
            close(fdd);
            exit(EXIT_FAILURE);
        }
    }

    if (bytesRead < 0) {
        perror("Error leyendo del archivo origen");
        close(fdo);
        close(fdd);
        exit(EXIT_FAILURE);
    }
}
// Función para copiar archivos regulares
void copy_regular(char *orig, char *dest) {
    int fdo, fdd;

    // Abrir archivo origen en modo lectura
    fdo = open(orig, O_RDONLY);  /*int open(const char *pathname, int flags, mode_t mode); Parámetros:

pathname: Ruta del archivo a abrir.
flags: Modos de apertura. Algunos de los más comunes son:
O_RDONLY: Solo lectura.
O_WRONLY: Solo escritura.
O_RDWR: Lectura y escritura.
O_CREAT: Crea el archivo si no existe.
O_TRUNC: Trunca el archivo a tamaño cero si ya existe.
mode: Permisos del archivo (solo si se usa O_CREAT). Ejemplo: 0660 (lectura/escritura para el propietario y grupo).*/
    if (fdo < 0) {
        perror("Error abriendo el archivo origen");
        exit(EXIT_FAILURE);
    }

    // Abrir o crear archivo destino en modo escritura
    fdd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0660);
    if (fdd < 0) {
        perror("Error abriendo el archivo destino");
        close(fdo);
        exit(EXIT_FAILURE);
    }

    // Copiar contenido del archivo
    copy(fdo, fdd);

    // Cerrar descriptores de archivo
    close(fdo);
    close(fdd);

    printf("Archivo regular copiado: %s -> %s\n", orig, dest);
}
void copy_link(char *orig, char *dest) {
    struct stat sb; // Estructura para almacenar información del archivo

    // Paso 1: Obtener información sobre el enlace simbólico
    if (lstat(orig, &sb) < 0) {  // Usar lstat para no seguir el enlace simbólico  
     /*int lstat(const char *pathname, struct stat *buf);  
     Parámetros:

pathname: Ruta del archivo.
buf: Puntero a una estructura stat donde se almacenará la información del archivo.
Devuelve: 0 si tiene éxito, -1 si hay un error.*/

        perror("Error obteniendo información del enlace simbólico");
        exit(EXIT_FAILURE); // Salir si ocurre un error
    }

    // Paso 2: Reservar memoria para almacenar la ruta apuntada por el enlace
    char *buf = malloc(sb.st_size + 1); // Reservar memoria suficiente (+1 para '\0')
    if (!buf) {  // Verificar que la memoria se asignó correctamente
        perror("Error reservando memoria");
        exit(EXIT_FAILURE);
    }

    // Paso 3: Leer la ruta del enlace simbólico
    ssize_t len = readlink(orig, buf, sb.st_size); // Leer el destino del enlace
    if (len < 0) {  // Manejar errores de readlink
        perror("Error leyendo el enlace simbólico");
        free(buf); // Liberar la memoria reservada antes de salir
        exit(EXIT_FAILURE);
    }
    buf[len] = '\0'; // Asegurarse de que la cadena termina en '\0'

    // Paso 4: Crear un nuevo enlace simbólico que apunte al mismo destino
    if (symlink(buf, dest) < 0) {  // Crear el enlace simbólico
        perror("Error creando el enlace simbólico");
        free(buf); // Liberar memoria si ocurre un error
        exit(EXIT_FAILURE);
    }

    // Paso 5: Imprimir un mensaje indicando que el enlace fue copiado
    printf("Enlace simbólico copiado: %s -> %s\n", orig, buf);

    // Paso 6: Liberar la memoria asignada
    free(buf);
}
int main(int argc, char *argv[]) {
    struct stat sb;

    // Verificar los argumentos de entrada
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <archivo_origen> <archivo_destino>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Obtener información del archivo origen
    if (lstat(argv[1], &sb) < 0) {
        perror("Error obteniendo información del archivo origen");
        exit(EXIT_FAILURE);
    }

    // Verificar el tipo de archivo y realizar la copia correspondiente
    if (S_ISLNK(sb.st_mode)) {
        copy_link(argv[1], argv[2]);
    } else if (S_ISREG(sb.st_mode)) {
        copy_regular(argv[1], argv[2]);
    } else {
        fprintf(stderr, "Error: Tipo de archivo no soportado\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}