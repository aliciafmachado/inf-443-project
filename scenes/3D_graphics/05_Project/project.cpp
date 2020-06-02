#include "project.hpp"

#ifdef SCENE_3D_PROJECT
#define KEY 1

// Add vcl namespace within the current one - Allows to use function from vcl library without explicitely preceeding their name with vcl::
using namespace vcl;

mesh create_block(float len);
mesh_drawable update_block(mesh_drawable block, float height);

/** This function is called before the beginning of the animation loop
    It is used to initialize all part-specific data */
void scene_model::setup_data(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_structure& gui)
{
    std::default_random_engine gen(KEY);
    g.setup();
    g.create_grid(gui_scene, gen);

    skybox.setup();
    player.setup(g.step/1.9f, shaders, &g);
    m1.setup(g.step*1.9f, shaders, &g, &player);
    //m2.setup(g.step*1.9f, shaders, &g, &player);
    //m3.setup(g.step*1.9f, shaders, &g, &player);

    // Setup initial camera mode and position
    scene.camera.camera_type = camera_control_spherical_coordinates;
    scene.camera.scale = 0.015f;
    float dc = sqrt(player.size * player.size / 4.0f + player.body_y * player.body_y / 4.0f);
    float teta = atan((player.size/2.0f)/(player.body_y/2.0f));
    vec3 center = vec3{dc * (float)sin(player.angle+teta),dc*(float)cos(player.angle+teta), 0 };
    scene.camera.translation = -player.hierarchy["body"].transform.translation - center;
    scene.camera.apply_rotation(-M_PI/2.0f,0,0, M_PI/2.0f);
}

/** This function is called at each frame of the animation loop.
    It is used to compute time-varying argument and perform data data drawing */
void scene_model::frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_structure& gui, int fps)
{
    // Drawing call: need to provide the camera information
    set_gui();
    glEnable( GL_POLYGON_OFFSET_FILL ); // avoids z-fighting when displaying wireframe
    skybox.frame_draw(shaders, scene, gui_scene.wireframe, fps);
    g.frame_draw(shaders, scene, gui_scene.wireframe, fps);
    player.frame_draw(shaders, scene, gui_scene, fps);
    m1.frame_draw(shaders, scene, gui_scene, fps);
   // m2.frame_draw(shaders, scene, gui_scene, fps);
    //m3.frame_draw(shaders, scene, gui_scene, fps);
}


void scene_model::set_gui()
{
    ImGui::Text("Player: "); ImGui::SameLine();
    ImGui::Checkbox("Surface player", &gui_scene.surface); ImGui::SameLine();
    ImGui::Checkbox("Wireframe", &gui_scene.wireframe); ImGui::SameLine();
    ImGui::Checkbox("skeleton", &gui_scene.skeleton);   ImGui::SameLine();

    ImGui::Spacing();

    ImGui::Text("Generate Map: "); ImGui::SameLine();
    ImGui::Checkbox("Surface", &gui_scene.generate_surface); ImGui::SameLine();
    ImGui::Checkbox("Dungeons", &gui_scene.generate_dungeons); ImGui::SameLine();
    ImGui::Checkbox("Trees", &gui_scene.generate_trees);   ImGui::SameLine();
    ImGui::Checkbox("River", &gui_scene.generate_river);   ImGui::SameLine();

    float height_min = 0.02f;
    float height_max = 0.50f;

    float scaling_min = 0.5f;
    float scaling_max = 8.0f;

    float persistency_min = 0.01f;
    float persistency_max = 1.0f;

    float frequency_gain_min = 0.01f;
    float frequency_gain_max = 5.0f;

    float min_noise_min = 0.3f;
    float min_noise_max = 0.8f;

    float se_min = 0.1f;
    float se_max = 5.0f;

    int trees_min = 0;
    int trees_max = 100;

    ImGui::Spacing();

    ImGui::SliderFloat("Height", &gui_scene.height, height_min, height_max);
    ImGui::SliderFloat("Scaling", &gui_scene.scaling, scaling_min, scaling_max);
    ImGui::SliderFloat("Persistency", &gui_scene.persistency, persistency_min, persistency_max);
    ImGui::SliderFloat("Frequency", &gui_scene.frequency, frequency_gain_min, frequency_gain_max);
    ImGui::SliderFloat("Min noise", &gui_scene.min_noise, min_noise_min, min_noise_max);
    ImGui::SliderFloat("Se", &gui_scene.se, se_min, se_max);
    ImGui::SliderInt("Trees", &gui_scene.trees, trees_min, trees_max);
}

void scene_model::keyboard_input(scene_structure& scene, GLFWwindow* window, int key, int scancode, int action, int mods) {
   player.keyboard_input(scene, window, key, scancode, action, mods);
}

void scene_model::mouse_click(scene_structure& scene, GLFWwindow* window, int button, int action, int mods) {
    player.mouse_click(scene, window, button, action, mods);
}

void scene_model::mouse_move(scene_structure& scene, GLFWwindow* window) {
    // Window size
    int w=0;
    int h=0;
    glfwGetWindowSize(window, &w, &h);

    // Current cursor position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Convert pixel coordinates to relative screen coordinates between [-1,1]
    const float x = 2*float(xpos)/float(w)-1;
    const float y = 1-2*float(ypos)/float(h);

    // std::cout << "x = " << x << std::endl;
    // std::cout << "y = " << y << std::endl;

}


#endif