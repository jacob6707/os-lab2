/*
 * forkbomb.c
 *
 * Program that generates N number of child processes using fork().
 * Usage: forkbomb [-h] -c N
 *
 * Options:
 *   -c N    number of child processes to create (required, N must be a positive integer)
 *   -h      display this help and exit
 *
 * The program performs checks for argument errors and reports exact problems.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>

static void usage(const char *prog) {
  fprintf(stderr,
    "Usage: %s [-h] -c COUNT\n"
    "  -c COUNT      number of child processes to create (required, positive integer)\n"
    "  -h            show this help message and exit\n",
    prog);
}

static long parse_childcount(const char *s, int *err) {
  char *end;
  errno = 0;
  long v = strtol(s, &end, 10);
  if (s[0] == '\0' || *end != '\0') {
    *err = 1;
    return 0;
  }
  if ((errno == ERANGE && (v == LONG_MAX || v == LONG_MIN)) || v <= 0) {
    *err = 1;
    return 0;
  }
  *err = 0;
  return v;
}

int main (int argc, char* argv[]) {
  setbuf(stdout, NULL); // Disable buffering for stdout
  
  const char *prog = argv[0];
  int opt;
  long child_count = -1;
  int parse_err = 0;

  while ((opt = getopt(argc, argv, "c:h")) != -1) {
    switch (opt) {
    case 'c':
      child_count = parse_childcount(optarg, &parse_err);
      if (parse_err) {
        fprintf(stderr, "Invalid child count: '%s' - must be a positive integer\n", optarg);
        return EXIT_FAILURE;
      }
      break;
    case 'h':
      usage(prog);
      return EXIT_SUCCESS;
    case '?':
    default:
      if (optopt == 'c')
        fprintf(stderr, "Option -%c requires an argument.\n", optopt);
      else
        fprintf(stderr, "Unknown option `-%c'.\n", optopt);
      usage(prog);
      return EXIT_FAILURE;
    }
  }

  if (child_count == -1) {
    fprintf(stderr, "Error: Child count (-c) is required.\n");
    usage(prog);
    return EXIT_FAILURE;
  }

  for (int i = 0; i < child_count; i++) {
    printf("Roditelj #%d\n", i + 1);

    pid_t pid = fork(); 
    if (pid < 0) {
      perror("fork");
      exit(EXIT_FAILURE);
    }
    else if (pid == 0) {
      printf("Dijete #%d\n", i + 1);
      exit(EXIT_SUCCESS);
    }
    else {
      wait(NULL);
      printf("end\n");
    }
  }

  return EXIT_SUCCESS;
}