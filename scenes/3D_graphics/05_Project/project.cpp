#include "project.hpp"
#include <cmath>

#ifdef SCENE_3D_PROJECT

#define NONE 0
#define GRASS 1
#define DIRTY 2
#define STONE 3
#define WATER 4
#define LEAVE 5
#define WOOD 6


// Add vcl namespace within the current one - Allows to use function from vcl library without explicitely preceeding their name with vcl::
using namespace vcl;

const size_t Nx = 70; // Number of blocks in x
const size_t Ny = 50; // Number of blocks in y
const size_t Nz = 70; // Number of blocks in z
const size_t Nz_dungeon = 30; // Number of blocks in z
const int Nz_surface = 32; // Number of blocks in z
const float step = 1 / (float) Nx; // Minimum step (Divide by the biggest N)

mesh create_block(float len);
mesh_drawable update_block(mesh_drawable block, float height);
float evaluate_terrain_z(float u, float v, const gui_scene_structure& gui_scene);

/** This function is called before the beginning of the animation loop
    It is used to initialize all part-specific data */
void scene_model::setup_data(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_structure& gui)
{
    g.create_grid(gui_scene);

    block = create_block(step / 2);
    block.shader = shaders["mesh"];
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


    // Setup initial camera mode and position
    scene.camera.camera_type = camera_control_spherical_coordinates;
    scene.camera.scale = 3.0f;
    scene.camera.apply_rotation(0,0,0,1.2f);
}

/** This function is called at each frame of the animation loop.
    It is used to compute time-varying argument and perform data data drawing */
void scene_model::frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_structure& gui)
{
    // Drawing call: need to provide the camera information
    set_gui();
    g.create_grid(gui_scene);
    glEnable( GL_POLYGON_OFFSET_FILL ); // avoids z-fighting when displaying wireframe

    for (int k=0; k<Nz; ++k){
        for (int j=0; j<Ny; ++j){
            for (int i=0; i<Nx; ++i){
                if (g.blocks[k][j][i] != 0 and g.draw_blocks[k][j][i]){
                    block.uniform.transform.translation   = {i * step, j * step, k * step};
                    if (g.blocks[k][j][i] == 1)
                        glBindTexture(GL_TEXTURE_2D, block_texture_grass);
                    if (g.blocks[k][j][i] == 2)
                        glBindTexture(GL_TEXTURE_2D, block_texture_dirty);
                    if (g.blocks[k][j][i] == 3)
                        glBindTexture(GL_TEXTURE_2D, block_texture_stone);
                    if (g.blocks[k][j][i] == 4)
                        glBindTexture(GL_TEXTURE_2D, block_texture_water);
                    if (g.blocks[k][j][i] == 5)
                        glBindTexture(GL_TEXTURE_2D, block_texture_leave);
                    if (g.blocks[k][j][i] == 6)
                        glBindTexture(GL_TEXTURE_2D, block_texture_wood);
                    block = update_block(block, k * step);
                    draw(block, scene.camera);
                    glBindTexture(GL_TEXTURE_2D, scene.texture_white);
                }
            }
        }
    }

}

void grid::create_grid(gui_scene_structure gui)
{
    // Inialize vector with blocks type 1
    blocks = std::vector<std::vector<std::vector<int>>>(Nz, std::vector<std::vector<int>>(Ny, std::vector<int>(Nx, 0)));
    draw_blocks = std::vector<std::vector<std::vector<bool>>>(Nz, std::vector<std::vector<bool>>(Ny, std::vector<bool>(Nx, false)));

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

    // Not draw hidden blocks
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

void grid::generate_surface(gui_scene_structure gui)
{

    // Fill terrain geometry
    for(size_t kv=0; kv<Ny; ++kv)
    {

        for(size_t ku=0; ku<Nx; ++ku)
        {

            // Compute local parametric coordinates (u,v) \in [0,1]
            const float u = ku/(Nx-1.0f);
            const float v = kv/(Ny-1.0f);
            const float z = evaluate_terrain_z(v,u, gui) * gui.height / 10;

            const int num_blocks = z / step;
            const int block_z = Nz_surface + num_blocks;
            blocks[block_z][kv][ku] = 1;
            draw_blocks[block_z][kv][ku] = true;

            if (num_blocks > 0){
                for (size_t kz=0; kz < num_blocks; ++ kz){
                    blocks[kz + Nz_surface][kv][ku] = 2;
                    draw_blocks[kz + Nz_surface][kv][ku] = true;
                }
            }else{
                for (size_t kz=0; kz > num_blocks; -- kz){
                    blocks[kz + Nz_surface][kv][ku] = 0;
                    draw_blocks[kz + Nz_surface][kv][ku] = false;
                }
            }
        }
    }
}

void grid::generate_dungeons(gui_scene_structure gui)
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

    std::cout << show_blocks << std::endl;
}

void grid::generate_trees(gui_scene_structure gui)
{
    // TODO
}

void grid::generate_river(gui_scene_structure gui)
{
    // TODO
}

void scene_model::set_gui()
{
    ImGui::Text("Generate Map: "); ImGui::SameLine();
    ImGui::Checkbox("Surface", &gui_scene.generate_surface); ImGui::SameLine();
    ImGui::Checkbox("Dungeons", &gui_scene.generate_dungeons); ImGui::SameLine();
    ImGui::Checkbox("Trees", &gui_scene.generate_trees);   ImGui::SameLine();
    ImGui::Checkbox("River", &gui_scene.generate_river);   ImGui::SameLine();

    float height_min = 0.2f;
    float height_max = 1.5f;

    float scaling_min = 2.0f;
    float scaling_max = 8.0f;

    float persistency_min = 0.01f;
    float persistency_max = 2.0f;

    float frequency_gain_min = 1.0f;
    float frequency_gain_max = 4.0f;

    float min_noise_min = 0.3f;
    float min_noise_max = 0.8f;

    ImGui::Spacing();

    ImGui::SliderFloat("Height", &gui_scene.height, height_min, height_max);
    ImGui::SliderFloat("Scaling", &gui_scene.scaling, scaling_min, scaling_max);
    ImGui::SliderFloat("Persistency", &gui_scene.persistency, persistency_min, persistency_max);
    ImGui::SliderFloat("Frequency", &gui_scene.frequency, frequency_gain_min, frequency_gain_max);
    ImGui::SliderFloat("Min noise", &gui_scene.min_noise, min_noise_min, min_noise_max);
}

mesh_drawable update_block(mesh_drawable block, float height)
{


    /*
    // Applying different textures
    if(height > 0.5) {
        block.uniform.color = {0,1,0};
    }

    else if(height > 0.2) {
        block.uniform.color = {0.8,0.8,0};
    }

    else if(height > -1) {
        block.uniform.color = {0.1,0.1,0.1};
    }*/

    return block;
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


float evaluate_terrain_z(float u, float v, const gui_scene_structure& gui)
{
    vec2 u0[] = {{0.0f, 0.0f}, {0.5f,0.5f}, {0.2f,0.7f}, {0.8f,0.7f}};
    float h0[] = {3.0f,-1.5f,1.0f,2.0f};
    float sigma0[] = {0.5f,0.15f,0.2f,0.2f};
    float d = 0;
    float z = 0;

    for (unsigned int i = 0; i < sizeof(sigma0)/sizeof(sigma0[0]); i++){
        d = norm(vec2(u,v)-u0[i])/sigma0[i];
        z += h0[i]*std::exp(-d*d);
    }
    return z;
}

// Evaluate 3D position of the terrain for any (u,v) \in [0,1]
vec3 evaluate_terrain(float u, float v, const gui_scene_structure& gui_scene)
{
    const float x = 20*(u-0.5f);
    const float y = 20*(v-0.5f);
    const float z = evaluate_terrain_z(u,v, gui_scene);

    return {x,y,z};
}
#endif