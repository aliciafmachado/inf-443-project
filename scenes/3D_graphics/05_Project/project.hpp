#pragma once

#include "main/scene_base/base.hpp"

#ifdef SCENE_3D_PROJECT


// GUI parameters
struct gui_scene_structure
{
    bool generate_surface = true;
    bool generate_dungeons = true;
    bool generate_trees = true;
    bool generate_river = false;

    float height = 1.0f;
    float scaling = 3.0f;
    int octave = 5;
    float persistency = 0.4f;
    float frequency = 2.0f;
    float min_noise = 0.65f;
};

struct grid
{
    size_t N; // Number of blocks N x N x N
    std::vector<std::vector<std::vector<int>>> blocks; // 3D array that contains the type of block in the place
    std::vector<std::vector<std::vector<bool>>> draw_blocks; // 3D array
    std::vector<std::vector<int>> surface_z; // 3D array

    void create_grid(gui_scene_structure gui);
    void generate_surface(gui_scene_structure gui);
    void generate_dungeons(gui_scene_structure gui);
    void generate_trees(gui_scene_structure gui);
    void generate_river(gui_scene_structure gui);
};

struct scene_model : scene_base
{

    /** A part must define two functions that are called from the main function:
     * setup_data: called once to setup data before starting the animation loop
     * frame_draw: called at every displayed frame within the animation loop
     *
     * These two functions receive the following parameters
     * - shaders: A set of shaders.
     * - scene: Contains general common object to define the 3D scene. Contains in particular the camera.
     * - data: The part-specific data structure defined previously
     * - gui: The GUI structure allowing to create/display buttons to interact with the scene.
    */

    void setup_data(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_structure& gui);
    void frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_structure& gui);

    vcl::mesh_drawable sphere;
    vcl::mesh_drawable block;

    GLuint block_texture_grass;
    GLuint block_texture_dirty;
    GLuint block_texture_water;
    GLuint block_texture_wood;
    GLuint block_texture_stone;
    GLuint block_texture_leave;

    gui_scene_structure gui_scene;
    void set_gui();

    grid g;

    void generate_surface(gui_structure &structure);
};

#endif