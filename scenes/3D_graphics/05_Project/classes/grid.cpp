#include "grid.hpp"

using namespace vcl;

#define NONE 0
#define GRASS 1
#define DIRTY 2
#define STONE 3
#define WATER 4
#define WOOD 5
#define LEAVE 6
#define WATER 7

float evaluate_terrain_z(float u, float v, const gui_scene_structure& gui_scene);
mesh create_block(float l);
mesh create_billboard_block(float l);

void Grid::setup()
{
    block = create_block(step / 2);
    block_billboard = create_billboard_block(step / 2);
    block_billboard.uniform.shading = {1,0,0};
    block_texture_grass = create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/grass.png"),
                                             GL_REPEAT, GL_REPEAT );
    block_texture_dirty = create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/dirty.png"),
                                             GL_REPEAT, GL_REPEAT );
    block_texture_stone = create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/rock.png"),
                                             GL_REPEAT, GL_REPEAT );
    block_texture_water = create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/water.png"),
                                             GL_REPEAT, GL_REPEAT );
    block_texture_leave = create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/leaves_oak.png"),
                                             GL_REPEAT, GL_REPEAT );
    block_texture_wood =  create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/wood.png"),
                                             GL_REPEAT, GL_REPEAT );
    block_billboard.uniform.shading = {1,0,0};
}

void Grid::frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, bool wireframe)
{
    for (int k=0; k<Nz; ++k){
        for (int j=0; j<Ny; ++j){
            for (int i=0; i<Nx; ++i){
                if (blocks[k][j][i] != 0 and draw_blocks[k][j][i]) {
                    if(blocks[k][j][i] < 6) {
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
                            glBindTexture(GL_TEXTURE_2D, block_texture_wood);
                        draw(block, scene.camera, shaders["mesh"]);
                        if (wireframe)
                            draw(block, scene.camera, shaders["wireframe"]);
                    }
                    else {
                        glEnable(GL_BLEND);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                        //glDepthMask(false);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // avoids sampling artifacts
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // avoids sampling artifacts
                        block_billboard.uniform.transform.translation = {i * step, j * step, k * step};
                        if (blocks[k][j][i] == 6) {
                            glBindTexture(GL_TEXTURE_2D, block_texture_leave);
                            block_billboard.uniform.color = {0.1,0.9,0.1};
                        }
                        draw(block_billboard, scene.camera, shaders["mesh"]);
                        if (wireframe)
                            draw(block_billboard, scene.camera, shaders["wireframe"]);
                        //glDepthMask(true);
                        glDisable(GL_BLEND);                    
                    }
                    glBindTexture(GL_TEXTURE_2D, scene.texture_white);
                }
            }
        }
    }
}
void Grid::create_grid(gui_scene_structure gui, std::default_random_engine g)
{
    gen = g;

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
                surface_z[kv][ku] = block_z;
            }else{
                for (int kz=0; kz > num_blocks; -- kz){
                    blocks[kz + Nz_surface][kv][ku] = 0;
                    draw_blocks[kz + Nz_surface][kv][ku] = false;
                }
                for (int kz=1; kz < 4; ++ kz){
                    blocks[-kz + block_z][kv][ku] = 2;
                    draw_blocks[-kz + block_z][kv][ku] = true;
                }
                surface_z[kv][ku] = block_z;
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

// Generate trees
void Grid::generate_trees(gui_scene_structure gui)
{
    int num_trees = gui.trees;
    int type_tree = 0; // TODO

    std::uniform_int_distribution<int> distrib_x(3, (int) Nx-4);
    std::uniform_int_distribution<int> distrib_y(3, (int) Ny-4);

    // For tree:
    std::uniform_int_distribution<int> size(3, 6);

    // For foliage:
    std::uniform_int_distribution<int> d_n_x(3, 4);
    std::uniform_int_distribution<int> d_n_y(3, 4);
    std::uniform_int_distribution<int> d_n_z(3, 4);
    std::uniform_real_distribution<float> sc(2.0, 4.0);
    std::uniform_real_distribution<float> pe(0.4, 0.55);

    int p_x, p_y, p_z;
    int n = 0;

    while (n < num_trees) {
        p_x = distrib_x(gen);
        p_y = distrib_y(gen);
        p_z = surface_z[p_y][p_x];

        // First, we see if the condition to draw a tree is satisfied
        if(blocks[p_z][p_y][p_x] == 1 and blocks[p_z+1][p_y][p_x] == 0) {
            
            int s = size(gen);
            for (int t=1; t<=s; t++){
                blocks[p_z+t][p_y][p_x] = 5;
                draw_blocks[p_z+t][p_y][p_x] = true;
            }

            float scaling = sc(gen);
            int octave = 7;
            float persistency = pe(gen);
            float frequency_gain = 2.0f;
            float min_noise = 0.65f;
            int show_blocks = 0;
            int n_x = d_n_x(gen);
            int n_y = d_n_y(gen);
            int n_z = d_n_z(gen);

            int height = s + p_z - 1;
            std::cout << height << std::endl;
            for (int k = 1; k < n_z; k++){
                for (int j = 0; j < n_y; j++){
                    for (int i = 0; i < n_x; i++){

                        const float u = i/(n_x-1.0f);
                        const float v = j/(n_y-1.0f);
                        const float w = k/(n_z-1.0f);
                        const float p = perlin(scaling*u, scaling*v, w, octave, persistency, frequency_gain);

                        if (p > min_noise && blocks[k + height][j+p_y - n_y / 2][i + p_x - n_x / 2] != 6){
                            show_blocks += 1;
                            draw_blocks[k + height][j + p_y - n_y / 2][i + p_x - n_x / 2] = true;
                            blocks[k + height][j + p_y - n_y / 2][i + p_x - n_x / 2] = 6;
                        }
                    }
                }
            }
            n++;
        }
    } 
}

void Grid::generate_river(gui_scene_structure gui)
{
    // TODO
}

int Grid::position_to_block(vec3 p)
{
    int x = p[0] / step;
    int y = p[1] / step;
    int z = p[2] / step;

    return blocks[z][y][x];
}

vec3 Grid::blocks_to_position(int x, int y, int z) {
    float u = x * step;
    float v = y * step;
    float w = z * step;

    return vec3{u, v, w};
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

mesh create_billboard_block(float l) {
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

    block.texture_uv = {
            {0,1}, {1,1}, {1,0}, {0,0},
            {0,1}, {1,1}, {1,0}, {0,0},
            {0,1}, {1,1}, {1,0}, {0,0},
            {0,1}, {1,1}, {1,0}, {0,0},
            {0,1}, {1,1}, {1,0}, {0,0},
            {0,1}, {1,1}, {1,0}, {0,0},
    };

    return block;
}