#pragma once

#include <vector>
#include "block.hpp"
#include "main/scene_base/base.hpp"
#include "gui_scene_structure.hpp"

class Grid
{
public:
    size_t Nx_chunks = 4; // TODO
    size_t Ny_chunks = 4;
    size_t N_dim_chunk = 16;
    size_t Nx = 128; // Number of blocks in x
    size_t Ny = 128; // Number of blocks in y
    size_t Nz = 100; // Number of blocks in z
    size_t Nz_dungeon = 30; // Number of blocks in z
    int Nz_surface = 32; // Number of blocks in z
    float step = 1 / (float) Nx; // Minimum step (Divide by the biggest N)

    size_t N; // Number of blocks N x N x N
    std::vector<std::vector<std::vector<int>>> blocks; // 3D array that contains the type of block in the place
    std::vector<std::vector<std::vector<bool>>> draw_blocks; // 3D array
    std::vector<std::vector<int>> surface_z; // 3D array
    std::map<int,std::vector<vcl::vec3>> translations;

    // chunks -> here for futher optimisation
    // std::vector<std::map<vcl::vec3, std::vector<int>>> chunks;
    
    // Seeding:
    std::default_random_engine gen;

    void create_grid(gui_scene_structure gui, std::default_random_engine g);
    void generate_surface(gui_scene_structure gui);
    void generate_dungeons(gui_scene_structure gui);
    void generate_trees(gui_scene_structure gui);
    void generate_river(gui_scene_structure gui);
    void feed_translations();

    vcl::mesh_drawable block;
    vcl::mesh_drawable block_billboard;

    GLuint* block_textures;

    void setup();
    void frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, bool wireframe);

    int position_to_block(vcl::vec3 p);
    vcl::vec3 blocks_to_position(int x, int y, int z);

    void delete_block(vcl::vec3 p);
};