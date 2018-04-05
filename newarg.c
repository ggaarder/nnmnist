#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ntwkarg.h"

int main() {
  FILE *fp = fopen(ARG, "w");
  struct header h = {2, 20, 28*28, 10};
  int i;
  int argsiz = ARGSIZ;
  fwrite(&h, sizeof(struct header), 1, fp);
  float *arg = malloc(sizeof(float)*argsiz);
  
  srand(time(0));
  for (i = 0; i < argsiz; ++i)
    arg[i] = (float)rand()/RAND_MAX;
  fwrite(arg, sizeof(float), argsiz, fp);

  fclose(fp);
  free(arg);
}
