/* Header file of the grid
 */

#pragma once

#include <algorithm>
#include <utility>
#include <vector>
#include "block.hpp"
#include "main/scene_base/base.hpp"
#include "gui_scene_structure.hpp"

class Grid
{
public:
    size_t Nx = 180; // Number of blocks in x
    size_t Ny = 180; // Number of blocks in y
    size_t Nz = 1000; // Number of blocks in z
    size_t Nz_dungeon = 50; // Number of blocks in z
    int Nz_surface = 51; // Number of blocks in z
    float step = 1 / (float) Nx; // Minimum step (Divide by the biggest N)

    size_t N; // Number of blocks N x N x N
    std::vector<std::vector<std::vector<int>>> blocks; // 3D array that contains the type of block in the place
    std::vector<std::vector<std::vector<bool>>> draw_blocks; // 3D array
    std::vector<std::vector<int>> surface_z; // 3D array
    std::map<int,std::vector<vcl::vec3>> translations;
    
    // Seeding:
    std::default_random_engine gen;

    void create_grid(gui_scene_structure gui, std::default_random_engine g);
    void generate_surface(gui_scene_structure gui);
    void generate_dungeons(gui_scene_structure gui);
    void generate_trees(gui_scene_structure gui);
    void generate_lake(gui_scene_structure gui);
    void create_enter_dungeon(gui_scene_structure gui);
    void feed_translations();
    bool near_block(float x, float y, float z, int block_type, int dist, bool only_surface);

    vcl::mesh_drawable block;
    vcl::mesh_drawable block_simple;

    GLuint* block_textures;

    void setup();
    void frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, bool wireframe, int fps);

    int position_to_block(vcl::vec3 p);
    vcl::vec3 blocks_to_position(int x, int y, int z);

    void delete_block(vcl::vec3 p);

    void join_surface_dungeon();
};