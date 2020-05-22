#include "project.hpp"

#ifdef SCENE_3D_PROJECT

// Add vcl namespace within the current one - Allows to use function from vcl library without explicitely preceeding their name with vcl::
using namespace vcl;

const size_t Nx = 50; // Number of blocks in x
const size_t Ny = 40; // Number of blocks in y
const size_t Nz = 30; // Number of blocks in z
const float v = 1 / (float) Nx; // Minimum step (Divide by the biggest N)
mesh create_block(float len);

/** This function is called before the beginning of the animation loop
    It is used to initialize all part-specific data */
void scene_model::setup_data(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_structure& gui)
{
    g.create_grid(gui_scene);

    sphere = mesh_primitive_sphere(0.01f);
    sphere.uniform.color = {1,1,0};
    sphere.shader = shaders["mesh"]; // associate default shader to sphere

    block = create_block(0.01f);
    block.uniform.color = {1,1,0};
    block.shader = shaders["mesh"];

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
    float cont_x = 0;
    float cont_y = 0;
    float cont_z = 0;
    for (auto &matrix : g.blocks) {
        cont_y = 0;
        for (auto &line : matrix) {
            cont_x = 0;
            for (auto &value : line){
                if (value == 1){
                    block.uniform.transform.translation   = {cont_x, cont_y, cont_z};
                    draw(block, scene.camera);
                }
                cont_x += v;
            }
            cont_y += v;
        }
        cont_z += v;
    }

}

void grid::create_grid(gui_scene_structure gui)
{
    // Inialize vector with blocks type 1
    blocks = std::vector<std::vector<std::vector<int>>>(Nz, std::vector<std::vector<int>>(Ny, std::vector<int>(Nx, 1)));

    float scaling = gui.scaling;
    int octave = gui.octave;
    float persistency = gui.persistency;
    float frequency_gain = gui.frequency;

    for (int k=0; k<Nz; ++k){
        for (int j=0; j<Ny; ++j){
            for (int i=0; i<Nx; ++i){
                const float p = perlin(scaling*i*v, scaling*j*v, scaling*k*v, octave, persistency, frequency_gain);
                if (gui.height * p < gui.min_noise)
                    blocks[k][j][i] = 0;
            }
        }
    }
}

void grid::draw_grid(std::map<std::string,GLuint>& shaders)
{
    // TODO
}

void scene_model::set_gui()
{
    float height_min = 0.2f;
    float height_max = 1.0f;

    float scaling_min = 2.0f;
    float scaling_max = 8.0f;

    float persistency_min = 0.01f;
    float persistency_max = 2.0f;

    float frequency_gain_min = 1.0f;
    float frequency_gain_max = 4.0f;

    float min_noise_min = 0.3f;
    float min_noise_max = 0.8f;

    ImGui::SliderFloat("Height", &gui_scene.height, height_min, height_max);
    ImGui::SliderFloat("Scaling", &gui_scene.scaling, scaling_min, scaling_max);
    ImGui::SliderFloat("Persistency", &gui_scene.persistency, persistency_min, persistency_max);
    ImGui::SliderFloat("Frequency", &gui_scene.frequency, frequency_gain_min, frequency_gain_max);
    ImGui::SliderFloat("Min noise", &gui_scene.min_noise, min_noise_min, min_noise_max);
}

mesh create_block(float len){
    mesh block;
    
    float l = len;

    const vec3 p000 = {-l,-l,-l};
    const vec3 p001 = {-l,-l, l};
    const vec3 p010 = {-l, l,-l};
    const vec3 p011 = {-l, l, l};
    const vec3 p100 = { l,-l,-l};
    const vec3 p101 = { l,-l, l};
    const vec3 p110 = { l, l,-l};
    const vec3 p111 = { l, l, l};

    block.position = {
            p000, p100, p110, p010,
            p010, p110, p111, p011,
            p100, p110, p111, p101,
            p000, p001, p010, p011,
            p001, p101, p111, p011,
            p000, p100, p101, p001
    };

    block.connectivity = {
            {0,1,2}, {0,2,3}, {4,5,6}, {4,6,7},
            {8,11,10}, {8,10,9}, {17,16,19}, {17,19,18},
            {23,22,21}, {23,21,20}, {13,12,14}, {13,14,15}
    };

    return block;

}
#endif