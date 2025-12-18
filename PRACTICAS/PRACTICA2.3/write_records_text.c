#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"

int main(int argc, char *argv[]) {

  if (argc != 2){
    fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
    return 1;
  }

  SimpleRecord records[3] = {
        {1, 3.10, "Barcelona"},
        {0, 19.77, "Madrid"},
        {2, 7.42, "Valencia"}
  };

  FILE *file = NULL;

  if ((file = fopen(argv[1], "w")) == NULL){
    err(2, "The input file %s could not be opened", argv[1]);
    exit(-1);
  }


  fclose(file);
  
  return 0;
}
