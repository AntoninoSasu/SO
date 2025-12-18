#include <stdio.h>
#include <stdlib.h>
#include "simple_record.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s fichero_salida.bin\n", argv[0]);
        return 1;
    }

    SimpleRecord records[3] = {
        {1, 3.10, "Barcelona"},
        {0, 19.77, "Madrid"},
        {2, 7.42, "Valencia"}
    };

    FILE *f = fopen(argv[1], "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    for (int i = 0; i < 3; i++) {
        size_t w = fwrite(&records[i], sizeof(SimpleRecord), 1, f);
        if (w != 1) {
            perror("fwrite");
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    printf("La escritura ha finalizado correctamente\n");
    return 0;
}

