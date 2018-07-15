#include "Grid.h"

void Grid::draw()
{
    int x_start = grid_offset_x;
    int y_start = grid_offset_y;
    int x_end = x_start + grid_width;
    int y_end = y_start + grid_height;

    int per_width = grid_width / num_cols;
    int per_height = grid_height / num_rows;

    SDL_SetRenderDrawColor(renderer, r, g, b, 255);

    while (x_start <= x_end)
    {
        SDL_RenderDrawLine(renderer, x_start, y_start, x_start, y_end);

        x_start += per_width;
    }

    x_start -= grid_width + per_width;

    while (y_start <= y_end)
    {
        SDL_RenderDrawLine(renderer, x_start, y_start, x_end, y_start);

        y_start += per_height;
    }
}

void Grid::clear()
{
    for(int i = 0; i < objects.size(); i++)
        objects[i] = EMPTY;
}

void Grid::fill_grid(int grid, Uint8 r, Uint8 g, Uint8 b)
{
    Point tmp_p = points[grid];
    SDL_Rect tmp_rect{tmp_p.x + 1, tmp_p.y + 1, per_width - 2, per_height - 2};
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderFillRect(renderer, &tmp_rect);
}

int Grid::get_x_grid(int x)
{
    int width_from_begin = abs(x - grid_offset_x);
    return ((float)width_from_begin / grid_width * num_cols);
}

int Grid::get_y_grid(int y)
{
    int height_from_begin = abs(y - grid_offset_y);
    return ((float)height_from_begin / grid_height * num_rows);
}

int Grid::get_grid(int x_grid, int y_grid)
{
    return (y_grid * num_rows + x_grid);
}

void Grid::set_object(int grid, Game_Object obj)
{
    objects[grid] = obj;
}

Game_Object Grid::get_object(int grid)
{
    return objects[grid];
}

void Grid::highlight_grids()
{
    int total_grids = num_cols * num_rows;
    for (int i = 0; i < total_grids; i++)
    {
        if (objects[i] == Game_Object::RECT)
            fill_grid(i, 190, 120, 55);
        else if(objects[i] == Game_Object::POINT)
            fill_grid(i, 255, 255, 51);
        else if(objects[i] == Game_Object::EXIT)
            fill_grid(i, 47, 79, 79);
    }
}

void Grid::set_dim(int off_x, int off_y, int w, int h, int rows, int cols)
{
    grid_offset_x = off_x;
    grid_offset_y = off_y;
    grid_width = w;
    grid_height = h;
    num_rows = rows;
    num_cols = cols;
}

void Grid::set_col(Uint8 r, Uint8 g, Uint8 b, SDL_Renderer *ren)
{
    this->r = r;
    this->g = g;
    this->b = b;
    renderer = ren;
}

void Grid::init()
{
    int x_start = grid_offset_x;
    int y_start = grid_offset_y;
    int x_end = x_start + grid_width;
    int y_end = y_start + grid_height;

    per_width = grid_width / num_cols;
    per_height = grid_height / num_rows;

    total_grids = num_cols * num_rows;
    for (int i = 0; i < total_grids; i++)
    {
        objects.push_back(EMPTY);
    }

    for (int i = y_start; i < y_end; i += per_height)
    {
        for (int j = x_start; j < x_end; j += per_width)
        {
            Point p;
            p.x = j;
            p.y = i;
            points.push_back(p);
        }
    }
}