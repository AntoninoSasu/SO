#include <stdio.h>
#include <unistd.h> /* for getopt() */
#include <stdlib.h> /* for EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h>
#include "defs.h"

/* Assume lines in the text file are no larger than 100 chars */
#define MAXLEN_LINE_FILE 100

void print_usage() {
    printf("Usage: ./student-records [ -h | -p | -i file  | -o <output_file> | -b ]\n");
}

void print_student(student_t student, int index) {
    printf("[Entry #%d]\n", index);
    printf("\tstudent_id=%d\n", student.student_id);
    printf("\tNIF=%s\n", student.NIF);
    printf("\tfirst_name=%s\n", student.first_name);
    printf("\tlast_name=%s\n", student.last_name);
}

int print_text_file(char *path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    char line[MAXLEN_LINE_FILE];
    if (fgets(line, sizeof(line), file) == NULL) {
        fprintf(stderr, "Error reading the number of records\n");
        fclose(file);
        return EXIT_FAILURE;
    }

    int num_records;
    if (sscanf(line, "%d", &num_records) != 1) {
        fprintf(stderr, "Invalid format: first line must contain the number of records\n");
        fclose(file);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < num_records; i++) {
        if (fgets(line, sizeof(line), file) == NULL) {
            fprintf(stderr, "Unexpected end of file\n");
            fclose(file);
            return EXIT_FAILURE;
        }

        student_t student;
        char *token;
        char *rest = line;

        token = strsep(&rest, ":"); /*Parámetros:

&rest: Puntero a una cadena que se va modificando con cada llamada.
":": El carácter delimitador para separar tokens.*/
        if (token != NULL) {
            student.student_id = atoi(token);
        } else {
            fprintf(stderr, "Error parsing student_id at line %d\n", i + 1);
            fclose(file);
            return EXIT_FAILURE;
        }

        token = strsep(&rest, ":");
        if (token != NULL) {
            strncpy(student.NIF, token, MAX_CHARS_NIF);
            student.NIF[MAX_CHARS_NIF] = '\0'; // Ensure null termination
        } else {
            fprintf(stderr, "Error parsing NIF at line %d\n", i + 1);
            fclose(file);
            return EXIT_FAILURE;
        }

        token = strsep(&rest, ":");
        if (token != NULL) {
            student.first_name = malloc(strlen(token) + 1);
            if (student.first_name == NULL) {
                perror("Error allocating memory for first_name");
                fclose(file);
                return EXIT_FAILURE;
            }
            strcpy(student.first_name, token); /*char *strncpy(char *dest, const char *src, size_t n); dest: Puntero al búfer de destino donde se copiarán los caracteres.

src: Puntero a la cadena de origen que se va a copiar.

n: Número máximo de caracteres a copiar desde src.

Devuelve: Un puntero a dest (la cadena de destino). */
        } else {
            fprintf(stderr, "Error parsing first_name at line %d\n", i + 1);
            fclose(file);
            return EXIT_FAILURE;
        }

        token = strsep(&rest, ":\n");
        if (token != NULL) {
            student.last_name = malloc(strlen(token) + 1);
            if (student.last_name == NULL) {
                perror("Error allocating memory for last_name");
                fclose(file);
                return EXIT_FAILURE;
            }
            strcpy(student.last_name, token);
        } else {
            fprintf(stderr, "Error parsing last_name at line %d\n", i + 1);
            fclose(file);
            return EXIT_FAILURE;
        }

        print_student(student, i);

        // Free dynamic memory
        free(student.first_name);
        free(student.last_name);
    }

    fclose(file);
    return EXIT_SUCCESS;
}

char *loadstr(FILE *file) {
    int stringBytes = 0;
    int bytesRead = 0;

    // Read character by character until '\0' is found
    while (1) {
        char c;
        bytesRead = fread(&c, sizeof(char), 1, file);
        if (bytesRead == 0 || c == '\0') {
            break;
        }
        stringBytes++;
    }

    if (stringBytes == 0) {
        return NULL; // No valid string
    }

    // Move back to read the complete string
    fseek(file, -(stringBytes + 1), SEEK_CUR);

    // Read the entire string
    char *str = malloc(stringBytes + 1);
    if (!str) {
        perror("Error allocating memory for string");
        return NULL;
    }

    fread(str, sizeof(char), stringBytes + 1, file);
    return str;
}

void write_binary_string(FILE *file, const char *str) {
    fwrite(str, sizeof(char), strlen(str) + 1, file); // Include null terminator
}

int write_binary_file(char *input_file, char *output_file) {
    FILE *infile = fopen(input_file, "r");
    if (!infile) {
        perror("Error opening input file");
        return EXIT_FAILURE;
    }

    FILE *outfile = fopen(output_file, "wb");
    if (!outfile) {
        perror("Error opening output file");
        fclose(infile);
        return EXIT_FAILURE;
    }

    int num_records;
    fscanf(infile, "%d\n", &num_records);   
    fwrite(&num_records, sizeof(num_records), 1, outfile);

    for (int i = 0; i < num_records; i++) {
        student_t student;
        char line[MAXLEN_LINE_FILE];
        fgets(line, sizeof(line), infile);  /*fgets(line, sizeof(line), file); Descripción: 
        Lee una línea completa de un archivo.

        Parámetros:

        line: Un buffer donde se almacenará la línea leída.
        sizeof(line): El tamaño máximo de caracteres a leer (para evitar desbordamiento de buffer).
        file: El puntero al archivo desde donde se leerá.*/

        char *token = strtok(line, ":");
        student.student_id = atoi(token);

        token = strtok(NULL, ":");
        strncpy(student.NIF, token, MAX_CHARS_NIF + 1);
        student.NIF[MAX_CHARS_NIF] = '\0'; // Ensure null termination

        token = strtok(NULL, ":");
        student.first_name = strdup(token);  /*char *strdup(const char *str); str: Puntero a la cadena que se quiere duplicar.
Devuelve: Un puntero a una nueva cadena que es una copia exacta de str. Esta copia se almacena en memoria dinámica y debe liberarse con free() cuando ya no se necesite.*/

        token = strtok(NULL, ":\n");
        student.last_name = strdup(token);

        fwrite(&student.student_id, sizeof(student.student_id), 1, outfile);
        write_binary_string(outfile, student.NIF);

        write_binary_string(outfile, student.first_name);
        write_binary_string(outfile, student.last_name);

        free(student.first_name);
        free(student.last_name);
    }

    fclose(infile);
    fclose(outfile);

    printf("%d student records written successfully to binary file %s\n", num_records, output_file);
    return EXIT_SUCCESS;
}

int print_binary_file(char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening binary file");
        return EXIT_FAILURE;
    }

    int num_records;
    fread(&num_records, sizeof(num_records), 1, file);

    for (int i = 0; i < num_records; i++) {
        student_t student;

        fread(&student.student_id, sizeof(student.student_id), 1, file);

        char *tmp_NIF = loadstr(file);
        strncpy(student.NIF, tmp_NIF, MAX_CHARS_NIF + 1); // Ensure null termination
        free(tmp_NIF);

        student.first_name = loadstr(file);
        student.last_name = loadstr(file);

        print_student(student, i);

        free(student.first_name);
        free(student.last_name);
    }

    fclose(file);
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    int ret_code, opt;
    struct options options;

    /* Initialize default values for options */
    options.input_file = NULL;
    options.output_file = NULL;
    options.action = NONE_ACT;
    ret_code = 0;

    /* Parse command-line options */
    while ((opt = getopt(argc, argv, "pho:i:b")) != -1) {
        switch (opt) {
            case 'h':
                print_usage();
                return EXIT_SUCCESS;
            case 'i':
                options.input_file = optarg;
                break;
            case 'p':
                options.action = PRINT_TEXT_ACT;
                break;
            case 'o':
                options.action = WRITE_BINARY_ACT;
                options.output_file = optarg;
                break;
            case 'b':
                options.action = PRINT_BINARY_ACT;
                break;
            default:
                print_usage();
                return EXIT_FAILURE;
        }
    }

    if (options.input_file == NULL) {
        fprintf(stderr, "Must specify one record file as an argument of -i\n");
        exit(EXIT_FAILURE);
    }

    switch (options.action) {
        case NONE_ACT:
            fprintf(stderr, "Must indicate one of the following options: -p, -o, -b \n");
            ret_code = EXIT_FAILURE;
            break;
        case PRINT_TEXT_ACT:
            /* Part A */
            ret_code = print_text_file(options.input_file);
            break;
        case WRITE_BINARY_ACT:
            /* Part B */
            ret_code = write_binary_file(options.input_file, options.output_file);
            break;
        case PRINT_BINARY_ACT:
            /* Part C */
            ret_code = print_binary_file(options.input_file);
            break;
        default:
            break;
    }
    exit(ret_code);
}
