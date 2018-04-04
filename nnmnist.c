#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <tcclib.h>
#include <math.h>

int L = 2; // hidden layers
int N = 20; // 20 neurons per a hidden layer
int K = 28*28; // input neurons, 28*28 grayscale
int D = 10; // 10 digits
#define LBL "train-labels.idx1-ubyte"
#define IMG "train-images.idx3-ubyte"
char *lbl, *lmg;

struct neuron {
  float w[K];
  float b;
  float z;
  float a;
};

float sigmoid(float x) {
  x = 1.0 + exp(x);
  return 1.0/x;
}

float dsigmoid(float x) {
  float ex = exp(x);
  x = 1.0+1.0/ex;
  x = ex*x*x;
  return 1.0/x;
}

struct layer { // hidden layer
  struct neuron nx[N];
} layers[L];

struct olayer { // output layer
  struct neuron nx[D];
} o;

int in[K]; // input layer

int getlbl(int no) { // no starts from 0
  return (int)lbl[4+4+no+1];
}

int getimg(int no, int noo) {
  // no starts from 0, noo also starts from 0
  // get the noo th pixel of the no th image
  return (int)img[4+4+4+4+28*28*no+noo+1];
}

int main() {
  int i;
  int lblfd = open(LBL, O_RDONLY);
  int imgfd = open(IMG, O_RDONLY);
  int lbllen = lseek(lblfd, 0, SEEK_END)+1;
  int imglen = lseek(imgfd, 0, SEEK_END)+1;
  lbl = (char*)mmap(NULL, len, PROT_READ, MAP_PRIVATE, lblfd, 0);
  img = (char*)mmap(NULL, len, PROT_READ, MAP_PRIVATE, imgfd, 0);

  while (1) {
    // todo train
  }

  munmap(lbl, lbllen);
  munmap(img, imglen);
  close(lblfd);
  close(imgfd);
}
