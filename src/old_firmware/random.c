#include <stdint.h>
#include <stdio.h>

uint32_t lfsr;
uint16_t byte;

void seed(uint32_t seed) {
  if(seed == 0) {
    seed = 1;
  }
  lfsr = seed;
}

uint8_t random() {
  if(byte == 0) {
    lfsr = (lfsr >> 1) ^ (-(lfsr & 1u) & 0xD0000001u);
    byte = 4;
  }
  return ((lfsr >> (8*(--byte))) & 0xff);
}

int main(int argc, char *argv[]) {

  seed(time(NULL));
  int i;
  uint32_t v; 
  uint32_t col[256];

  for(i=0;i<256;i++) {
    col[i] = 0;
  }
  //do {
    /* taps: 32 31 29 1; characteristic polynomial: x^32 + x^31 + x^29 + x + 1 */
  //  lfsr = (lfsr >> 1) ^ (-(lfsr & 1u) & 0xD0000001u); 
  //  ++period;
  //} while(lfsr != 1u);

  for(i=0;i<1000000;i++) {
    v = random();
    col[v]++;
  }
  
  FILE *fw = fopen("output.dat", "w");

  for(i=0;i<256;i++) {
    fprintf(fw, "%d %u\n", i, col[i]);
  }

  fclose(fw);

}

