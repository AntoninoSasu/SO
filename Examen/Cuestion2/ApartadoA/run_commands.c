#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>
#include <wait.h>


#define MAX_PROCESOS 10
#define MAX_CHAR_LINE 512



pid_t launch_command(char** argv){
    pid_t pid = fork();
    if(pid == 0){
        //Child
        execvp(argv[0], argv);
        perror("execvp");  // Si execvp falla
        exit(EXIT_FAILURE);
    }
    else if(pid < 0){
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    return pid;
}

char **parse_command(const char *cmd, int* argc) {
    // Allocate space for the argv array (initially with space for 10 args)
    size_t argv_size = 10;
    const char *end;
    size_t arg_len; 
    int arg_count = 0;
    const char *start = cmd;
    char **argv = malloc(argv_size * sizeof(char *));

    if (argv == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    while (*start && isspace(*start)) start++; // Skip leading spaces

    while (*start) {
        // Reallocate more space if needed
        if (arg_count >= argv_size - 1) {  // Reserve space for the NULL at the end
            argv_size *= 2;
            argv = realloc(argv, argv_size * sizeof(char *));
            if (argv == NULL) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }

        // Find the start of the next argument
        end = start;
        while (*end && !isspace(*end)) end++;

        // Allocate space and copy the argument
        arg_len = end - start;
        argv[arg_count] = malloc(arg_len + 1);

        if (argv[arg_count] == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strncpy(argv[arg_count], start, arg_len);
        argv[arg_count][arg_len] = '\0';  // Null-terminate the argument
        arg_count++;

        // Move to the next argument, skipping spaces
        start = end;
        while (*start && isspace(*start)) start++;
    }

    argv[arg_count] = NULL; // Null-terminate the array

    (*argc)=arg_count; // Return argc

    return argv;
}


//Libera la memoria reservada
void free_arguments(char **argv) {
    if (argv != NULL) {
        for (int i = 0; argv[i] != NULL; i++) {
            free(argv[i]);
        }
        free(argv);
    }
}

//OPCIÓN -x
//Un proceso hijo ejecutará el comando pasado por la línea de comandos
void opcion_x(char **cmd_argv){
    pid_t pid = launch_command(cmd_argv);
    waitpid(pid, NULL, 0);
}

//OPCIÓN -s
//Los comandos se ejecutan en orden de aparición el fichero
void opcion_s(char *archivo){

    //Abro el archivo
    FILE* file;
    if((file = fopen(archivo, "r")) == NULL){
        perror("Error opening file\n");
        exit(EXIT_FAILURE);
    }

    //Lee cada línea del archivo, la cual contiene un comando.
    char line[MAX_CHAR_LINE];
    int total_comandos = 0;
    while(fgets(line, MAX_CHAR_LINE, file) !=  NULL){
        char **line_argv;
        int line_argc;
        //Llamo a la función parse_command para que ponga cada palabra en una posición del array
        line_argv = parse_command(line, &line_argc); 
        
        //Muestro el comando que se va a ejecutar
        printf("@@ Running command #%d: ", total_comandos);
        for(int i = 0; i < line_argc; i++)
            printf("%s ", line_argv[i]);
        printf("\n");


        //Ejecuto el comando llamando a launch_command
        pid_t pid = launch_command(line_argv);
        int status;
        waitpid(pid, &status, 0);

        printf("@@ Command #%d terminated (pid: %d, status: %d)\n", total_comandos, pid, status);

        total_comandos++;
        
        //libero la memoria creada en parse_command
        free_arguments(line_argv);
    }  
}

//OPCIÓN -s -b (parte opcional)
//Los comandos del fichero se ejecutarán uno tras otro sin esperar a que el comando anterior termine
void opcion_sb(char *archivo){
    //Abro el archivo
    FILE* file;
    if((file = fopen(archivo, "r")) == NULL){
        perror("Error opening file\n");
        exit(EXIT_FAILURE);
    }

    char line[MAX_CHAR_LINE];
    int total_comandos = 0;
    int procesos[MAX_PROCESOS]; //Almacena el pid de los procesos
    pid_t pid;

    //Lee cada línea del archivo, la cual contiene un comando.
    while(fgets(line, MAX_CHAR_LINE, file) !=  NULL){
        char **line_argv;
        int line_argc;
        //Llamo a la función parse_command para que ponga cada palabra en una posición del array
        line_argv = parse_command(line, &line_argc); 
        
        //Muestro el comando que se va a ejecutar
        printf("@@ Running command #%d: ", total_comandos);
        for(int i = 0; i < line_argc; i++)
            printf("%s ", line_argv[i]);
        printf("\n");
        
        //Ejecuto el comando llamando a launch_command
        pid = launch_command(line_argv);

        //Almaceno el pid del proceso iésimo
        procesos[total_comandos]= pid;
        total_comandos++;

        //libero la memoria creada en parse_command
        free_arguments(line_argv);    
    }  

    //Espero a que acaben todos los procesos y muestro su pid y número
    int status;
    while((pid = wait(&status)) != -1){
        for(int i = 0; i < total_comandos; i++){
            if(pid == procesos[i])
                printf("@@ Command #%d terminated (pid: %d, status: %d)\n", i, procesos[i], status);
        }
    }
}

void signal_handler(int sig){
    kill(getpid(), SIGKILL);
}

void opcion_i(){
    char command[1024];
    while(1){
        printf("> ");
        fflush(stdout);
        if(fgets(command, sizeof(command), stdin) == NULL){
            perror("fgets");
            exit(EXIT_FAILURE);
        }

        int linec;
        char **line = parse_command(command, &linec);
        launch_command(line);
        wait(NULL);

        //Liberar la memoria
        free_arguments(line);
    }

}


int main(int argc, char *argv[]) {
    char **cmd_argv;
    int cmd_argc;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s \"command\"\n", argv[0]);
        return EXIT_FAILURE;
    }

    
    int opt;
    bool opcionB = false;
    char* archivo;
    while((opt = getopt(argc, argv, "x:s:bi")) != -1){
        switch(opt){
            case 'x':
                // Parsear el comando
                //Cada posición contiene un argumento, sin contar la opción
                //Ejemplo: "ls -l", cmd_argv[0] = "ls", cmd_argv[1] = "-l", cmd_argv[2] = NULL
                cmd_argv = parse_command(optarg, &cmd_argc);

                opcion_x(cmd_argv);
                
                //Liberar la memoria
                free_arguments(cmd_argv);
                break;
            case 's':
                archivo = optarg;
                if(!opcionB)
                    opcion_s(archivo);
                else
                    opcion_sb(archivo);
                break;
            case 'b':
                opcionB = true;
                break;
            case 'i':
                signal(SIGSTOP, signal_handler);
                opcion_i();
            break;
            default:
                perror("Invalid option\n");
                break;
        }
    }


    return EXIT_SUCCESS;
}