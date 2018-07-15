#pragma once
#include <SDL2/SDL.h>
#include "Text.h"

struct Button
{
    Text text;
    SDL_Renderer *renderer = nullptr;
    SDL_Color b_color = { 0, 0, 0, 0 };
    SDL_Color i_color = { 0, 0, 0, 0 };
    SDL_Color h_color = { 0, 0, 0, 0 };
    SDL_Rect rect;

    bool switched_on = false;

    void set_text(std::string txt, int size);
    void set_pos(int x, int y);
    void set_i_color(Uint8 r, Uint8 g, Uint8 b);
    void set_b_color(Uint8 r, Uint8 g, Uint8 b);
    void set_h_color(Uint8 r, Uint8 g, Uint8 b);
    void draw();
    bool was_clicked(int x, int y);
};