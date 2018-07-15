#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include "Point.h"
#include "Game_Objects.h"

struct Grid
{
    int grid_offset_x = 0;
    int grid_offset_y = 0;
    int grid_width = 0;
    int grid_height = 0;
    int num_rows = 0;
    int num_cols = 0;
    int per_width = 0;
    int per_height = 0;
    int total_grids = 0;

    std::vector<Point> points;
    std::vector<Game_Object> objects;

    Uint8 r = 0, g = 0, b = 0;
    SDL_Renderer *renderer = nullptr;

    void draw();
    void clear();
    void set_dim(int off_x, int off_y, int w, int h, int rows, int cols);
    void set_col(Uint8 r, Uint8 g, Uint8 b, SDL_Renderer *ren);
    void fill_grid(int grid, Uint8 r, Uint8 g, Uint8 b);
    void set_object(int grid, Game_Object obj);
    Game_Object get_object(int grid);
    void highlight_grids();
    void init();
    int get_x_grid(int x);
    int get_y_grid(int y);
    int get_grid(int x_grid, int y_grid);
};