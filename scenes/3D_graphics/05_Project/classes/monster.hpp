#pragma once

#include "main/scene_base/base.hpp"
#include "grid.hpp"
#include "player.hpp"

class Monster
{
public:

    // Character dimension
    float head_x = 0.30f;
    float head_y = 0.30f;
    float head_z = 0.30f;
    float body_x = 0.15f;
    float body_y = 0.30f;
    float body_z = 0.47f;
    float leg_x = 0.15f;
    float leg_y = 0.30f;
    float leg_z = 0.23f;
    float dist_feet;
    float dist_head;

    void setup(float scale, std::map<std::string,GLuint>& shaders, Grid* g_, Player* player_);
    void frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_scene_structure gui_scene, int fps);

    int fps;

    void updatePosition(scene_structure& scene);
    void move(float speed, scene_structure &scene);
    void turn(float speed);
    void jump(float initial_speed);
    void fall(float mass);

    std::default_random_engine generator;

    Grid* g;
    Player* player;

    // Position in the grid
    int x;
    int y;
    int z;
    vcl::vec3 p;

    float angle;

    float error;
    float scale;
    float distance;

    bool jumping;
    bool falling;

    bool moving;
    bool turning;
    bool out;

    float angle_turn;
    float f_angle;
    float t_mov;
    float f_t_mov;
    float l_angle;
    float c_t;
    bool find_player;

    vcl::vec3 v;

    GLuint monster_texture;

    vcl::timer_interval timer;
    vcl::hierarchy_mesh_drawable hierarchy;
    vcl::hierarchy_mesh_drawable_display_skeleton hierarchy_visual_debug;

    void create_monster();

    bool check_ahead(scene_structure &scene);
    bool check_down(scene_structure &scene);
    bool check_up();

    float min_d;
    void close_player();
};