#ifndef LCD_H
#define LCD_H 1

// Sitronix ST7565P Status Masks
#define LCD_BUSY                0x80
#define LCD_ADC                 0x40
#define LCD_ON                  0x20
#define LCD_RESET               0x10

// Sitronix ST7565P Command codes
#define LCD_DISPLAY_ON          0xAF
#define LCD_DISPLAY_OFF         0xAE
#define LCD_START_LINE(line)    (0x80 + (line))
#define LCD_PAGE_SET(page)      (0xB0 + (page))
#define LCD_COLUMN_SET_HI(col)  (0x10 + (((col) & 0xF0) >> 4))
#define LCD_COLUMN_SET_LO(col)  ((col) & 0x0F)
#define LCD_DIRECTION_FWD       0xA0
#define LCD_DIRECTION_REV       0xA1
#define LCD_COLOUR_NORM         0xA6
#define LCD_COLOUR_INV          0xA7
#define LCD_BLANK               0xA5            // doesn't affect contents of RAM only display
#define LCD_UNBLANK             0xA4
#define LCD_BIAS_NINTH          0xA2
#define LCD_BIAS_SEVENTH        0xA3
#define LCD_MODIFY_START        0xE0
#define LCD_MODIFY_END          0xEE
#define LCD_SOFT_RESET          0xE2
#define LCD_COMMON_FWD          0xC0
#define LCD_COMMON_REV          0xC8
#define LCD_POWER_SETUP         0x2F            // this is the only correct value for the BATRON module
#define LCD_VREG_SET            0x25            // this works??
#define LCD_CONTRAST_HI         0x81
#define LCD_CONTRAST_LO(con)    (con & 0x3F)
#define LCD_CURSOR_START        0xAD
#define LCD_CURSOR_OFF          0x00
#define LCD_CURSOR_SLOW         0x01            // 1 second blink cursor
#define LCD_CURSOR_FAST         0x02            // 0.5 second blink cursor
#define LCD_CURSOR_ON           0x03
#define LCD_CURSOR_END          0xAC

void lcdCommand(unsigned char);
void lcdData(unsigned char);
unsigned char lcdStatus();
void lcdInit();
void lcdClear();
void lcdBacklight(unsigned short);
void lcdPrintPortrait(char *, char);
void lcdBlit(unsigned char *img, unsigned char rows, unsigned char cols, unsigned char x, unsigned char y);
void lcdBlitPortrait(unsigned char *img, unsigned char rows, unsigned char cols, unsigned char x, unsigned char y);
#endif /* ifndef LCD_H */