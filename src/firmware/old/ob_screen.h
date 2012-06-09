#ifndef OB_SCREEN_H
#define OB_SCREEN_H 1

#define WAIT 10

void ob_screen_setup();
void ob_screen_set_bl(unsigned short);
void ob_screen_set_contrast(unsigned short level);

void ob_screen_startup();
void ob_screen_test();

void ob_screen_clear_row(int);
void ob_screen_clear();

int ob_screen_print(char *);

void ob_screen_set_row(int);
int ob_screen_get_row();
void ob_screen_set_col(int);
int ob_screen_get_col();

void ob_screen_set_inv(int);
int ob_screen_get_inv();

#endif /* ifndef OB_SCREEN_H */

