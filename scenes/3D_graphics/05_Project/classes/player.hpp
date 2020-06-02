#pragma once

#include "main/scene_base/base.hpp"
#include "grid.hpp"

class Player
{
public:

    // Character dimension
    float size = 0.5f;
    float head_x = 0.75f;
    float head_y = 1.0f;
    float head_z = 1.0f;
    float body_y = 1.0f;
    float body_z = 1.5f;
    float leg_y = 0.5f;
    float leg_z = 1.5f;
    float arm_y = 0.5f;
    float arm_z =  1.5f;
    float dist_feet;
    float dist_head;

    void setup(float scale, std::map<std::string,GLuint>& shaders, Grid* g_);
    void frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_scene_structure gui_scene, int fps);
    int fps;

    void keyboard_input(scene_structure& scene, GLFWwindow* window, int key, int scancode, int action, int mods);
    void updatePosition(scene_structure& scene);
    void updateCamera(scene_structure &structure);
    void move(float speed);
    void turn(float speed);
    void jump(float initial_speed);
    void fall(float mass);

    Grid* g;

    // Position in the grid
    int x;
    int y;
    int z;
    vcl::vec3 p;

    float angle;
    float camera_angle;

    float error;
    float scale;
    float distance;

    bool jumping;
    bool falling;

    bool moving_up = false;
    bool moving_down = false;
    bool moving_left = false;
    bool moving_right = false;

    bool moving_camera_up = false;
    bool moving_camera_down = false;
    bool moving_camera_left = false;
    bool moving_camera_right = false;

    bool moving = false;
    vcl::vec3 v;

    GLuint player_texture;

    vcl::timer_interval timer;
    vcl::hierarchy_mesh_drawable hierarchy;
    vcl::hierarchy_mesh_drawable_display_skeleton hierarchy_visual_debug;

    void create_player();

    bool check_ahead();
    bool check_behind();
    bool check_down();
    bool check_up();

    // Called every time the mouse is clicked
    void mouse_click(scene_structure& scene, GLFWwindow* window, int button, int action, int mods);
    // Called every time the mouse is moved
    void mouse_move(scene_structure& scene, GLFWwindow* window);
};