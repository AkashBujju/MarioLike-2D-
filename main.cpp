#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <fstream>
#include "Grid.h"
#include "Rectangle.h"
#include "Text.h"
#include "Button.h"

const int scr_width = 840;
const int scr_height = 800;
SDL_Renderer *renderer = nullptr;
SDL_Window *window = nullptr;
SDL_Event ev;
bool running = true;
const Uint8 *keystate = nullptr;

const int gravity = 10;
int padding = 2;
bool apply_gravity = true;
bool draw_grid = false;
bool p_jump = false;
bool edit_mode = false;
bool pause = false;
bool game_over = false;
SDL_Rect menu_border = {10, 10, scr_width - 20, 140};
SDL_Rect background_rect = {0, 0, scr_width, scr_width};
SDL_Rect input_text_rect = {13, 90, 300, 40};

Grid grid;
Rectangle player;
std::vector<Rectangle> enemies;
std::vector<std::string> levels;
int current_level = -1;
int num_lives = 3;
int total_points = 0;
double time_elapsed = 0.0;
double start_sec = 0.0;
double time_limit = 120.0;
std::string text_input;

Text lives_text;
Text gameover_text;
Text points_text;
Text level_text;
Text time_text;
Text input_text;

Button grid_button;
Button editmode_button;
Button pause_button;
Button gravity_button;
Button save_button;
Button load_button;
Button restart_button;

Mix_Chunk *coin_effect;
Mix_Chunk *jump_effect;
Mix_Chunk *dead_effect;
Mix_Chunk *level_clear_effect;
Mix_Chunk *game_over_effect;
Mix_Music *sound_track;

void init();
void update();
void render();
void events();
void load(std::string filename);
void save(std::string filename);

void update_player(Rectangle &player_rect);
void update_enemy(Rectangle &enemy_rect);
void player_jump(Rectangle &p_rect);
void update_grid(Rectangle &rectangle);
void update_grid_positions();
void check_player_collisions();
void add_enemy(std::vector<Rectangle> &ene, int x, int y);
void remove_enemy(std::vector<Rectangle> &ene, int x, int y);
void check_for_exit();

void update_lives_text();
void update_points_text();
void update_level_text();
void update_time_text();
void update_input_text();
void show_game_over();

void setup_ui();

Game_Object get_left_1(SDL_Rect &rect);
Game_Object get_left_2(SDL_Rect &rect);
Game_Object get_right_1(SDL_Rect &rect);
Game_Object get_right_2(SDL_Rect &rect);
Game_Object get_up_1(SDL_Rect &rect);
Game_Object get_up_2(SDL_Rect &rect);
Game_Object get_down_1(SDL_Rect &rect);
Game_Object get_down_2(SDL_Rect &rect);
Game_Object get_type(int &x, int &y);

int main()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cout << "Could not initialise sdl: " << SDL_GetError();
        std::cout << std::endl;
        return -1;
    }

    window = SDL_CreateWindow("Rectangles_2", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, scr_width, scr_height, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    TTF_Init();

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        std::cout << Mix_GetError() << std::endl;
    }

    init();
    Uint32 start_tick = 0;
    const int fps = 60;
    start_sec = SDL_GetTicks();

    while (running)
    {
        start_tick = SDL_GetTicks();
        time_elapsed = abs(SDL_GetTicks() - start_sec) / 1000.0f;
        keystate = SDL_GetKeyboardState(NULL);
        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_QUIT)
                running = false;
            events();
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        render();
        SDL_RenderPresent(renderer);

        if (!game_over)
            if (!pause)
                update();

        if ((fps / 1000) > (SDL_GetTicks() - start_tick))
            SDL_Delay(fps / 1000 - (SDL_GetTicks() - start_tick));
    }

    Mix_FreeChunk(coin_effect);
    Mix_FreeChunk(jump_effect);
    Mix_FreeChunk(dead_effect);
    Mix_FreeChunk(level_clear_effect);
    Mix_FreeChunk(game_over_effect);
    Mix_FreeMusic(sound_track);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void add_enemy(std::vector<Rectangle> &ene, int x, int y)
{
    Rectangle tmp_rect;
    tmp_rect.set(x, y, grid.per_width - 10, grid.per_height - 10,
                 201, 28, 28, renderer);
    tmp_rect.h_speed = 3;
    ene.push_back(tmp_rect);
}

void remove_enemy(std::vector<Rectangle> &ene, int x, int y)
{
    for (int i = 0; i < ene.size(); i++)
    {
        int start_x = enemies[i].rect.x;
        int end_x = start_x + enemies[i].rect.w;
        int start_y = enemies[i].rect.y;
        int end_y = start_y + enemies[i].rect.h;
        if (x > start_x && x < end_x && y > start_y && y < end_y)
            ene.erase(ene.begin() + i);
    }
}

void check_for_exit()
{
    Game_Object tmp_obj = grid.get_object(player.grid);
    if (tmp_obj == Game_Object::EXIT)
    {
        // Check if all points are taken
        bool taken = true;
        for (int i = 0; i < grid.objects.size(); i++)
        {
            if (grid.objects[i] == Game_Object::POINT)
                taken = false;
        }
        if (taken)
        {
            Mix_PlayChannel(-1, level_clear_effect, 0);
            total_points = 0;
            update_points_text();
            current_level++;
            update_level_text();
            start_sec = SDL_GetTicks();
            if (current_level < levels.size())
                load(levels[current_level]);
        }
    }
}

void init()
{
    grid.set_dim(0, 160, 840, 640, 20, 20);
    grid.set_col(255, 0, 0, renderer);
    grid.init();

    player.set(grid.grid_offset_x + 10, grid.grid_offset_y + 10,
               15, 15, 50, 50, 200, renderer);
    player.h_speed = 3;

    levels.push_back("Level_1");
    levels.push_back("Level_2");
    current_level = 0;

    load(levels[current_level]);

    setup_ui();

    coin_effect = Mix_LoadWAV("coin.wav");
    jump_effect = Mix_LoadWAV("jump.wav");
    dead_effect = Mix_LoadWAV("stomp.wav");
    level_clear_effect = Mix_LoadWAV("stage_clear.wav");
    game_over_effect = Mix_LoadWAV("dead.wav");
    sound_track = Mix_LoadMUS("sound_track.mp3");

    Mix_PlayMusic(sound_track, -1);
}

void show_game_over()
{
    Mix_PauseMusic();
    Mix_PlayChannel(-1, game_over_effect, 0);
    SDL_Rect tmp_rect;
    tmp_rect.x = gameover_text.rect.x - 20;
    tmp_rect.y = gameover_text.rect.y - 20;
    tmp_rect.w = gameover_text.rect.w + 40;
    tmp_rect.h = gameover_text.rect.h + 40;
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_RenderFillRect(renderer, &tmp_rect);
    gameover_text.draw();
}

void update()
{
    update_player(player);

    for (int i = 0; i < enemies.size(); i++)
        update_enemy(enemies[i]);

    update_grid_positions();
    check_player_collisions();
    check_for_exit();

    // Timer
    update_time_text();

    if (time_elapsed >= time_limit)
    {
        num_lives--;
        total_points = 0;
        start_sec = SDL_GetTicks();
        load(levels[current_level]);
        update_lives_text();
        update_points_text();
    }

    if (num_lives < 0)
    {
        game_over = true;
        show_game_over();
    }
}

void render()
{
    SDL_SetRenderDrawColor(renderer, 150, 210, 220, 255);
    SDL_RenderFillRect(renderer, &background_rect);

    if (draw_grid)
        grid.draw();

    grid.highlight_grids();

    player.draw();

    for (int i = 0; i < enemies.size(); i++)
        enemies[i].draw();

    SDL_SetRenderDrawColor(renderer, 75, 120, 200, 255);
    SDL_RenderFillRect(renderer, &menu_border);

    lives_text.draw();
    points_text.draw();
    level_text.draw();
    time_text.draw();

    if (save_button.switched_on || load_button.switched_on)
    {
        SDL_SetRenderDrawColor(renderer, 75, 120, 20, 255);
        SDL_RenderFillRect(renderer, &input_text_rect);
        input_text.draw();
    }

    grid_button.draw();
    editmode_button.draw();
    pause_button.draw();
    gravity_button.draw();
    save_button.draw();
    load_button.draw();
    restart_button.draw();

    if (game_over)
    {
        show_game_over();
    }
}

void update_grid_positions()
{
    update_grid(player);
    for (int i = 0; i < enemies.size(); i++)
        update_grid(enemies[i]);
}

void check_player_collisions()
{
    // Enemy collisons
    int player_grid = player.grid;
    for (int i = 0; i < enemies.size(); i++)
    {
        if (enemies[i].grid == player_grid)
        {
            Mix_PlayChannel(-1, dead_effect, 0);
            load(levels[current_level]);
            num_lives--;
            update_lives_text();
            total_points = 0;
            update_points_text();
            start_sec = SDL_GetTicks();
            if (num_lives == 0)
                game_over = true;
        }
    }

    // Update points
    Game_Object tmp_obj = grid.get_object(player_grid);
    if (tmp_obj == Game_Object::POINT)
    {
        Mix_PlayChannel(-1, coin_effect, 0);
        grid.set_object(player_grid, Game_Object::EMPTY);
        total_points++;
        update_points_text();
    }
}

void update_lives_text()
{
    std::string f_string = "Lives: ";
    f_string += std::to_string(num_lives);
    lives_text.set_text(f_string, "SansExtraBold.ttf", 25);
}

void update_points_text()
{
    std::string f_string = "Points: ";
    f_string += std::to_string(total_points);
    points_text.set_text(f_string, "SansExtraBold.ttf", 25);
}

void update_level_text()
{
    std::string f_string = "Level: ";
    f_string += std::to_string(current_level + 1);
    level_text.set_text(f_string, "SansExtraBold.ttf", 25);
}

void update_time_text()
{
    std::string f_string = "Time: ";
    f_string += std::to_string(time_limit - time_elapsed);
    time_text.set_text(f_string, "SansExtraBold.ttf", 25);
}

void update_input_text()
{
    std::string f_string = "Input: ";
    f_string += text_input;
    input_text.set_text(f_string, "SansExtraBold.ttf", 25);
}

void update_player(Rectangle &player_rect)
{
    Game_Object left_1 = get_left_1(player_rect.rect);
    Game_Object left_2 = get_left_2(player_rect.rect);
    Game_Object right_1 = get_right_1(player_rect.rect);
    Game_Object right_2 = get_right_2(player_rect.rect);
    Game_Object up_1 = get_up_1(player_rect.rect);
    Game_Object up_2 = get_up_2(player_rect.rect);
    Game_Object down_1 = get_down_1(player_rect.rect);
    Game_Object down_2 = get_down_2(player_rect.rect);

    if (keystate[SDL_SCANCODE_LEFT])
    {
        if (left_1 != RECT && left_2 != RECT)
            player_rect.rect.x -= player_rect.h_speed;
    }
    else if (keystate[SDL_SCANCODE_RIGHT])
    {
        if (right_1 != RECT && right_2 != RECT)
            player_rect.rect.x += player_rect.h_speed;
    }

    if (edit_mode)
    {
        if (keystate[SDL_SCANCODE_UP])
            player_rect.rect.y -= player_rect.h_speed;
        else if (keystate[SDL_SCANCODE_DOWN])
            player_rect.rect.y += player_rect.h_speed;
    }

    if (up_1 == RECT || up_2 == RECT)
        player_rect.rect.y += padding;

    if (down_1 != RECT && down_2 != RECT && !p_jump)
        player_rect.v_speed = gravity;
    else
        player_rect.v_speed = 0;

    if (apply_gravity)
        player_rect.rect.y += player_rect.v_speed;

    player_jump(player_rect);

    // Keeping player within grid
    if (player_rect.rect.x + player_rect.rect.w > grid.grid_offset_x + grid.grid_width)
        player_rect.rect.x -= player_rect.h_speed;
    else if (player.rect.x < grid.grid_offset_x)
        player_rect.rect.x += player_rect.h_speed;
    else if (player_rect.rect.y + player_rect.rect.h > grid.grid_offset_y + grid.grid_height)
        player_rect.rect.y -= player_rect.h_speed;
    else if (player.rect.y < grid.grid_offset_y)
        player_rect.rect.y = grid.grid_offset_y;
}

void update_enemy(Rectangle &enemy_rect)
{
    Game_Object enemy_1_left_1 = get_left_1(enemy_rect.rect);
    Game_Object enemy_1_left_2 = get_left_2(enemy_rect.rect);
    Game_Object enemy_1_right_1 = get_right_1(enemy_rect.rect);
    Game_Object enemy_1_right_2 = get_right_2(enemy_rect.rect);
    Game_Object enemy_1_down_1 = get_down_1(enemy_rect.rect);
    Game_Object enemy_1_down_2 = get_down_2(enemy_rect.rect);

    if (enemy_1_down_1 == RECT || enemy_1_down_2 == RECT)
    {
        if (enemy_1_left_1 == RECT || enemy_1_left_2 == RECT)
        {
            enemy_rect.h_speed = -enemy_rect.h_speed;
            enemy_rect.rect.x += 5;
        }
        else if (enemy_1_right_1 == RECT || enemy_1_right_2 == RECT)
        {
            enemy_rect.h_speed = -enemy_rect.h_speed;
            enemy_rect.rect.x -= 5;
        }
    }

    if (enemy_1_down_1 != RECT && enemy_1_down_2 != RECT)
        enemy_rect.v_speed = gravity;
    else
    {
        enemy_rect.rect.x += enemy_rect.h_speed;
        enemy_rect.v_speed = 0;
    }

    if (apply_gravity)
        enemy_rect.rect.y += enemy_rect.v_speed;
}

void events()
{

    if (ev.type == SDL_TEXTINPUT || ev.type == SDL_KEYDOWN)
    {
        if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_BACKSPACE && text_input.length() > 0)
        {
            text_input = text_input.substr(0, text_input.length() - 1);
            update_input_text();
        }
        else if (ev.type == SDL_TEXTINPUT && pause)
        {
            text_input += ev.text.text;
            update_input_text();
        }

        else if (ev.key.keysym.sym == SDLK_SPACE && !pause)
        {
            Game_Object down_1 = get_down_1(player.rect);
            Game_Object down_2 = get_down_2(player.rect);
            if (down_1 == RECT || down_2 == RECT)
            {
                player.rect.y -= 10;
                p_jump = true;
                Mix_PlayChannel(-1, jump_effect, 0);
            }
        }
    }

    else if (ev.type == SDL_MOUSEBUTTONDOWN || ev.type == SDL_MOUSEMOTION)
    {
        int x, y;
        SDL_GetMouseState(&x, &y);

        if (ev.button.button == SDL_BUTTON_LEFT)
        {
            if (grid_button.was_clicked(x, y))
            {
                grid_button.switched_on = !grid_button.switched_on;
                draw_grid = !draw_grid;
            }
            else if (editmode_button.was_clicked(x, y))
            {
                editmode_button.switched_on = !editmode_button.switched_on;
                edit_mode = !edit_mode;
            }
            else if (pause_button.was_clicked(x, y))
            {
                pause_button.switched_on = !pause_button.switched_on;
                pause = !pause;
            }
            else if (gravity_button.was_clicked(x, y))
            {
                gravity_button.switched_on = !gravity_button.switched_on;
                apply_gravity = !apply_gravity;
            }
            else if (save_button.was_clicked(x, y))
            {
                save_button.switched_on = !save_button.switched_on;
                if (save_button.switched_on)
                {
                    SDL_StartTextInput();
                    pause = true;
                }
                else
                {
                    SDL_StopTextInput();
                    save(text_input);
                    text_input = "";
                    update_input_text();
                    pause = false;
                }
            }
            else if (load_button.was_clicked(x, y))
            {
                load_button.switched_on = !load_button.switched_on;
                if (load_button.switched_on)
                {
                    SDL_StartTextInput();
                    pause = true;
                }
                else
                {
                    SDL_StopTextInput();
                    load(text_input);
                    text_input = "";
                    update_input_text();
                    pause = false;
                }
            }
            else if (restart_button.was_clicked(x, y))
            {
                Mix_ResumeMusic();
                current_level = 0;
                total_points = 0;
                num_lives = 3;
                game_over = false;
                start_sec = SDL_GetTicks();
                update_level_text();
                update_lives_text();
                update_points_text();
                update_time_text();
                load(levels[current_level]);
            }
        }

        if (edit_mode)
        {
            // Make sure Mouse click is inside grid
            if (x < grid.grid_offset_x || x > (grid.grid_offset_x + grid.grid_width) || y < grid.grid_offset_y || y > (grid.grid_offset_y + grid.grid_height))
                return;

            int tmp_x_grid = grid.get_x_grid(x);
            int tmp_y_grid = grid.get_y_grid(y);
            int tmp_grid = grid.get_grid(tmp_x_grid, tmp_y_grid);

            if (ev.button.button == SDL_BUTTON_LEFT && keystate[SDL_SCANCODE_1])
                grid.set_object(tmp_grid, Game_Object::RECT);
            else if (ev.button.button == SDL_BUTTON_LEFT && keystate[SDL_SCANCODE_2])
                grid.set_object(tmp_grid, Game_Object::POINT);
            else if (ev.button.button == SDL_BUTTON_RIGHT)
                grid.set_object(tmp_grid, Game_Object::EMPTY);

            // Setting exit
            else if (ev.button.button == SDL_BUTTON_LEFT && keystate[SDL_SCANCODE_3])
                grid.set_object(tmp_grid, Game_Object::EXIT);

            // Adding enemies
            else if (ev.button.button == SDL_BUTTON_LEFT && keystate[SDL_SCANCODE_A])
                add_enemy(enemies, x, y);

            // Removing enemies
            else if (ev.button.button == SDL_BUTTON_LEFT && keystate[SDL_SCANCODE_R])
                remove_enemy(enemies, x, y);

            // Moving enemies
            else if (ev.button.button == SDL_BUTTON_LEFT && keystate[SDL_SCANCODE_SPACE])
            {
                for (int i = 0; i < enemies.size(); i++)
                {
                    int start_x = enemies[i].rect.x;
                    int end_x = start_x + enemies[i].rect.w;
                    int start_y = enemies[i].rect.y;
                    int end_y = start_y + enemies[i].rect.h;

                    if (x > start_x && x < end_x && y > start_y && y < end_y)
                    {
                        enemies[i].rect.x = x - enemies[i].rect.w / 2;
                        enemies[i].rect.y = y - enemies[i].rect.h / 2;
                    }
                }
            }
        }
    }
}

void player_jump(Rectangle &p_rect)
{
    if (p_jump)
    {
        static float y_speed = -10.0;
        y_speed += 0.6f;
        Game_Object up_1 = get_up_1(p_rect.rect);
        Game_Object up_2 = get_up_2(p_rect.rect);

        if (up_1 == RECT || up_2 == RECT)
        {
            if (abs(y_speed) >= 5.0)
                p_rect.rect.y += 10;
            y_speed = 0.0;
        }

        p_rect.rect.y += y_speed;

        if (y_speed >= 0.0)
        {
            p_jump = false;
            y_speed = -10.0;
        }
    }
}

void load(std::string filename)
{
    filename += ".bin";
    std::ifstream read;
    read.open(filename, std::ios::binary);
    if (!read)
    {
        std::cout << "Error: " << filename << std::endl;
        return;
    }

    // Setting all grid to EMPTY
    grid.clear();

    // Player position
    int tmp_x, tmp_y;
    read.read((char *)&tmp_x, sizeof(int));
    read.read((char *)&tmp_y, sizeof(int));
    player.rect.x = tmp_x;
    player.rect.y = tmp_y;

    // Num Enemies
    int num_enemies;
    read.read((char *)&num_enemies, sizeof(int));

    // Clear vector<Rectangle> enemies
    enemies.erase(enemies.begin(), enemies.end());

    // add enemies
    for (int i = 0; i < num_enemies; i++)
    {
        int _x, _y;
        read.read((char *)&_x, sizeof(int));
        read.read((char *)&_y, sizeof(int));
        add_enemy(enemies, _x, _y);
    }

    while (!read.eof())
    {
        Game_Object tmp_object;
        int tmp_grid;
        read.read((char *)&tmp_grid, sizeof(int));
        read.read((char *)&tmp_object, sizeof(Game_Object));
        grid.set_object(tmp_grid, tmp_object);
    }

    read.close();
}

void save(std::string filename)
{
    filename += ".bin";

    std::ofstream write;
    write.open(filename, std::ios::binary);

    // Player position
    write.write((char *)&player.rect.x, sizeof(int));
    write.write((char *)&player.rect.y, sizeof(int));

    // Number of enemies
    int num_enemies = enemies.size();
    write.write((char *)&num_enemies, sizeof(int));

    // All positions of enemies
    for (int i = 0; i < enemies.size(); i++)
    {
        int tmp_x = enemies[i].rect.x;
        int tmp_y = enemies[i].rect.y;
        write.write((char *)&tmp_x, sizeof(int));
        write.write((char *)&tmp_y, sizeof(int));
    }

    int total = grid.total_grids;
    for (int i = 0; i < total; i++)
    {
        Game_Object o = grid.get_object(i);
        if (o != EMPTY)
        {
            write.write((char *)&i, sizeof(int));
            write.write((char *)&o, sizeof(Game_Object));
        }
    }

    write.close();
}

Game_Object get_left_1(SDL_Rect &rect)
{
    Point lu1;
    lu1.x = rect.x;
    lu1.y = rect.y + padding;
    int gr = grid.get_grid(grid.get_x_grid(lu1.x), grid.get_y_grid(lu1.y));
    Game_Object res = grid.get_object(gr);
    return res;
}

Game_Object get_left_2(SDL_Rect &rect)
{
    Point ld1;
    ld1.x = rect.x;
    ld1.y = rect.y + rect.h - padding;
    int gr = grid.get_grid(grid.get_x_grid(ld1.x), grid.get_y_grid(ld1.y));
    Game_Object res = grid.get_object(gr);
    return res;
}

Game_Object get_right_1(SDL_Rect &rect)
{
    Point ru2;
    ru2.x = rect.x + rect.w;
    ru2.y = rect.y + padding;
    return get_type(ru2.x, ru2.y);
}

Game_Object get_right_2(SDL_Rect &rect)
{
    Point rd1;
    rd1.x = rect.x + rect.w;
    rd1.y = rect.y + rect.h - padding;
    return get_type(rd1.x, rd1.y);
}

Game_Object get_up_1(SDL_Rect &rect)
{
    Point lu2;
    lu2.x = rect.x + padding;
    lu2.y = rect.y;
    return get_type(lu2.x, lu2.y);
}

Game_Object get_up_2(SDL_Rect &rect)
{
    Point ru1;
    ru1.x = rect.x + rect.w - padding;
    ru1.y = rect.y;
    return get_type(ru1.x, ru1.y);
}

Game_Object get_down_1(SDL_Rect &rect)
{
    Point rd2;
    rd2.x = rect.x + rect.w - padding;
    rd2.y = rect.y + rect.h + 10;
    return get_type(rd2.x, rd2.y);
}

Game_Object get_down_2(SDL_Rect &rect)
{
    Point ld2;
    ld2.x = rect.x + padding;
    ld2.y = rect.y + rect.h + 10;
    return get_type(ld2.x, ld2.y);
}

Game_Object get_type(int &x, int &y)
{
    int gr = grid.get_grid(grid.get_x_grid(x), grid.get_y_grid(y));
    return grid.get_object(gr);
}

void update_grid(Rectangle &rectangle)
{
    int x = rectangle.rect.x + rectangle.rect.w / 2;
    int y = rectangle.rect.y + rectangle.rect.h / 2;
    rectangle.grid = grid.get_grid(grid.get_x_grid(x), grid.get_y_grid(y));
}

void setup_ui()
{
    // UI
    lives_text.set_color(250, 150, 200);
    lives_text.set_pos(15, 12);
    lives_text.renderer = renderer;
    update_lives_text();

    gameover_text.set_color(0, 0, 0);
    gameover_text.set_pos(scr_width / 2 - 200, scr_height / 2 - 100);
    gameover_text.renderer = renderer;
    gameover_text.set_text("Game Over", "SansExtraBold.ttf", 60);

    points_text.set_color(250, 150, 100);
    points_text.set_pos(140, 12);
    points_text.renderer = renderer;
    update_points_text();

    time_text.set_color(130, 50, 25);
    time_text.set_pos(15, 50);
    time_text.renderer = renderer;

    level_text.set_color(170, 250, 100);
    level_text.set_pos(280, 12);
    level_text.renderer = renderer;
    update_level_text();

    input_text.set_color(10, 10, 10);
    input_text.set_pos(15, 90);
    input_text.renderer = renderer;
    update_input_text();

    grid_button.set_pos(420, 15);
    grid_button.set_b_color(100, 50, 50);
    grid_button.set_i_color(50, 150, 75);
    grid_button.set_h_color(40, 50, 100);
    grid_button.renderer = renderer;
    grid_button.set_text("Grid", 20);

    editmode_button.set_pos(490, 15);
    editmode_button.set_b_color(100, 50, 50);
    editmode_button.set_i_color(50, 150, 75);
    editmode_button.set_h_color(40, 50, 100);
    editmode_button.renderer = renderer;
    editmode_button.set_text("Edit Mode", 20);

    pause_button.set_pos(620, 15);
    pause_button.set_b_color(100, 50, 50);
    pause_button.set_i_color(50, 150, 75);
    pause_button.set_h_color(40, 50, 100);
    pause_button.renderer = renderer;
    pause_button.set_text("Pause", 20);

    gravity_button.set_pos(710, 15);
    gravity_button.set_b_color(100, 50, 50);
    gravity_button.set_i_color(50, 150, 75);
    gravity_button.set_h_color(40, 50, 100);
    gravity_button.renderer = renderer;
    gravity_button.set_text("Gravity", 20);

    save_button.set_pos(420, 60);
    save_button.set_b_color(100, 50, 50);
    save_button.set_i_color(50, 150, 75);
    save_button.set_h_color(40, 50, 100);
    save_button.renderer = renderer;
    save_button.set_text("Save", 20);

    load_button.set_pos(492, 60);
    load_button.set_b_color(100, 50, 50);
    load_button.set_i_color(50, 150, 75);
    load_button.set_h_color(40, 50, 100);
    load_button.renderer = renderer;
    load_button.set_text("Load", 20);

    restart_button.set_pos(568, 60);
    restart_button.set_b_color(100, 50, 50);
    restart_button.set_i_color(50, 150, 75);
    restart_button.set_h_color(40, 50, 100);
    restart_button.renderer = renderer;
    restart_button.set_text("Restart", 20);
}