#pragma once

#include "main/scene_base/base.hpp"

#ifdef SCENE_3D_PROJECT

// Stores some parameters that can be set from the GUI
struct gui_scene_structure
{
    bool wireframe   = false;
    bool surface     = true;
    bool skeleton    = false;

    float height = 0.6f;
    float scaling = 3.0f;
    int octave = 7;
    float persistency = 0.4f;

    bool display_keyframe = true;
    bool display_polygon  = true;
};

// Store a vec3 (p) + time (t)
struct vec3t{
    vcl::vec3 p; // position
    float t;     // time
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

    void set_gui();

    // visual representation of a surface
    vcl::mesh_drawable terrain;
    vcl::mesh_drawable tronc;
    vcl::mesh_drawable foliage;
    vcl::mesh_drawable tronc_c;
    vcl::mesh_drawable foliage_c;
    vcl::mesh_drawable surface;
    vcl::mesh_drawable skybox;

    std::vector<vcl::vec3> tree_position;
    std::vector<vcl::vec3> champignons_position;
    std::vector<vcl::vec3> bill_position;

    gui_scene_structure gui_scene;
    GLuint texture_terrain_id;
    GLuint bill_texture_id;
    GLuint bill_flower_texture_id;
    GLuint skybox_texture_id;

    // Store the index of a selected sphere
    int picked_object;
    vcl::timer_interval timer;

    // Called every time the mouse is clicked
    void mouse_click(scene_structure& scene, GLFWwindow* window, int button, int action, int mods);
    // Called every time the mouse is moved
    void mouse_move(scene_structure& scene, GLFWwindow* window);
    void draw_trajectory(std::map<std::string,GLuint>& shaders, scene_structure& scene);

    // Data (p_i,t_i)
    vcl::buffer<vec3t> keyframes; // Given (position,time)

    vcl::mesh_drawable sphere;                             // keyframe samples
    vcl::segment_drawable_immediate_mode segment_drawer;   // used to draw segments between keyframe samples
    vcl::curve_dynamic_drawable trajectory;                // Draw the trajectory of the moving point as a curve

    // Store the index of a select

    // Movement functions
    vcl::timer_interval penguin_timer;
    void create_penguin(float scale);
    void create_trajectory();
    void draw_pinguim(std::map<std::string,GLuint>& shaders, scene_structure& scene);
    vcl::hierarchy_mesh_drawable hierarchy;
    vcl::hierarchy_mesh_drawable_display_skeleton hierarchy_visual_debug;

};

#endif