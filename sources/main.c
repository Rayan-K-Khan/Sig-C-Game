#include "raylib.h"
#include "easings.h"

#define SCREEN_WIDTH (1000)
#define SCREEN_HEIGHT (800)
#define PLAYER_SPEED (5)

#define WINDOW_TITLE "Window title"

typedef struct Laser Laser;
struct Laser
{
    int x;
    int y;
    bool active;
    bool enemy;
};

#define MAX_LASER_COUNT 999
Laser lasers[MAX_LASER_COUNT];


typedef struct Enemy Enemy;
struct Enemy
{
    int x;
    int y;
    bool alive;

    int start_x;
    int start_y;
    int end_x;
    int end_y;
    float movement_time;
    float time_until_laser;
};

#define ENEMY_MOVEMENT_TIME (1.0)

#define MAX_ENEMY_COUNT 10

Enemy enemies[MAX_ENEMY_COUNT];

int player_x = 0;
int player_y = 0;
int score = 0;
int player_lives = 3;
bool player_damaged;
int player_damaged_flashes_left = 3;
float player_damaged_animation_time = 0;
#define PLAYER_DAMAGED_ANIMATION_DURATION (0.15)

Texture2D player_texture;
Texture2D enemy_texture;
Texture2D laser_texture;
Texture2D background_texture;
Texture2D enemy_laser_texture;

void start_game()
{
    score = 0;
    player_lives = 3;

    player_x = SCREEN_WIDTH / 2 - player_texture.width/2;
    player_y = SCREEN_HEIGHT*3/4 - player_texture.height/2;

    for (int i = 0; i < MAX_ENEMY_COUNT; i++)
    {
        enemies[i].alive = true;
        enemies[i].x = enemies[i].start_x = GetRandomValue(0, SCREEN_WIDTH - enemy_texture.width);
        enemies[i].y = enemies[i].start_y = GetRandomValue(0, SCREEN_HEIGHT/2 - enemy_texture.height);
        enemies[i].end_x = GetRandomValue(0, SCREEN_WIDTH - enemy_texture.width);
        enemies[i].end_y = GetRandomValue(0, SCREEN_HEIGHT / 2 - enemy_texture.height);
        enemies[i].time_until_laser = GetRandomValue(1, 3);
    }
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);

    player_texture = LoadTexture(ASSETS_PATH"spaceship.png"); // Check README.md for how this works
    enemy_texture = LoadTexture(ASSETS_PATH"enemy.png");
    laser_texture = LoadTexture(ASSETS_PATH"player-laser.png");
    background_texture = LoadTexture(ASSETS_PATH"background.png");
    enemy_laser_texture = LoadTexture(ASSETS_PATH"evil-laser.png");
    start_game();

    while (!WindowShouldClose())
    {
        bool enemy_alive = false;
        for (int i = 0; i < MAX_ENEMY_COUNT; i++)
        {
            if (enemies[i].alive)
            {
                enemy_alive = true;
                break;
            }
        }
        //Updates
        {
            if (IsKeyDown(KEY_A)) player_x -= PLAYER_SPEED;
            if (IsKeyDown(KEY_D)) player_x += PLAYER_SPEED;
            if (IsKeyDown(KEY_W)) player_y -= PLAYER_SPEED;
            if (IsKeyDown(KEY_S)) player_y += PLAYER_SPEED;

            if (player_x < 0) player_x = 0;
            if (player_y < SCREEN_HEIGHT / 2) player_y = SCREEN_HEIGHT / 2;
            if (player_x > (SCREEN_WIDTH-player_texture.width)) player_x = (SCREEN_WIDTH - player_texture.width);
            if (player_y > (SCREEN_HEIGHT-player_texture.height)) player_y = (SCREEN_HEIGHT - player_texture.height);

            bool can_start_game = (player_lives <= 0) || !enemy_alive;

            if (IsKeyPressed(KEY_ENTER) && can_start_game)
            {
                start_game();
            }

            //Shooting Lasers
            if (player_lives > 0 && IsKeyPressed(KEY_SPACE))
            {
                for (int i = 0; i < MAX_LASER_COUNT; i++)
                {
                    if (!lasers[i].active)
                    {
                        lasers[i].active = true;
                        lasers[i].enemy = false;
                        lasers[i].x = player_x + player_texture.width/2;
                        lasers[i].y = player_y;
                        break;
                    }
                }
            }

            //Move Lasers
            for (int i = 0; i < MAX_LASER_COUNT; i++)
            {
                if (lasers[i].active)
                {

                    if (lasers[i].enemy)
                    {
                        lasers[i].y += 10;
                    }

                    else
                    {
                        lasers[i].y -= 10;
                    }


                    //Remove Lasers off screen
                    if (lasers[i].y < -laser_texture.height || lasers[i].y > SCREEN_HEIGHT)
                    {
                        lasers[i].active = false;
                    }

                    if (lasers[i].enemy)
                    {
                        //Check collision with players
                        Rectangle laser_rectangle = {
                                    .x = lasers[i].x,
                                    .y = lasers[i].y,
                                    .width = laser_texture.width,
                                    .height = laser_texture.height

                        };

                        Rectangle player_rectangle = {
                            .x = player_x,
                            .y = player_y,
                            .width = enemy_texture.width,
                            .height = enemy_texture.height
                        };

                        if (player_lives>0 && CheckCollisionRecs(laser_rectangle, player_rectangle))
                        {
                            player_damaged = true;
                            player_lives -= 1;
                            lasers[i].active = false;
                            score -= 10;
                        }

                    }

                    else
                    {

                        for (int j = 0; j < MAX_ENEMY_COUNT; j++)
                        {
                            if (enemies[j].alive)
                            {
                                Rectangle laser_rectangle = {
                                    .x = lasers[i].x,
                                    .y = lasers[i].y,
                                    .width = laser_texture.width,
                                    .height = laser_texture.height

                                };

                                Rectangle enemy_rectangle = {
                                    .x = enemies[j].x,
                                    .y = enemies[j].y,
                                    .width = enemy_texture.width,
                                    .height = enemy_texture.height

                                };

                                if (CheckCollisionRecs(laser_rectangle, enemy_rectangle))
                                {
                                    enemies[j].alive = false;
                                    lasers[i].active = false;
                                    score += 100;
                                }
                            }
                        }
                    }
                }
            }

            //Move Enemies
            for (int i = 0; i < MAX_ENEMY_COUNT; i++)
            {
                if (enemies[i].alive)
                {
                    if (enemies[i].time_until_laser <= 0)
                    {
                        for (int j = 0; j < MAX_LASER_COUNT; j++)
                        {
                            if (!lasers[j].active)
                            {
                                lasers[j].active = true;
                                lasers[j].enemy = true;
                                lasers[j].x= enemies[i].x + enemy_texture.width / 2;
                                lasers[j].y = enemies[i].y;
                                break;
                            }
                        }

                        enemies[i].time_until_laser = GetRandomValue(1, 3);
                    }

                    enemies[i].time_until_laser -= GetFrameTime();
                    float x1 = EaseQuadInOut(enemies[i].movement_time, enemies[i].start_x, enemies[i].end_x - enemies[i].start_x, ENEMY_MOVEMENT_TIME);
                    float y1 = EaseQuadInOut(enemies[i].movement_time, enemies[i].start_y, enemies[i].end_y - enemies[i].start_y, ENEMY_MOVEMENT_TIME);
                    enemies[i].x = x1;
                    enemies[i].y = y1;

                    enemies[i].movement_time += GetFrameTime();

                    if (enemies[i].movement_time >= ENEMY_MOVEMENT_TIME)
                    {
                        enemies[i].start_x = enemies[i].x;
                        enemies[i].start_y = enemies[i].y;
                        enemies[i].end_x = GetRandomValue(0, SCREEN_WIDTH - enemy_texture.width);
                        enemies[i].end_y = GetRandomValue(0, SCREEN_HEIGHT / 2 - enemy_texture.height);
                        enemies[i].movement_time = 0;
                    }
                }
            }
        }

        

        //Rendering
        {
            BeginDrawing();

            ClearBackground(RAYWHITE);

            Vector2 background_pos = {
                .x = 0,
                .y = 0
            };

            DrawTextureEx(background_texture, background_pos, 0, 4, WHITE);
            //Render player
            if (player_lives > 0)
            {
                int alpha_begin = 0;
                int alpha_end = 255;
                int alpha = EaseQuadInOut(player_damaged_animation_time, alpha_begin, alpha_end-alpha_begin, PLAYER_DAMAGED_ANIMATION_DURATION);

                if (player_damaged_animation_time >= PLAYER_DAMAGED_ANIMATION_DURATION)
                {
                    if (player_damaged_flashes_left > 0)
                    {
                        player_damaged_animation_time = 0;
                        player_damaged_flashes_left -= 1;
                    }

                    else
                    {
                        player_damaged_animation_time = 0;
                        player_damaged = false;
                        player_damaged_flashes_left = 3;
                    }
                }
                
                if (player_damaged)
                {
                    player_damaged_animation_time += GetFrameTime();
                }

                DrawTexture(player_texture, player_x, player_y, WHITE);
                DrawTexture(player_texture, player_x, player_y, (Color) { 255, 0, 0, alpha });
            }

            if (!enemy_alive)
            {
                Vector2 text_dimensions = MeasureTextEx(GetFontDefault(), "YOU WIN!", 30, 2);
                Vector2 screen_position = (Vector2){
                    .x = (SCREEN_WIDTH / 2) - (text_dimensions.x / 2),
                    .y = (SCREEN_HEIGHT/2) - (text_dimensions.y/2)
                };

                DrawText("YOU WIN!", screen_position.x, screen_position.y, 30, WHITE);
            }

            //Render score
            {
                const char* score_str = TextFormat("Score: %i", score);
                DrawText(score_str, 5, 5, 30, WHITE);

                const char* lives_str = TextFormat("Lives: %i", player_lives);
                int lives_str_width = MeasureText(lives_str, 30);
                DrawText(lives_str, SCREEN_WIDTH-lives_str_width-5, 5, 30, WHITE);

            }

            //Render Enemies
            for (int i = 0; i < MAX_ENEMY_COUNT; i++)
            {
                if (enemies[i].alive)
                {
                    DrawTexture(enemy_texture, enemies[i].x, enemies[i].y, WHITE);
                }
            }

            //Render Lasers
            for (int i = 0; i < MAX_LASER_COUNT; i++)
            {
                if (lasers[i].active)
                {
                    if (lasers[i].enemy)
                    {
                        DrawTexture(enemy_laser_texture, lasers[i].x, lasers[i].y, WHITE);
                    }
                    else
                    {
                        DrawTexture(laser_texture, lasers[i].x, lasers[i].y, WHITE);
                    }
                }
            }

            if (player_lives <= 0)
            {
                Vector2 text_dimensions = MeasureTextEx(GetFontDefault(), "YOU LOSE!", 30, 2);
                Vector2 screen_position = (Vector2){
                    .x = (SCREEN_WIDTH / 2) - (text_dimensions.x / 2),
                    .y = (SCREEN_HEIGHT / 2) - (text_dimensions.y / 2)
                };

                DrawText("YOU LOSE!", screen_position.x, screen_position.y, 30, WHITE);
            }

            EndDrawing();
        }
    }

    CloseWindow();

    return 0;
}
