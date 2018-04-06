#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NTWKFN "ntwkarg"

#if 0
// Network Storage Format

int L; // number of layers, at least 3, that is one input layer, one hidden layer and one output layer

struct layer {
    int ncnt; // number of neurons. <imgsiz> for the input layer
    
    struct neuron {
      float w[layers[l-1].ncnt]; // weights. number of weights is the number of the neurons of the last layer. there is no weights in the input layer
      float bias; // specially there is no bias in the input layer
    } neurons[ncnt];
} layers[L];
#endif
char *rawntwk = 0;
int L;
char **layers = 0; // for convenience
int *ncnt = 0; // for convenience
int *wcnt = 0; // for convenience

struct neuron {
  float a, z, theta,
    *arg, // point to mmap'd network
    *gradient; // allocated
}  **neurons = 0; // for convenience

#define LBL "train-labels.idx1-ubyte"
#define IMG "train-images.idx3-ubyte"
char *lbl = 0, *img = 0, *imgp, *lblp;
int imgsiz, imgfd = -1, lblsiz, lblfd = -1, ntwkfd = -1, imglen, xcnt, ntwklen;

uint32_t toggledn(uint32_t value) { // Change Endianness. My environment is little endianess but the MNIST file is big endianess
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

void newntwk() {
  int ncnt, wcnt, l, n, i;
  float v;
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
        v = (float)rand()/RAND_MAX;
        write(ntwkfd, &v, sizeof(float));
      }
  }
  
  // output layer
  wcnt = ncnt;
  ncnt = 10; // recognizing 10 digits
  write(ntwkfd, &ncnt, sizeof(int));
  for (n = 0; n < ncnt; ++n)
    for (i = 0; i <= wcnt; ++i) { // together with bias
      v = (float)rand()/RAND_MAX;
      write(ntwkfd, &v, sizeof(float));
    }
}

void initntwk() {
  int i;
  char *p;
  layers = calloc(L, sizeof(char*));
  ncnt = calloc(L, sizeof(int*));
  wcnt = calloc(L, sizeof(int*));
  p = rawntwk+sizeof(int); // jumps over L
  layers[0] = p;
  ncnt[0] = *(int*)p; // wcnt of layer 1, ncnt of layer 0
  wcnt[0] = 0;
  p += sizeof(int); // jumps over the ncnt of layer 0
                    // now p points to layer 1
  for (i = 1; i < L; ++i) {
    layers[i] = p;
    ncnt[i] = *(int*)p;
    wcnt[i] = ncnt[i-1];
    printf("%f\n", *(float*)(p+sizeof(int)));
    p += sizeof(int); // jumps over the ncnt of the current layer
    p += ncnt[i]*(1+wcnt[i])*sizeof(float); // jumps over the neurons
                                            // now points to layer currl+1
  }
}

void initneun() {
  int i, j;
  neurons = calloc(L, sizeof(struct neuron*));
  for (i = 0; i < L; ++i) {
    neurons[i] = calloc(ncnt[i], sizeof(struct neuron));
    if (i == 0) continue;
    neurons[i][0].arg = (float*)(layers[i] + sizeof(int));
    for (j = 0; j < ncnt[i]; ++j) {
      neurons[i][j].arg = neurons[i][j-1].arg + wcnt[i] + 1;
      neurons[i][j].gradient = calloc(wcnt[i]+1, sizeof(float));
    }
  }
}

void loadimg() {
  imgfd = open(IMG, O_RDONLY);
  imglen = lseek(imgfd, 0, SEEK_END)+1;
  img = mmap(NULL, imglen, PROT_READ, MAP_PRIVATE, imgfd, 0);
  imgsiz = toggledn(*((int*)(img)+2))*toggledn(*((int*)(img)+3));
  printf("Image size: %d\n", imgsiz);
  imgp = (char*)((int*)img + 4); // skip the magic number, number of images, number of rows and number of cols, pointing to the first image
}

void loadlbl() {
  lblfd = open(LBL, O_RDONLY);
  lblsiz = lseek(lblfd, 0, SEEK_END)+1;
  lbl = mmap(NULL, lblsiz, PROT_READ, MAP_PRIVATE, lblfd, 0);
  xcnt = toggledn(*((int*)lbl+1));
  lblp = (char*)((int*)lbl+2); // skip the magic number and the number of items, pointing to the first label
}

void freeall() {
  int i, j;
  
  free(layers);
  if (neurons) {
    for (i = 0; i < L; ++i) {
      if (i != 0)
        for (j = 0; j < ncnt[i]; ++j)
          free(neurons[i][j].gradient);
      free(neurons[i]);
    }
    free(neurons);
  }
  free(ncnt);
  free(wcnt);
  if (rawntwk) munmap(rawntwk, ntwklen);
  if (ntwkfd > 0) close(ntwkfd);
  if (lbl) munmap(lbl, lblsiz);
  if (lblfd > 0) close(lblfd);
  if (img) munmap(img, imglen);
  if (imgfd > 0) close(imgfd);
}

float calc() {
  int imgno, i, j, k;
  float loss = 0.0, f, v;
  
  for (imgno = 0; imgno < xcnt; ++imgno, ++lblp, imgp += imgsiz) {
    for (i = 0; i < imgsiz; ++i)
      neurons[0][i].a = imgp[i];
    for (i = 1; i < L; ++i)
      for (j = 0; j < ncnt[i]; ++j) {
        neurons[i][j].z = neurons[i][j].arg[wcnt[i]]; // bias
        for (k = 0; k < wcnt[i]; ++k)
          neurons[i][j].z += neurons[i][j].arg[k]*neurons[i-1][k].a;
        neurons[i][j].a = sigm(neurons[i][j].z);
      }
    v = 0;
    for (i = 0; i < ncnt[L-1]; ++i) {
      f = neurons[L-1][i].a - (i == toggledn(*lblp));
      v += f*f;
    }
    loss += v/xcnt;
  }
}

int main() {
  int i;
  float loss;
  loadimg();
  
  ntwkfd = open(NTWKFN, O_RDWR);
  if (errno == ENOENT) {
    printf("Creating new network storage in \"%s\"\n", NTWKFN);
    newntwk();
    goto byebye;
  }
  
  ntwklen = lseek(ntwkfd, 0, SEEK_END)+1;
  rawntwk = mmap(NULL, ntwklen, PROT_READ|PROT_WRITE, MAP_SHARED, ntwkfd, 0);
  L = *(int*)rawntwk;
  printf("Network: %d Layers\n", L);

  initntwk();

  printf("%5s %6s %7s\n", "", "Neuron", "Weights");
  for (i = 0; i < L; ++i)
    printf("%5d %6d %7d\n", i, ncnt[i], wcnt[i]);

  initneun();

  loadlbl();
  printf("%d Training Samples\n", xcnt);
  loss = calc();
  printf("Loss: %f\n", loss);
 byebye:
  freeall();
  return EXIT_SUCCESS;
}
