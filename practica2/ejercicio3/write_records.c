#include <stdio.h>
#include <stdlib.h>
#include "simple_record.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s fichero_salida.txt\n", argv[0]);
        return 1;
    }

    SimpleRecord records[3] = {
        {1, 3.10, "Barcelona"},
        {0, 19.77, "Madrid"},
        {2, 7.42, "Valencia"}
    };

    FILE *f = fopen(argv[1], "w");
    if (!f) {
        perror("fopen");
        return 1;
    }

    for (int i = 0; i < 3; i++) {
        fprintf(f, "%d %.2f %s\n", records[i].id, records[i].value, records[i].label);
    }

    fclose(f);
    printf("La escritura ha finalizado correctamente\n");
    return 0;
}

