#ifndef LCD_H
#define LCD_H 1

#define LCD_BUSY 0x80

void lcdCommand(unsigned char);
void lcdData(unsigned char);
volatile unsigned char lcdStatus();
void lcdInit();
void lcdBacklight(unsigned short);

#endif /* ifndef LCD_H */