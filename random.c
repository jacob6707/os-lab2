#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

char useless[100000]; 

int main() {
  srand(time(NULL));

  for(int i = 0; i < 100000; i++) {
    /* generate either -1 or +1 (no zero) */
    useless[i] = (rand() & 1) ? 1 : -1;
  }
  

  for(int i = 0; i < 100000; i++) {
    printf("%d ", useless[i]);
  }

  sleep(20);
  
  return 0;
}