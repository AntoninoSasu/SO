#include <stdio.h>
#include <stdlib.h>
#include "simple_record.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s fichero_entrada.bin\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    SimpleRecord r;

    while (fread(&r, sizeof(SimpleRecord), 1, f) == 1) {
        printf("ID:%d, Valor:%.2f, Etiqueta: '%s'\n", r.id, r.value, r.label);
    }

    fclose(f);
    return 0;
}

