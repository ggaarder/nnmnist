#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "ntwkarg.h"

int main() {
  int fd = open(ARG, O_RDONLY);
  int len = lseek(fd, 0, SEEK_END)+1;
  char *raw = (char*)mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
  struct header *hp = (struct header*)raw;
  struct header h = *hp;
  float *arg = (float*)(raw+sizeof(struct header));
  int l, n, k;

  printf("%d Hidden Layers\n", h.L);
  printf("%d Neurons Per Hidden Layer\n", h.N);
  printf("%d Input Neurons\n", h.K);
  printf("%d Neurons in the Output Layout\n", h.D);

  for (l = 0; l < h.L; ++l) {
    printf("Hidden Layer #%d\n", l);
    for (n = 0; n < h.N; ++n) {
      printf("@%03d ", n);
      for (k = 0; k <= h.K; ++k)
        printf("%.2f ", *WEIG);
      printf("\n");
    }
  }

  munmap(raw, len);
}
