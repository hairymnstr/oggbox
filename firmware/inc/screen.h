#ifndef LCD_H
#define LCD_H 1

// Screen and Font Metrics
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_OFFSET 0
#define FONT_WIDTH 6
#define FONT_HEIGHT 8

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

// fill constants for line and shape drawing
#define FILL_TYPE_NONE          0
#define FILL_TYPE_WHITE         1
#define FILL_TYPE_BLACK         2
#define FILL_TYPE_INVERT        3

void lcdCommand(unsigned char);
void lcdData(unsigned char);
void screen_init();
int screen_shutdown();
void screen_backlight(unsigned short);

void lcd_set_contrast(unsigned char);
unsigned char lcd_get_contrast();

void frame_clear();
void frame_print_at(int x, int y, const char *);
void frame_bar_display(uint8_t x, uint8_t y, uint8_t len, uint8_t percent);
void frame_vline_at(uint8_t x, uint8_t y, uint8_t len);
void frame_show();
void lcd_splash(const char *image[]);

void frame_draw_rect(int x, int y, int width, int height, int fill, int line_style);

#endif /* ifndef LCD_H */