#include "grid.hpp"

using namespace vcl;

#define NONE 0
#define GRASS 1
#define DIRTY 2
#define STONE 3
#define WATER 4
#define LEAVE 5
#define WOOD 6

float evaluate_terrain_z(float u, float v, const gui_scene_structure& gui_scene);
mesh create_block(float l);

void Grid::setup()
{
    block = create_block(step / 2);
    block_texture_grass = create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/grass.png"),
                                             GL_REPEAT, GL_REPEAT );
    block_texture_dirty = create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/dirty.png"),
                                             GL_REPEAT, GL_REPEAT );
    block_texture_stone = create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/rock.png"),
                                             GL_REPEAT, GL_REPEAT );
    block_texture_water = create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/water.png"),
                                             GL_REPEAT, GL_REPEAT );
    block_texture_leave = create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/leave.png"),
                                             GL_REPEAT, GL_REPEAT );
    block_texture_wood =  create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/wood.png"),
                                             GL_REPEAT, GL_REPEAT );
    // TODO : Merge setup and create_grid
}

void Grid::frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, bool wireframe)
{
    for (int k=0; k<Nz; ++k){
        for (int j=0; j<Ny; ++j){
            for (int i=0; i<Nx; ++i){
                if (blocks[k][j][i] != 0 and draw_blocks[k][j][i]) {
                    block.uniform.transform.translation   = {i * step, j * step, k * step};
                    if (blocks[k][j][i] == 1)
                        glBindTexture(GL_TEXTURE_2D, block_texture_grass);
                    if (blocks[k][j][i] == 2)
                        glBindTexture(GL_TEXTURE_2D, block_texture_dirty);
                    if (blocks[k][j][i] == 3)
                        glBindTexture(GL_TEXTURE_2D, block_texture_stone);
                    if (blocks[k][j][i] == 4)
                        glBindTexture(GL_TEXTURE_2D, block_texture_water);
                    if (blocks[k][j][i] == 5)
                        glBindTexture(GL_TEXTURE_2D, block_texture_leave);
                    if (blocks[k][j][i] == 6)
                        glBindTexture(GL_TEXTURE_2D, block_texture_wood);
                    draw(block, scene.camera, shaders["mesh"]);
                    if (wireframe)
                        draw(block, scene.camera, shaders["wireframe"]);
                    glBindTexture(GL_TEXTURE_2D, scene.texture_white);
                }
            }
        }
    }
}
void Grid::create_grid(gui_scene_structure gui)
{
    // Inialize vector with blocks type 1
    blocks = std::vector<std::vector<std::vector<int>>>(Nz, std::vector<std::vector<int>>(Ny, std::vector<int>(Nx, 0)));
    draw_blocks = std::vector<std::vector<std::vector<bool>>>(Nz, std::vector<std::vector<bool>>(Ny, std::vector<bool>(Nx, false)));
    surface_z = std::vector<std::vector<int>>(Ny, std::vector<int>(Nx, Nz_surface));

    for (int k=0; k<Nz; ++k){
        for (int j=0; j<Ny; ++j){
            for (int i=0; i<Nx; ++i){
                if (k <= Nz_dungeon)
                    blocks[k][j][i] = 3;
                if (k > Nz_dungeon && k < Nz_surface)
                    blocks[k][j][i] = 2;
                if (k == Nz_surface)
                    blocks[k][j][i] = 1;
                if (i == 0 || j == 0 || k == 0)
                    draw_blocks[k][j][i] = true;
                else if (i == (Nx-1) || j == (Ny-1) || k == (Nz_surface))
                    draw_blocks[k][j][i] = true;
            }
        }
    }
    if( gui.generate_surface )
        generate_surface(gui);
    if( gui.generate_dungeons )
        generate_dungeons(gui);
    if( gui.generate_trees )
        generate_trees(gui);
    if( gui.generate_river )
        generate_river(gui);

    // Avoid drawing hidden blocks
    for (int k=1; k<Nz_dungeon-1; ++k) {
        for (int j = 1; j < Ny-1; ++j) {
            for (int i = 1; i < Nx-1; ++i) {
                if (blocks[k+1][j][i] == 0 || blocks[k-1][j][i] == 0 ||
                    blocks[k][j+1][i] == 0 || blocks[k][j-1][i] == 0 ||
                    blocks[k][j][i+1] == 0 || blocks[k][j][i-1] == 0){}
                else
                    draw_blocks[k][j][i] = false;

            }
        }
    }

}

void Grid::generate_surface(gui_scene_structure gui)
{

    // Fill terrain geometry
    for(size_t kv=0; kv<Ny; ++kv)
    {

        for(size_t ku=0; ku<Nx; ++ku)
        {

            // Compute local parametric coordinates (u,v) \in [0,1]
            const float u = ku/(Nx-1.0f);
            const float v = kv/(Ny-1.0f);
            const float z = evaluate_terrain_z(v,u, gui);

            const int num_blocks = z / step;

            const int block_z = Nz_surface + num_blocks;
            blocks[block_z][kv][ku] = 1;
            surface_z[kv][ku] = block_z;
            draw_blocks[block_z][kv][ku] = true;

            if (num_blocks > 0){
                for (size_t kz=1; kz < num_blocks; ++ kz){
                    blocks[kz + Nz_surface][kv][ku] = 2;
                    draw_blocks[kz + Nz_surface][kv][ku] = true;
                }
            }else{
                for (int kz=0; kz > num_blocks; -- kz){
                    blocks[kz + Nz_surface][kv][ku] = 0;
                    draw_blocks[kz + Nz_surface][kv][ku] = false;
                }
                for (int kz=1; kz < 9; ++ kz){
                    blocks[-kz + block_z][kv][ku] = 2;
                    draw_blocks[-kz + block_z][kv][ku] = true;
                }
            }
        }
    }
}

void Grid::generate_dungeons(gui_scene_structure gui)
{

    float scaling = gui.scaling;
    int octave = gui.octave;
    float persistency = gui.persistency;
    float frequency_gain = gui.frequency;
    int show_blocks = 0;

    for (int k=1; k<Nz_dungeon; ++k){
        for (int j=0; j<Ny; ++j){
            for (int i=0; i<Nx; ++i){

                const float u = i/(Nx-1.0f);
                const float v = j/(Ny-1.0f);
                const float w = k/(Nz_dungeon-1.0f);

                const float p = perlin(scaling*u, scaling*v, w, octave, persistency, frequency_gain);

                if (p > gui.min_noise){
                    show_blocks += 1;
                    draw_blocks[k][j][i] = true;
                    blocks[k][j][i] = 3;
                }
                else{
                    draw_blocks[k][j][i] = false;
                    blocks[k][j][i] = 0;
                }
            }
        }
    }
}

void Grid::generate_trees(gui_scene_structure gui)
{
    int num_trees = 17;

    std::uniform_int_distribution<int> distrib_x(6, (int) Nx-7);
    std::uniform_int_distribution<int> distrib_y(6, (int) Ny-7);

    std::uniform_int_distribution<int> size(4, 6);

    std::uniform_int_distribution<int> size_leave_edge(0, 1);
    std::uniform_int_distribution<int> size_leave_middle(1, 2);

    std::uniform_int_distribution<int> size_leave_end(0, 1);

    std::default_random_engine generator;
    int p_x, p_y, z, s, sl, el, sle, slm, slend;

    for (int n=0; n<num_trees; ++n){
        p_x = distrib_x(generator);
        p_y = distrib_y(generator);
        z = surface_z[p_y][p_x];

        if (blocks[z][p_y][p_x] == 1 and blocks[z+1][p_y][p_x] == 0){

            s = size(generator);
            std::uniform_int_distribution<int> start_leave(s-2, s-1);
            sl = start_leave(generator);
            std::uniform_int_distribution<int> end_leave(s, s+1);
            el = end_leave(generator);

            sle = size_leave_edge(generator);
            slm = size_leave_middle(generator);
            slend = size_leave_end(generator);

            for (int t=1; t<=s; ++t){
                blocks[z+t][p_y][p_x] = 6;
                draw_blocks[z+t][p_y][p_x] = true;
            }

            for (int t=sl; t<=el; ++t){
                if (t == sl || t == el){
                    for (int i=-sle; i<=sle; ++i) {
                        for (int j =-sle; j <= sle; ++j) {
                            if (blocks[z + t][p_y + i][p_x + j] == 0)
                                blocks[z + t][p_y + i][p_x + j] = 5;
                            draw_blocks[z + t][p_y + i][p_x + j] = true;
                        }
                    }
                }else {
                    for (int i =-slm; i <= slm; ++i) {
                        for (int j =-slm; j <= slm; ++j) {
                            if (blocks[z + t][p_y + i][p_x + j] == 0)
                                blocks[z + t][p_y + i][p_x + j] = 5;
                            draw_blocks[z + t][p_y + i][p_x + j] = true;
                        }
                    }
                }
            }
            if (slend == 1){
                if (blocks[z+s+1][p_y][p_x] == 0){
                    blocks[z+s+1][p_y][p_x] = 5;
                    draw_blocks[z+s+1][p_y][p_x] = true;
                }

            }
        }
    }

}

void Grid::generate_river(gui_scene_structure gui)
{
    // TODO
}

float evaluate_terrain_z(float u, float v, const gui_scene_structure& gui)
{
    const float Se = gui.se;
    const float scaling = gui.scaling;
    const int octave = gui.octave;
    const float persistency = gui.persistency;
    const float frequency_gain = gui.frequency;

    const float p = perlin(scaling * u, scaling * v, octave, persistency, frequency_gain);
    const float z = p * pow(fabs(p), Se) * gui.height;

    return z;
}

mesh create_block(float l)
{
    mesh block;

    const vec3 p000 = {-l,-l,-l};
    const vec3 p001 = {-l,-l, l};
    const vec3 p010 = {-l, l,-l};
    const vec3 p011 = {-l, l, l};
    const vec3 p100 = { l,-l,-l};
    const vec3 p101 = { l,-l, l};
    const vec3 p110 = { l, l,-l};
    const vec3 p111 = { l, l, l};

    block.position = {
            p111, p011, p001, p101,
            p101, p001, p000, p100,
            p011, p001, p000, p010,
            p111, p110, p101, p100,
            p110, p010, p000, p100,
            p111, p011, p010, p110
    };

    block.connectivity = {
            {0,1,2}, {0,2,3}, {4,5,6}, {4,6,7},
            {8,11,10}, {8,10,9}, {17,16,19}, {17,19,18},
            {23,22,21}, {23,21,20}, {13,12,14}, {13,14,15}
    };

    const float e = 1e-3f;
    const float u0 = 0.0f;
    const float u1 = 0.25f+e;
    const float u2 = 0.5f-e;
    const float u3 = 0.75f-e;
    const float u4 = 1.0f;
    const float v0 = 0.0f;
    const float v1 = 1.0f/3.0f+e;
    const float v2 = 2.0f/3.0f-e;
    const float v3 = 1.0f;
    block.texture_uv = {
            {u2,v1}, {u2,v0}, {u1,v0}, {u1,v1},
            {u1,v1}, {u0,v1}, {u0,v2}, {u1,v2},
            {u3,v1}, {u4,v1}, {u4,v2}, {u3,v2},
            {u2,v1}, {u2,v2}, {u1,v1}, {u1,v2},
            {u2,v2}, {u2,v3}, {u1,v3}, {u1,v2},
            {u2,v1}, {u3,v1}, {u3,v2}, {u2,v2},
    };

    return block;

}