#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

/*
 * loadstr()
 * ----------
 * Lee del fichero una cadena terminada en carácter nulo ('\0').
 * 
 * - Lee el fichero byte a byte usando fread()
 * - Cuenta cuántos bytes ocupa la cadena (incluyendo el '\0')
 * - Reserva memoria suficiente
 * - Retrocede el puntero del fichero y vuelve a leer la cadena completa
 *
 * Devuelve:
 *  - Un puntero a la cadena leída (memoria dinámica)
 *  - NULL si se alcanza el fin de fichero o hay error
 *
 * IMPORTANTE:
 *  La memoria devuelta debe ser liberada por el llamador con free().
 */
char *loadstr(FILE *file)
{
    int bytes_leidos = 0;        // Número de bytes leídos en cada fread
    int longitud_cadena = 0;     // Longitud total de la cadena
    char *cadena = malloc(sizeof(char));  // Buffer temporal de 1 byte

    /* Comprobación de error de malloc */
    if (cadena == NULL) {
        return NULL;
    }

    /*
     * Leer el fichero byte a byte hasta encontrar el carácter '\0'
     */
    do {
        bytes_leidos = fread(cadena, 1, 1, file);

        /* Si no se pudo leer ningún byte, se asume EOF o error */
        if (bytes_leidos <= 0) {
            free(cadena);
            return NULL;
        }

        longitud_cadena += bytes_leidos;

    } while (*cadena != '\0');

    /*
     * Reservar memoria para la cadena completa
     * (incluyendo el carácter nulo final)
     */
    char *str = malloc(longitud_cadena * sizeof(char));
    if (str == NULL) {
        free(cadena);
        return NULL;
    }

    /*
     * Volver atrás en el fichero para leer la cadena completa de una vez
     */
    fseek(file, -longitud_cadena, SEEK_CUR);

    /*
     * Leer la cadena completa en el buffer definitivo
     */
    fread(str, 1, longitud_cadena, file);

    /* Liberar el buffer temporal */
    free(cadena);

    return str;
}

int main(int argc, char *argv[])
{
    /* Comprobar número de argumentos */
    if (argc < 2) {
        fprintf(stderr, "Usage: ./read_strings <input_file>\n");
        exit(1);
    }

    /* Abrir el fichero de entrada */
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        err(2, "The input file %s could not be opened", argv[1]);
    }

    char *cadena;

    /*
     * Leer cadenas del fichero hasta que loadstr() devuelva NULL
     */
    while ((cadena = loadstr(file)) != NULL) {

        /*
         * Imprimir la cadena leída
         * Se usa strlen(cadena)+1 para incluir el '\0'
         */
        fwrite(cadena, sizeof(char), strlen(cadena) + 1, stdout);
        printf("\n");

        /* Liberar memoria de la cadena */
        free(cadena);
    }

    /* Cerrar el fichero */
    fclose(file);

    return 0;
}
