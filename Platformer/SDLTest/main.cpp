/**
* Author: Justin Yee
* Assignment: Platformer
* Date due: 2023-11-26, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f


#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 17
#define LEVEL_LEFT_EDGE -3.0f
#define LEVEL_BOTTOM_EDGE -3.0f


#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "Start.h"
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.h"

// ————— CONSTANTS ————— //
constexpr int WINDOW_WIDTH = 640 * 2,
WINDOW_HEIGHT = 480 * 2;

constexpr float BG_RED = 0.0f,
BG_BLUE = 0.0f,
BG_GREEN = 0.0f,
BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char GAME_WINDOW_NAME[] = "FORM!";


int heart_count = 3;
constexpr char HEART_FILEPATH[] = "assets/heart.png", //From https://opengameart.org/content/heart-1616

TEXT_FILEPATH[] = "assets/font1.png";
GLuint text_texture_id;

std::vector<Entity*> hearts;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

constexpr int NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL = 0;
constexpr GLint TEXTURE_BORDER = 0;

// ————— VARIABLES ————— //
Scene* g_current_scene;

LevelA* g_level_a;
LevelB* g_level_b;
LevelC* g_level_c;

Start* g_start;

enum AppStatus { RUNNING, TERMINATED };

SDL_Window* g_display_window;

AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f,
g_accumulator = 0.0f;

void switch_to_scene(Scene* scene)
{
    g_current_scene = scene;
    g_current_scene->initialise();
}

void initialise();
void process_input();
void update();
void render();
void shutdown();

void initialise()
{
    // ————— GENERAL ————— //
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow(GAME_WINDOW_NAME,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    if (context == nullptr)
    {
        LOG("ERROR: Could not create OpenGL context.\n");
        shutdown();
    }

#ifdef _WINDOWS
    glewInit();
#endif

    // ————— VIDEO SETUP ————— //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);


    // ————— Show Start ————— //
    g_start = new Start();
    switch_to_scene(g_start);

    //Load hearts
    text_texture_id = Utility::load_texture(TEXT_FILEPATH);
    GLuint heart_texture_id = Utility::load_texture(HEART_FILEPATH);

    for (int i = 0; i < 3; i++)
    {
        hearts.push_back(new Entity(heart_texture_id, 0.0f, 1, 1, HEART));
		hearts[i]->set_position(glm::vec3(0.0f,0.0f,0.0f));
    }

    // ————— BLENDING ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    if (g_current_scene != g_start)
    {
        g_current_scene->get_state().player->set_movement(glm::vec3(0.0f));
    }
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_app_status = TERMINATED;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
                // Quit the game with a keystroke
                g_app_status = TERMINATED;
                break;

            case SDLK_SPACE:

                if (g_current_scene == g_start)
                {
                    g_level_a = new LevelA();
                    switch_to_scene(g_level_a);
                }

                else if (g_current_scene != g_start)
                {
                    // Jump
                    if (g_current_scene->get_state().player->get_collided_bottom())
                    {
                        g_current_scene->get_state().player->jump();
                        Mix_PlayChannel(-1, g_current_scene->get_state().jump_sfx, 0);
                    }
                }
                break;

            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (g_current_scene != g_start)
    {

        //move left and right, can only dash while on the ground
        if (key_state[SDL_SCANCODE_LEFT])
        {

            //will yell whenever dashing, feel free to remove it if it gets annoying
            if (key_state[SDL_SCANCODE_LSHIFT] && g_current_scene->get_state().player->get_collided_bottom())
            {
                Mix_PlayChannel(-1, g_current_scene->get_state().yell_sfx, 0);
                g_current_scene->get_state().player->run_left();
            }
            else
            {
                g_current_scene->get_state().player->move_left();
            }
        }
        else if (key_state[SDL_SCANCODE_RIGHT])
        {
            if (key_state[SDL_SCANCODE_LSHIFT] && g_current_scene->get_state().player->get_collided_bottom())
            {
                Mix_PlayChannel(-1, g_current_scene->get_state().yell_sfx, 0);
                g_current_scene->get_state().player->run_right();
            }

            else
            {
                g_current_scene->get_state().player->move_right();
            }
        }

        if (glm::length(g_current_scene->get_state().player->get_movement()) > 1.5f)
            g_current_scene->get_state().player->normalise_movement();

    }
}

void update()
{
    if (g_current_scene != g_start && heart_count > 0)
    {


        // ————— DELTA TIME / FIXED TIME STEP CALCULATION ————— //
        float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
        float delta_time = ticks - g_previous_ticks;
        g_previous_ticks = ticks;

        delta_time += g_accumulator;

        if (delta_time < FIXED_TIMESTEP)
        {
            g_accumulator = delta_time;
            return;
        }

        while (delta_time >= FIXED_TIMESTEP) {
            // ————— UPDATING THE SCENE (i.e. map, character, enemies...) ————— //
            g_current_scene->update(FIXED_TIMESTEP);
            if (heart_count > 0 && !g_current_scene->get_state().player->get_is_active())
            {
                g_current_scene->respawn();
                heart_count--;
            }

            if (g_current_scene == g_level_a && g_current_scene->get_state().player->get_goal())
            {
                g_current_scene->get_state().player->change_goal();
                g_level_b = new LevelB();
                switch_to_scene(g_level_b);
            }

            if (g_current_scene == g_level_b && g_current_scene->get_state().player->get_goal())
            {
                g_current_scene->get_state().player->change_goal();
                g_level_c = new LevelC();
                switch_to_scene(g_level_c);
            }



            delta_time -= FIXED_TIMESTEP;
        }

        g_accumulator = delta_time;


        for (int i = 0; i < heart_count; i++)
        {
            hearts[i]->set_position(glm::vec3(g_current_scene->get_state().player->get_position().x - 4.5 + i, g_current_scene->get_state().player->get_position().y + 3, 0));
            hearts[i]->update(0, NULL, NULL, 0, g_current_scene->get_state().map);
        }

        // ————— PLAYER CAMERA ————— //
        g_view_matrix = glm::mat4(1.0f);
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->get_state().player->get_position().x, -g_current_scene->get_state().player->get_position().y, 0));
    }
}

void render()
{
    g_shader_program.set_view_matrix(g_view_matrix);

    glClear(GL_COLOR_BUFFER_BIT);
    g_current_scene->render(&g_shader_program);


    if (heart_count == 0)
    {
        Utility::draw_text(&g_shader_program, text_texture_id, "You Lose :(",
            .75, .01, glm::vec3(g_current_scene->get_state().player->get_position().x - 3, g_current_scene->get_state().player->get_position().y + 1, 0.0f));
    }

    if (g_current_scene == g_level_c && g_current_scene->get_state().player->get_goal())
    {
        g_current_scene->get_state().player->set_position(glm::vec3(-100, -100, 0));
        Utility::draw_text(&g_shader_program, text_texture_id, "You WIN!!!!!!!",
            .75, .01, glm::vec3(g_current_scene->get_state().player->get_position().x - 3, g_current_scene->get_state().player->get_position().y + 1, 0.0f));
    }

    if (g_current_scene != g_start)
    {
        for (int i = 0; i < heart_count; i++)
        {
            hearts[i]->render(&g_shader_program);
        }
    }
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();

	delete g_level_a;
    delete g_level_b;
    delete g_level_c;
    delete g_start;
	for (int i = 0; i < 3; i++)
	{
		delete hearts[i];
	}   
}

// ————— GAME LOOP ————— //
int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
