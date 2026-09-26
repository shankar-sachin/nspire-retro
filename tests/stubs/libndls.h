#ifndef TEST_LIBNDLS_H
#define TEST_LIBNDLS_H
/* Host harness only. These are not a substitute for an actual SDK build. */
#include <stdbool.h>
typedef enum { SCR_TYPE_INVALID = -1, SCR_320x240_565 = 0 } scr_type_t;
enum {
 KEY_NSPIRE_UP, KEY_NSPIRE_DOWN, KEY_NSPIRE_LEFT, KEY_NSPIRE_RIGHT,
 KEY_NSPIRE_UPRIGHT, KEY_NSPIRE_LEFTUP, KEY_NSPIRE_RIGHTDOWN,
 KEY_NSPIRE_DOWNLEFT, KEY_NSPIRE_ENTER, KEY_NSPIRE_CTRL, KEY_NSPIRE_ESC, KEY_NSPIRE_SHIFT
};
#define has_colors true
bool isKeyPressed(int key);
bool lcd_init(scr_type_t type);
void lcd_blit(void *buffer, scr_type_t type);
int enable_relative_paths(char **argv);
void assert_ndless_rev(unsigned rev);
void wait_no_key_pressed(void);
unsigned msleep(unsigned ms);
#endif
