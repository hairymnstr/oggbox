#ifndef INTERFACE_H
#define INTERFACE_H 1

#define MENU_BTN_FLAG   (1 << 0)
#define SET_BTN_FLAG    (1 << 1)
#define UP_BTN_FLAG     (1 << 2)
#define DOWN_BTN_FLAG   (1 << 3)
#define LEFT_BTN_FLAG   (1 << 4)
#define RIGHT_BTN_FLAG  (1 << 5)
#define VOL_UP_BTN_FLAG (1 << 6)
#define VOL_DN_BTN_FLAG (1 << 7)

#define DISPLAY_MODE_PREVIOUS           0
#define DISPLAY_MODE_NOW_PLAYING        1
#define DISPLAY_MODE_MENU               2
#define DISPLAY_MODE_PLAYLIST           3
#define DISPLAY_MODE_SETTINGS           4
#define DISPLAY_MODE_MEDIA              5

void start_interface_task();

#endif /* ifndef INTERFACE_H */
