#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAXBUF 20

// ESTA FUNCION NOS LA DAN
char **parse_command(const char *cmd, int *argc) {
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

  while (*start && isspace(*start))
    start++; // Skip leading spaces

  while (*start) {
    // Reallocate more space if needed
    if (arg_count >= argv_size - 1) { // Reserve space for the NULL at the end
      argv_size *= 2;
      argv = realloc(argv, argv_size * sizeof(char *));
      if (argv == NULL) {
        perror("realloc");
        exit(EXIT_FAILURE);
      }
    }

    // Find the start of the next argument
    end = start;
    while (*end && !isspace(*end))
      end++;

    // Allocate space and copy the argument
    arg_len = end - start;
    argv[arg_count] = malloc(arg_len + 1);

    if (argv[arg_count] == NULL) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }
    strncpy(argv[arg_count], start, arg_len);
    argv[arg_count][arg_len] = '\0'; // Null-terminate the argument
    arg_count++;

    // Move to the next argument, skipping spaces
    start = end;
    while (*start && isspace(*start))
      start++;
  }

  argv[arg_count] = NULL; // Null-terminate the array

  (*argc) = arg_count; // Return argc

  return argv;
}

void freeCmd(int cmd_argc, char **cmd_argv) {

  // printf("argc: %d\n", cmd_argc);
  for (int i = 0; cmd_argv[i] != NULL; i++) {
    // printf("argv[%d]: %s\n", i, cmd_argv[i]);
    free(cmd_argv[i]); // Free individual argument
  }

  free(cmd_argv); // Free the cmd_argv array
}

// receives the command to exec and its arguments
pid_t launch_command(char **argv) {
  int pid = fork();

  // if is son process
  if (pid == 0)
    // El comando que ejecuto (como ls) y la lista de argumentos del comando
    execvp(argv[0], argv);

  return pid;
}

int main(int argc, char *argv[]) {
  char **cmd_argv;
  int cmd_argc;

  int pids[MAXBUF] = {0};

  if (argc < 3) {
    fprintf(stderr, "Usage: %s <option> \"command\"\n", argv[0]);
    return EXIT_FAILURE;
  }

  // cmd_argv = parse_command(argv[1], &cmd_argc);
  //  options parse with getopt
  int opt = getopt(argc, argv, "x:s:b");
  FILE *fd;
  int multiprocess = 0;

  if (opt == 'b') {
    multiprocess = 1;
    opt = getopt(argc, argv, "x:s:");
  }

  switch (opt) {
  case 'x':
    if (multiprocess == 1) {
      fprintf(stderr, "-b option is not a valid option for -x command");
      exit(EXIT_FAILURE);
    }
    cmd_argv = parse_command(optarg, &cmd_argc);
    int pid = launch_command(cmd_argv);
    wait(&pid);
    freeCmd(cmd_argc, cmd_argv);
    break;

  case 's':
    if ((fd = fopen(optarg, "r")) == NULL) {
      fprintf(stderr, "File: %s couldn't be open\n", optarg);
      exit(EXIT_FAILURE);
    }
    char *str = malloc(sizeof(char) * MAXBUF);
    // fgets() lee la linea entera en str con tamaño maximo MAXBUF del archivo con descriptor fd
    while (fgets(str, MAXBUF, fd) != NULL) {
      cmd_argv = parse_command(str, &cmd_argc);
      int pid = launch_command(cmd_argv);
      if (multiprocess == 1) {
        for (int i = 0; i < MAXBUF; i++) {
          if (pids[i] == 0) {
            pids[i] = pid;
            // printf("empezo pid: %d\n", pids[i]);
            break;
          }
        }

        for (int i = 0; i < MAXBUF; i++) {
          if (pids[i] != 0 && pids[i] == waitpid(pids[i], NULL, WNOHANG))
            pids[i] = 0;
        }
      } else
        wait(&pid);
    }
    freeCmd(cmd_argc, cmd_argv);
    break;

  default:
    fprintf(stderr, "Argument %s not recognized", argv[1]);
    exit(EXIT_FAILURE);
    break;
  }

  return EXIT_SUCCESS;
}
