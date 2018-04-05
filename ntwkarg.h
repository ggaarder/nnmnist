#ifndef _NNMNIST_CONF_H
#define _NNMNIST_CONF_H

#define ARG "ntwkarg"

struct header {
  int L; // hidden layers
  int N; // neurons per a hidden layer
  int K; // input neurons, 28*28 grayscale. and that would be the count of the weights
  int D; // neurons of the output layout
};

#define NEUSIZ (h.N*(h.K+1)) // +1 means bias
#define HLASIZA (h.N*NEUSIZ)  // hidden layer size
#define HLASIZ (h.L*HLASIZA)
#define OLASIZ (h.D*NEUSIZ)    // output layer size
#define ARGSIZ (HLASIZ+OLASIZ)

// weight, l = 0..L, n = 0..N-1, k = 0..K-1
// l = L means the output layer, where n = 0..D-1
#define WEIG (arg+l*HLASIZA+n*NEUSIZ+k)
// bias
#define BIAS HLW(l, n, h.K)

#endif /* _NNMNIST_CONF_H */
