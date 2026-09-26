#include <libndls.h>
#include "input.h"
enum { UP = 1, DOWN = 2, ACTION = 4, TARGET = 8, LEFT = 16, RIGHT = 32, BACK = 64 };
static unsigned previous;
void input_init(void) { previous = UP | DOWN | ACTION | TARGET | LEFT | RIGHT | BACK; }
Input input_poll(void) {
    Input in = {0};
    unsigned held = 0, pressed;
    bool up = isKeyPressed(KEY_NSPIRE_UP) || isKeyPressed(KEY_NSPIRE_UPRIGHT) || isKeyPressed(KEY_NSPIRE_LEFTUP);
    bool down = isKeyPressed(KEY_NSPIRE_DOWN) || isKeyPressed(KEY_NSPIRE_RIGHTDOWN) || isKeyPressed(KEY_NSPIRE_DOWNLEFT);
    bool left = isKeyPressed(KEY_NSPIRE_LEFT) || isKeyPressed(KEY_NSPIRE_LEFTUP) || isKeyPressed(KEY_NSPIRE_DOWNLEFT);
    bool right = isKeyPressed(KEY_NSPIRE_RIGHT) || isKeyPressed(KEY_NSPIRE_UPRIGHT) || isKeyPressed(KEY_NSPIRE_RIGHTDOWN);
    if (up) held |= UP;
    if (down) held |= DOWN;
    if (isKeyPressed(KEY_NSPIRE_ENTER)) held |= ACTION;
    if (isKeyPressed(KEY_NSPIRE_CTRL)) held |= TARGET;
    if (left) held |= LEFT;
    if (right) held |= RIGHT;
    if (isKeyPressed(KEY_NSPIRE_ESC)) held |= BACK;
    pressed = held & ~previous;
    previous = held;
    in.dx = (int)right - (int)left;
    in.dy = (int)down - (int)up;
    in.up_pressed = (pressed & UP) != 0;
    in.down_pressed = (pressed & DOWN) != 0;
    in.action_pressed = (pressed & ACTION) != 0;
    in.target_pressed = (pressed & TARGET) != 0;
    in.quit = (pressed & BACK) != 0;
    in.left_pressed = (pressed & LEFT) != 0;
    in.right_pressed = (pressed & RIGHT) != 0;
    in.boost = isKeyPressed(KEY_NSPIRE_SHIFT);
    return in;
}
