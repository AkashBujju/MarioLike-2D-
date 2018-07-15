#include "Rectangle.h"

void Rectangle::draw()
{
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderFillRect(renderer, &rect);
}

void Rectangle::set(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b, SDL_Renderer *ren)
{
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    this->r = r;
    this->g = g;
    this->b = b;
    renderer = ren;
}