#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>   // getopt
#include "simple_record.h"

static int read_record_text(FILE *f, SimpleRecord *r) {
    return fscanf(f, "%d %lf %15s", &r->id, &r->value, r->label) == 3;
}
static int read_record_bin(FILE *f, SimpleRecord *r) {
    return fread(r, sizeof(SimpleRecord), 1, f) == 1;
}
static int write_record_text(FILE *f, const SimpleRecord *r) {
    return fprintf(f, "%d %.2f %s\n", r->id, r->value, r->label) > 0;
}
static int write_record_bin(FILE *f, const SimpleRecord *r) {
    return fwrite(r, sizeof(SimpleRecord), 1, f) == 1;
}

static void usage(const char *prog) {
    fprintf(stderr, "Uso: %s [-i t|b] [-o t|b] entrada salida\n", prog);
    fprintf(stderr, "  Usa '-' para stdin/stdout (bonus)\n");
}

int main(int argc, char *argv[]) {
    char in_fmt = 't';
    char out_fmt = 't';

    int opt;
    while ((opt = getopt(argc, argv, "i:o:")) != -1) {
        switch (opt) {
        case 'i':
            if (optarg[0] != 't' && optarg[0] != 'b') { usage(argv[0]); return 1; }
            in_fmt = optarg[0];
            break;
        case 'o':
            if (optarg[0] != 't' && optarg[0] != 'b') { usage(argv[0]); return 1; }
            out_fmt = optarg[0];
            break;
        default:
            usage(argv[0]);
            return 1;
        }
    }

    if (argc - optind != 2) {
        usage(argv[0]);
        return 1;
    }

    char *in_name = argv[optind];
    char *out_name = argv[optind + 1];

    FILE *in = NULL;
    FILE *out = NULL;

    if (strcmp(in_name, "-") == 0) in = stdin;
    else {
        in = fopen(in_name, (in_fmt == 't') ? "r" : "rb");
        if (!in) { perror("fopen entrada"); return 1; }
    }

    if (strcmp(out_name, "-") == 0) out = stdout;
    else {
        out = fopen(out_name, (out_fmt == 't') ? "w" : "wb");
        if (!out) { perror("fopen salida"); if (in != stdin) fclose(in); return 1; }
    }

    SimpleRecord r;
    int ok;

    while (1) {
        ok = (in_fmt == 't') ? read_record_text(in, &r) : read_record_bin(in, &r);
        if (!ok) break;

        ok = (out_fmt == 't') ? write_record_text(out, &r) : write_record_bin(out, &r);
        if (!ok) { perror("escritura"); break; }
    }

    if (in != stdin) fclose(in);
    if (out != stdout) fclose(out);

    return 0;
}

