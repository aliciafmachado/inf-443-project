#pragma once

#include "main/scene_base/base.hpp"
#include "grid.hpp"

class Player
{
public:
    void setup(float scale, std::map<std::string,GLuint>& shaders);
    void frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_scene_structure gui_scene, Grid g);

    void key_press(scene_structure& scene, GLFWwindow* window, int key, int scancode, int action, int mods);
    void updatePosition(scene_structure& scene);
    void move(float dx, float dy);
    void turn(float d_angle);

    float angle;
    float camera_angle;
    float x;
    float y;

    bool moving_up;
    bool moving_down;
    bool moving_left;
    bool moving_right;
    bool moving;

    GLuint player_texture;

    vcl::timer_interval timer;
    vcl::hierarchy_mesh_drawable hierarchy;
    vcl::hierarchy_mesh_drawable_display_skeleton hierarchy_visual_debug;

    void draw_player(float scale);
};