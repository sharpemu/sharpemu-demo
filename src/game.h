#ifndef SHARPEMU_DEMO_GAME_H
#define SHARPEMU_DEMO_GAME_H

#include <stdbool.h>
#include <stdint.h>

enum {
    ViewWidth = 480,
    ViewHeight = 270,
};

typedef struct Input {
    float stick;
    bool cross;
    bool options;
} Input;

extern uint32_t canvas[ViewHeight][ViewWidth];

void game_init(uint32_t seed);
void game_update(float dt, const Input *input);
void game_render(int framesPerSecond);

void platform_set_light_bar(uint32_t color);

#endif
