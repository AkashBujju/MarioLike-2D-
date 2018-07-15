#include "Button.h"

void Button::set_text(std::string txt, int size)
{
    text.set_color(0, 0, 0);
    text.set_pos(rect.x + 2, rect.y + 2);
    text.renderer = renderer;
    text.set_text(txt, "SansExtraBold.ttf", size);
    rect.w = text.rect.w + 4;
    rect.h = text.rect.h + 4;
}

void Button::set_pos(int x, int y)
{
    this->rect.x = x;
    this->rect.y = y;
}

void Button::draw()
{
    SDL_Rect border_rect = { rect.x - 1, rect.y - 1, rect.w + 2, rect.h + 2 };
    SDL_SetRenderDrawColor(renderer, b_color.r, b_color.g, b_color.b, 255);
    SDL_RenderDrawRect(renderer, &border_rect);
    if(switched_on)
        SDL_SetRenderDrawColor(renderer, h_color.r, h_color.g, h_color.b, 255);
    else
        SDL_SetRenderDrawColor(renderer, i_color.r, i_color.g, i_color.b, 255);
    SDL_RenderFillRect(renderer, &rect);
    text.draw();
}

bool Button::was_clicked(int x, int y)
{
    if(x > rect.x && x < rect.x + rect.w
    && y > rect.y && y < rect.y + rect.h)
        return true;
    return false;
}

void Button::set_i_color(Uint8 r, Uint8 g, Uint8 b)
{
    i_color = { r, g, b, 255 };
}

void Button::set_b_color(Uint8 r, Uint8 g, Uint8 b)
{
    b_color = { r, g, b, 255 };
}

void Button::set_h_color(Uint8 r, Uint8 g, Uint8 b)
{
    h_color = { r, g, b, 255 };
}