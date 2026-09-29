#ifndef NSPIRE_RETRO_INPUT_H
#define NSPIRE_RETRO_INPUT_H
#include <stdbool.h>
/* Held movement is separate from edge-triggered actions. No SDK types here. */
typedef struct {
    int dx, dy;
    bool up_pressed, down_pressed, action_pressed, target_pressed, quit;
    bool left_pressed, right_pressed, boost, action_held, action_released;
} Input;
void input_init(void);
Input input_poll(void);
#endif
