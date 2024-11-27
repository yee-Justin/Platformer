#include "LevelA.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 17

constexpr char SPRITESHEET_FILEPATH[] = "assets/mc.png", //From https://opengameart.org/content/anime-collection
MAP_TILESET_FILEPATH[] = "assets/map.png", //From https://opengameart.org/content/tile-map-dark-2d-platformer

ROCK_FILEPATH[] = "assets/rock.png", //From https://opengameart.org/content/rock-0
GHOST_FILEPATH[] = "assets/ghost.png", //From https://opengameart.org/content/stendhal-ghost
STAR_FILEPATH[] = "assets/star.png", //From https://opengameart.org/content/star

BGM_FILEPATH[] = "assets/builder.mp3", //From Incompetech, "The Builder"

JUMP_SFX_FILEPATH[] = "assets/jump.wav", //From https://opengameart.org/content/jump-landing-sound
YELL_SFX_FILEPATH[] = "assets/yell.wav"; //From https://opengameart.org/content/battlecry


int ghost_num_A = 5;
std::vector<glm::vec3> ghost_positions =
{
    glm::vec3(0.0f, -1.0f, 0.0f),
    glm::vec3(13.0f, -5.0f, 0.0f),
    glm::vec3(0.0f, -4.0f, 0.0f),
    glm::vec3(8.0f, -7.0f, 0.0f),
    glm::vec3(4.0f, -13.0f, 0.0f)
};

unsigned int LEVEL_A_DATA[] =
{
    36, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 49,
    55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 55,
    55, 10, 10, 10, 10, 10, 10, 10, 10, 12, 0, 0, 0, 55,
    55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 55,
    55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 55,

    55, 0, 5, 10, 12, 0, 0, 0, 5, 10, 10, 10, 10, 55,
    55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 55,
    55, 0, 0, 0, 5, 10, 10, 10, 10, 12, 0, 0, 0, 55,
    55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 55,
    55, 0, 0, 5, 10, 12, 0, 0, 0, 0, 0, 0, 0, 55,

    55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 55,
    55, 5, 10, 10, 10, 10, 10, 10, 10, 10, 12, 0, 0, 55,
    55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 55,
    55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 55,
    55, 0, 5, 10, 10, 10, 10, 10, 10, 10, 10, 12, 0, 55,
    55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 55,

    18, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 87

};

LevelA::~LevelA()
{
    delete    m_game_state.player;
    delete m_game_state.rock;

    for (int i = 0; i < ghost_num_A; i++)
    {
        delete m_game_state.enemies[i];
    }
    
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeChunk(m_game_state.yell_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelA::initialise()
{
    // ————— MAP SET-UP ————— //
    GLuint map_texture_id = Utility::load_texture(MAP_TILESET_FILEPATH);
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVEL_A_DATA, map_texture_id, 1.0f, 6, 14);

    // ————— GEORGE SET-UP ————— //

    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    GLuint rock_texture_id = Utility::load_texture(ROCK_FILEPATH);
	GLuint ghost_texture_id = Utility::load_texture(GHOST_FILEPATH);
	GLuint star_texture_id = Utility::load_texture(STAR_FILEPATH);

    glm::vec3 acceleration = glm::vec3(0.0f, -4.15f, 0.0f);

    std::vector<std::vector <int> > player_walking_animation =
    {
        { 0, 1, 2, 3 },  // for mc to move to the left,
        { 4, 5, 6, 7 }, // for mc to move to the right,
        { 8, 9, 10, 11 }, // for mc to run left,
        { 12, 13, 14, 15 },   // for mc to run right
        { 16, 17, 18, 19 },   // for mc to attack left
        { 20, 21, 22, 23 }   // for mc to attack right
    };

    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        5.0f,                      // speed
        acceleration,              // acceleration
        .45f,                      // jumping power
        player_walking_animation,  // animation index sets
        0.0f,                      // animation time
        4,                         // animation frame amount
        0,                         // current animation index
        4,                         // animation column amount
        6,                         // animation row amount
        0.4f,                      // width
        0.8f,                       // height
        PLAYER
    );
    m_game_state.player->set_position(glm::vec3(7.0f, -2.0f, 0.0f));
    m_game_state.player->update(0, NULL, NULL, 0, m_game_state.map);


    m_game_state.rock = new Entity(rock_texture_id, 1.0f, .95f, .90f, ENEMY, ROCK, IDLE);

    m_game_state.rock->set_position(glm::vec3(6.0f, -3.0f, 0.0f));

    //No acceleration because I want it to "stick" to the bottom of the tile
    m_game_state.rock->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    m_game_state.rock->set_scale(glm::vec3(2.0f, 2.0f, 1.0f));
    m_game_state.rock->update(0, NULL, NULL, 0, m_game_state.map);

	for (int i = 0; i < ghost_num_A; i++)
	{
		m_game_state.enemies.push_back(new Entity(ghost_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, GHOST, IDLE));
		m_game_state.enemies[i]->set_position(ghost_positions[i]);
		m_game_state.enemies[i]->set_acceleration(glm::vec3(0.0f,0.0f,0.0f));
		m_game_state.enemies[i]->set_scale(glm::vec3(2.0f, 2.0f, 1.0f));

		m_game_state.enemies[i]->update(0, NULL, NULL, 0, m_game_state.map);
	}

    m_game_state.star = new Entity(star_texture_id, 1.0f, .95f, .90f, GOAL);
    m_game_state.star->set_position(glm::vec3(4.0f, -13.0f, 0.0f));
    m_game_state.star->update(0, NULL, NULL, 0, m_game_state.map);


    // Jumping
    m_game_state.player->set_jumping_power(5.0f);

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_game_state.bgm = Mix_LoadMUS(BGM_FILEPATH);
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 8.0f);
    m_game_state.jump_sfx = Mix_LoadWAV(JUMP_SFX_FILEPATH);
    m_game_state.yell_sfx = Mix_LoadWAV(YELL_SFX_FILEPATH);

    Mix_VolumeChunk(m_game_state.yell_sfx, MIX_MAX_VOLUME / 16.0f);
}

void LevelA::update(float delta_time)
{


    if (m_game_state.player->get_is_active())
    {
        m_game_state.player->update(delta_time, m_game_state.player, NULL, 0,
            m_game_state.map);
        m_game_state.rock->update(delta_time, m_game_state.player, NULL, 0,
            m_game_state.map);


        m_game_state.star->update(delta_time, NULL, NULL, 0, m_game_state.map);

        for (int i = 0; i < ghost_num_A; i++)
        {
            m_game_state.enemies[i]->update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);
        }


        m_game_state.player->check_collision_x(m_game_state.rock, 1);
        m_game_state.player->check_collision_y(m_game_state.rock, 1);

        for (int i = 0; i < ghost_num_A; i++)
        {
            m_game_state.player->check_collision_x(m_game_state.enemies[i], 1);
            m_game_state.player->check_collision_y(m_game_state.enemies[i], 1);
        }


        m_game_state.player->check_collision_x(m_game_state.star, 1);
        m_game_state.player->check_collision_y(m_game_state.star, 1);


    }
}


void LevelA::render(ShaderProgram* g_shader_program)
{
    
    //draws collidable entities if they are active
    if (m_game_state.player->get_is_active())
    {
        m_game_state.player->render(g_shader_program);
    }

    m_game_state.rock->render(g_shader_program);

	for (int i = 0; i < ghost_num_A; i++)
	{
        m_game_state.enemies[i]->render(g_shader_program);
	}

	m_game_state.star->render(g_shader_program);

    m_game_state.map->render(g_shader_program);
}

void LevelA::respawn()
{
    m_game_state.player->activate();
    m_game_state.player->set_position(glm::vec3(7.0f, -2.0f, 0.0f));

    m_game_state.rock->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    m_game_state.rock->set_position(glm::vec3(6.0f, -3.0f, 0.0f));
    m_game_state.rock->update(0, NULL, NULL, 0, m_game_state.map);


    for (int i = 0; i < ghost_num_A; i++)
    {
        m_game_state.enemies[i]->set_position(ghost_positions[i]);
        m_game_state.enemies[i]->set_ai_state(IDLE);
        m_game_state.enemies[i]->set_movement(glm::vec3(0.0f, 0.0f, 0.0f));
        m_game_state.enemies[i]->update(0, NULL, NULL, 0, m_game_state.map);
    }

    m_game_state.rock->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    m_game_state.rock->set_position(glm::vec3(6.0f, -3.0f, 0.0f));
    m_game_state.rock->update(0, NULL, NULL, 0, m_game_state.map);
}