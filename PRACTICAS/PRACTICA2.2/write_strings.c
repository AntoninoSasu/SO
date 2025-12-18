#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {

  if (argc < 3) { // debe haber al menos el fichero de output y un string a meter
    printf("Did not receive the minimum amount of arguments. (Sent %i)", argc);
    exit(1);
  }

  // Opcion "w" para escribir, "wb" seria escribir en binario
  FILE *file = fopen(argv[1], "w");
  if (file == NULL)
    err(EXIT_FAILURE, "File %s couldn't be opened.", argv[1]);

  // Empezmos a leer las cadenas para escribir en el archivo
  int i = 2;
  char *word = NULL;

  
  while (i < argc) {
    word = argv[i];
    // "+1" porque se inclute el "/0"
    fwrite(word, sizeof(char), strlen(word) + 1, file); // imprimimos cada palabra incluyendo el null terminator
    i++;
  }

  fclose(file);

  return 0;
}
