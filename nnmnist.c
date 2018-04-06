#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#if 0
// Network Storage Format

int L; // number of layers, at least 3, that is one input layer, one hidden layer and one output layer

struct layer {
    int ncnt; // number of neurons. 0 for the input layer
    
    struct neuron {
      float w[layers[l-1].ncnt]; // weights. number of weights is the number of the neurons of the last layer
      float bias; // specially there is no bias in the input layer
    } neurons[ncnt];
} layers[L];
#endif
char *rawntwk = 0;
int L;
char **layers = 0; // for convenience

#define LBL "train-labels.idx1-ubyte"
#define IMG "train-images.idx3-ubyte"
char *lbl = 0, *img = 0;
int imgsiz;

uint32_t toggledn(uint32_t value) { // Change Endianness
    int result = 0;
    result |= (value & 0x000000FF) << 24;
    result |= (value & 0x0000FF00) << 8;
    result |= (value & 0x00FF0000) >> 8;
    result |= (value & 0xFF000000) >> 24;
    return result;
}

float sigm(float x) {
  x = 1.0 + exp(-x);
  return 1.0/x;
}

float dsigm(float x) {
  x = exp(-x);
  return x/(1.0+x)/(1.0+x);
}

int getlbl(int no) { // no starts from 0
  return (int)lbl[4+4+no+1];
}

void getimg(int no) { // no starts from 0
  int i;
  // for (i = 0; i < imgsiz; ++i)
    //    a[i] = (float)(int)img[4+4+4+4+imgsiz*no+i];
}

int main() {
  int i, wcnt, ncnt, l, n;
  char *p;
  float v;
  int imgfd = 0, lblfd = 0, ntwkfd = 0,
    imglen, lbllen, xcnt, ntwklen, imgno;

  imgfd = open(IMG, O_RDONLY);
  imglen = lseek(imgfd, 0, SEEK_END)+1;
  img = (char*)mmap(NULL, imglen, PROT_READ, MAP_PRIVATE, imgfd, 0);
  imgsiz = toggledn(*((int*)(img)+2))*toggledn(*((int*)(img)+3));
  printf("Image size: %d\n", imgsiz);
  
  ntwkfd = open(NTWKFN, O_RDWR);
  if (errno == ENOENT) {
    printf("Creating new network storage in \"%s\"\n", NTWKFN);
    ntwkfd = open(NTWKFN, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);

    L = 4+2;
    write(ntwkfd, &L, sizeof(int));
    ncnt = imgsiz;
    write(ntwkfd, &ncnt, sizeof(int)); // input layer
    srand(time(0));
    
    for (l = 1; l < L-1; ++l) { // hidden layers
      wcnt = ncnt;
      ncnt = 20;
      write(ntwkfd, &ncnt, sizeof(int));
      for (n = 0; n < ncnt; ++n)
        for (i = 0; i <= wcnt; ++i) { // together with bias
          v = rand()/RAND_MAX;
          write(ntwkfd, &v, sizeof(float));
        }
    }

    // output layer
    wcnt = ncnt;
    ncnt = 10; // recognizing 10 digits
    write(ntwkfd, &ncnt, sizeof(int));
    for (n = 0; n < ncnt; ++n)
      for (i = 0; i <= wcnt; ++i) { // together with bias
        v = rand()/RAND_MAX;
        write(ntwkfd, &v, sizeof(float));
      }    
        
    goto byebye;
  }
  
  ntwklen = lseek(ntwkfd, 0, SEEK_END)+1;
  rawntwk = (char*)mmap(NULL, ntwklen, PROT_READ|PROT_WRITE, MAP_SHARED, ntwkfd, 0);
  L = *(int*)rawntwk;
  printf("Network: %d Layers\n", L);

  // initializing an array points to each layer for convenience
  layers = (char**)calloc(L, sizeof(char*));
  p = rawntwk+sizeof(int); // jumps over L
  layers[0] = p;
  wcnt = *(int*)p; // wcnt of layer 1, ncnt of layer 0
  p += sizeof(int); // jumps over the ncnt of layer 0
                    // now p points to layer 1
  for (i = 1; i < L; ++i) {
    layers[i] = p;
    ncnt = *(int*)p;
    p += sizeof(int); // jumps over the ncnt of layer currl
    p += ncnt*(1+wcnt)*sizeof(float); // jumps over the neurons
                                      // now points to layer currl+1
    wcnt = ncnt; // update
  }
  
  for (i = 0; i < L; ++i)
    printf("Layer %d: %d neurons\n", i, *(int*)layers[i]);
    
  lblfd = open(LBL, O_RDONLY);
  lbllen = lseek(lblfd, 0, SEEK_END)+1;
  lbl = (char*)mmap(NULL, lbllen, PROT_READ, MAP_PRIVATE, lblfd, 0);
  xcnt = toggledn(*((int*)lbl+1));
  printf("%d Training Samples\n", xcnt);

 byebye:
  free(layers);
  if (rawntwk) munmap(rawntwk, ntwklen);
  if (ntwkfd) close(ntwkfd);
  if (lbl) munmap(lbl, lbllen);
  if (lblfd) close(lblfd);
  if (img) munmap(img, imglen);
  if (imgfd) close(imgfd);
}
