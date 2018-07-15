#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>

struct Text 
{
    std::string text;
    SDL_Texture *texture = nullptr;
    SDL_Renderer *renderer = nullptr;

    Uint8 r = 0, g = 0, b = 0;
    SDL_Rect rect;

    void draw();
    void set_text(std::string txt, std::string font, int size);
    void set_pos(int x, int y);
    void set_color(Uint8 r, Uint8 g, Uint8 b);
    ~Text();
};