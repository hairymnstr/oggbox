#include <stdint.h>
#include "lcd.h"
#include "icons.h"

void uiShowSD(int16_t present) {
  if(!present) {
    lcdBlit(sdpresentimg, 8, 8, 0, 0);
  } else {
    lcdBlit(sdabsentimg, 8, 8, 0, 0);
  }
}

void uiShowLocked(int16_t locked) {
  if(locked) {
    lcdBlit(lockedimg, 8, 8, 8, 0);
  } else {
    lcdBlit(unlockedimg, 8, 8, 8, 0);
  }
}