#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ntwkarg.h"

#define HNECNT 20 // hidden layer neuron count
#define CHARN 10 // number of characters to recognize, that is the number of neurons of the output layer
#define RND ((float)rand()/RAND_MAX)

int main() {
  FILE *fp = fopen(ARGFN, "w");
  int i, j, l;
  int L = 6;
  int K = 28*28;
  fprintf(fp, "Number of Layers: %d\nNumber of Input Layers: %d\n", L, K);

  srand(time(0));
  for (l = 1; l < L; ++l) {  
    fprintf(fp, "--- Layer %d %s\n", l, (l == L-1 ? "(Output Layer)" : "(Hidden Layer)"));
    int ncnt = (l == L-1 ? CHARN : HNECNT);
    fprintf(fp, "Number of Neurons: %d\n", ncnt);
    int wcnt = (l == 1 ? K : HNECNT);
    fprintf(fp, "Number of Weights: %d\n", wcnt);
    for (i = 0; i < ncnt; ++i) {
      fprintf(fp, "---- Neuron %d of Layer %d\n", i, l);
      for (j = 0; j < wcnt; ++j)
        fprintf(fp, "%f ", RND);
      fprintf(fp, "%f\n", RND);
    }
  }
 
  fclose(fp);
}
