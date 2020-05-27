#pragma once

#include "main/scene_base/base.hpp"
#include "gui_scene_structure.hpp"

class Grid
{
public:
    size_t Nx = 70; // Number of blocks in x
    size_t Ny = 50; // Number of blocks in y
    size_t Nz = 60; // Number of blocks in z
    size_t Nz_dungeon = 30; // Number of blocks in z
    int Nz_surface = 32; // Number of blocks in z
    float step = 1 / (float) Nx; // Minimum step (Divide by the biggest N)

    size_t N; // Number of blocks N x N x N
    std::vector<std::vector<std::vector<int>>> blocks; // 3D array that contains the type of block in the place
    std::vector<std::vector<std::vector<bool>>> draw_blocks; // 3D array
    std::vector<std::vector<int>> surface_z; // 3D array

    void create_grid(gui_scene_structure gui);
    void generate_surface(gui_scene_structure gui);
    void generate_dungeons(gui_scene_structure gui);
    void generate_trees(gui_scene_structure gui);
    void generate_river(gui_scene_structure gui);

    vcl::mesh_drawable block;

    GLuint block_texture_grass;
    GLuint block_texture_dirty;
    GLuint block_texture_water;
    GLuint block_texture_wood;
    GLuint block_texture_stone;
    GLuint block_texture_leave;

    void setup();
    void frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, bool wireframe);

    int position_to_block(vcl::vec3 p);
    vcl::vec3 blocks_to_position(int x, int y, int z);
};