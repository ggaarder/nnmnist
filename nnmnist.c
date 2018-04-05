#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ntwkarg.h"

#if 0
// Network Storage Format

uint32 L; // number of layers, at least 3, that is one input layer, one hidden layer and one output layer

struct layer {
    uint32 ncnt; // number of neurons. 0 for the input layer
    
    struct neuron {
        float32 w[layers[l-1].ncnt]; // weights. number of weights is the number of the neurons of the last layer
        float32 bias;
    } neurons[ncnt];
} layers[L];
#endif

#define ARGFN "ntwkarg" // arg filename

#define LBL "train-labels.idx1-ubyte"
#define IMG "train-images.idx3-ubyte"
char *lbl, *img;

uint32_t toggledn(uint32_t value) // Change Endianness
{
    uint32_t result = 0;
    result |= (value & 0x000000FF) << 24;
    result |= (value & 0x0000FF00) << 8;
    result |= (value & 0x00FF0000) >> 8;
    result |= (value & 0xFF000000) >> 24;
    return result;
}

float sigm(float x) {
  x = 1.0 + exp(x);
  return 1.0/x;
}

float dsigm(float x) {
  float ex = exp(x);
  x = 1.0+1.0/ex;
  x = ex*x*x;
  return 1.0/x;
}

int getlbl(int no) { // no starts from 0
  return (int)lbl[4+4+no+1];
}

void getimg(int no) { // no starts from 0
  int i;
  for (i = 0; i < h.K; ++i)
    a[i] = (float)(int)img[4+4+4+4+28*28*no+i];
}

int main() {
  int i;
  int lblfd = open(LBL, O_RDONLY);
  int imgfd = open(IMG, O_RDONLY);
  int lbllen = lseek(lblfd, 0, SEEK_END)+1;
  int imglen = lseek(imgfd, 0, SEEK_END)+1;
  lbl = (char*)mmap(NULL, lbllen, PROT_READ, MAP_PRIVATE, lblfd, 0);
  img = (char*)mmap(NULL, imglen, PROT_READ, MAP_PRIVATE, imgfd, 0);

  int xcnt = toggledn(*(uint32_t*)(lbl+4));
  printf("%d Training Samples\n", xcnt);
  
  while (1) {
    int imgno;

    for (imgno = 0; imgno < xcnt; ++imgno) {
      int i;
      int l, n, k;

      for (l = 0; l <= h.L; ++l) {
        for (n = 0; n < h.N; ++n) {
          float *zat = z+NEUOFS
            *zat = *BIAS;
          for (k = 0; k < h.K; ++k)
            *zat += (*WEIG)*in[k];
          a[NEUOFS] = sigm(*zat);
        }
      }
    }
  }

  for (i = 0; i < LYRCNT; ++i) free(neuofs[i]);
  munmap(lbl, lbllen);
  munmap(img, imglen);
}
