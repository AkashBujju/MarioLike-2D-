#pragma once
#include <SDL2/SDL.h>

struct Rectangle
{
    SDL_Rect rect;
    Uint8 r = 0, g = 0, b = 0;
    SDL_Renderer *renderer = nullptr;

    int h_speed = 0;
    int v_speed = 0;
    int grid = -1;

    void draw();
    void set(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b, SDL_Renderer *ren);
};