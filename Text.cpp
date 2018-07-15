#include "Text.h"

void Text::draw()
{
    SDL_RenderCopy(renderer, texture, NULL, &rect);
}

void Text::set_color(Uint8 r, Uint8 g, Uint8 b)
{
    this->r = r;
    this->g = g;
    this->b = b;
}

void Text::set_pos(int x, int y)
{
    rect.x = x;
    rect.y = y;
}

void Text::set_text(std::string txt, std::string font, int size)
{
    SDL_DestroyTexture(texture);
    TTF_Font *f = TTF_OpenFont(font.c_str(), size);
    SDL_Color color = { r, g, b };
    SDL_Surface *tmp = TTF_RenderText_Solid(f, txt.c_str(), color);
    if(!tmp)
    {
        std::cout << "Error: " << SDL_GetError() << std::endl;
    }
    texture = SDL_CreateTextureFromSurface(renderer, tmp);
    SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
    SDL_FreeSurface(tmp);
    TTF_CloseFont(f);
}

Text::~Text()
{
    SDL_DestroyTexture(texture);
    texture = nullptr;
}