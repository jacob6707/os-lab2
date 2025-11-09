// /home/jakov/os-lab2/analyze_forkbomb.c
// Usage: ./analyze_forkbomb filename
// Counts anomaly blocks where expected sequence is:
//   Roditelj #<n>
//   Dijete   #<n>
//   end
// Each group of 3 lines is one record; any mismatch increments anomaly count.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char *trim(char *s) {
  if (!s) return s;
  while (isspace((unsigned char)*s)) s++;
  char *end = s + strlen(s) - 1;
  while (end >= s && isspace((unsigned char)*end)) *end-- = '\0';
  return s;
}

static int parse_hash_number(const char *s, long *out) {
  const char *p = strchr(s, '#');
  if (!p) return 0;
  char *endptr;
  long v = strtol(p + 1, &endptr, 10);
  if (p + 1 == endptr) return 0; // no digits
  *out = v;
  return 1;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <file>\n", argv[0]);
    return 2;
  }

  FILE *f = fopen(argv[1], "r");
  if (!f) {
    perror("fopen");
    return 2;
  }

  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  char **lines = NULL;
  size_t count = 0;

  while ((read = getline(&line, &len, f)) != -1) {
    // store trimmed copy
    char *copy = strdup(line);
    if (!copy) { perror("strdup"); return 2; }
    lines = realloc(lines, (count + 1) * sizeof(*lines));
    if (!lines) { perror("realloc"); return 2; }
    lines[count++] = trim(copy);
  }
  free(line);
  fclose(f);

  size_t anomalies = 0;
  size_t i = 0;
  while (i + 2 < count) {
    char *a = lines[i];
    char *b = lines[i + 1];
    char *c = lines[i + 2];
    int ok = 1;
    long na = 0, nb = 0;

    // check first line starts with "Roditelj"
    if (strncmp(a, "Roditelj", 8) != 0 || !parse_hash_number(a, &na)) ok = 0;
    // second must start with "Dijete"
    if (strncmp(b, "Dijete", 6) != 0 || !parse_hash_number(b, &nb)) ok = 0;
    // numbers must match
    if (ok && na != nb) ok = 0;
    // third must be exactly "end"
    if (strcmp(c, "end") != 0) ok = 0;

    if (!ok) anomalies++;
    i += 3;
  }

  // any leftover lines that don't form a full block count as anomalies (one per incomplete block)
  if (i < count) anomalies += 1;

  // cleanup
  for (size_t j = 0; j < count; ++j) free(lines[j]);
  free(lines);

  printf("%zu\n", anomalies);
  return 0;
}