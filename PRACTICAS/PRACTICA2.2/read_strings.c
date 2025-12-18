#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor
 *
 * The loadstr() function must allocate memory from the heap to store
 * the contents of the string read from the FILE.
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc())
 *
 * Returns: !=NULL if success, NULL if error
 */

#define BUF 10

void bufferResize(char **buf, int *elems) {
  if (*buf == NULL)
    *buf = (char *)malloc(*elems);
  else {
    char *aux = (char *)malloc(*elems);
    memcpy(aux, *buf, *elems);
    free(*buf);
    *buf = (char *)malloc(*elems + BUF);
    memcpy(*buf, aux, *elems);
    free(aux);
    *elems += BUF;
  }
}

char *loadstr(FILE *file) {
  // leemos una cantidad predeterminada de bytes, preferiblemente baja
  // luego leemos esos bytes y si encontramos un null terminator en nuestra
  // secuencia hacemos fseek hacia atras hasta el byte del null terminator si no
  // lo encontramos, volvemos a iterar por el archivo, y repetimos lo de arriba

  char buf[BUF], *endBuf = NULL;
  char ch = malloc(sizeof(char));
  int i = 0, totalSizeBuf = BUF, elemsEndBuf = 0, bytesRead = 0;
  short wordComplete = 0;

  if (file == NULL)
    return NULL;

  while (!wordComplete) { // mientras no tengamos la palabra completa

    bytesRead = fread(buf, sizeof(char), BUF, file);
    i = 0;

    while (!wordComplete && i < bytesRead) { // iterar por cada buffer
      ch = buf[i];
      wordComplete = ch == EOF || ch == '\0';
      i++;
    }
    // asignamos nuevo espacio al buf
    bufferResize(&endBuf, &totalSizeBuf);
    for (int j = 0; j < i; j++)
      endBuf[j + elemsEndBuf] = buf[j];

    elemsEndBuf += i;
  }

  // devolvemos el puntero del archivo al siguiente byte despues del null
  // terminator (si no lo hemos encontrado, no hace nada).
  fseek(file, i - bytesRead, SEEK_CUR);

  return wordComplete ? endBuf : NULL;
}

int main(int argc, char *argv[]) {
  if (argc != 2)
    fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);

  FILE *file = NULL;

  if ((file = fopen(argv[1], "r")) == NULL)
    err(2, "The input file %s could not be opened", argv[1]);

  char *str;
  while ((str = loadstr(file)) != NULL) {

    printf("%s\n", str);
  }
  fclose(file);
  
  return 0;
}
