#include "Start.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 17

constexpr char SPRITESHEET_FILEPATH[] = "assets/mc.png", //From https://opengameart.org/content/anime-collection
MAP_TILESET_FILEPATH[] = "assets/map.png", //From https://opengameart.org/content/tile-map-dark-2d-platformer
TEXT_FILEPATH[] = "assets/font1.png",

BGM_FILEPATH[] = "assets/SCP-x3x.mp3", //From Incompetech
JUMP_SFX_FILEPATH[] = "assets/jump.wav", //From https://opengameart.org/content/jump-landing-sound
YELL_SFX_FILEPATH[] = "assets/yell.wav"; //From https://opengameart.org/content/battlecry


Start::~Start()
{
    Mix_FreeMusic(m_game_state.bgm);
}

void Start::initialise()
{
    text_texture_id = Utility::load_texture(TEXT_FILEPATH);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_game_state.bgm = Mix_LoadMUS(BGM_FILEPATH);
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2.0f);
}

void Start::update(float delta_time)
{
}


void Start::render(ShaderProgram* g_shader_program)
{
    //draws "menu"
        Utility::draw_text(g_shader_program, text_texture_id, "Press Space!",
            .65, .01, glm::vec3(-3.5,0,0));
}


void Start::respawn() {}
