#pragma once

#include "main/scene_base/base.hpp"
#include "classes/grid.hpp"
#include "classes/gui_scene_structure.hpp"
#include "classes/monster.hpp"
#include "classes/player.hpp"
#include "classes/skybox.hpp"

#ifdef SCENE_3D_PROJECT



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
    void frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_structure& gui, int fps_value);

    Skybox skybox;
    Grid g;
    Player player;
    Monster m1;
    Monster m2;
    Monster m3;

    gui_scene_structure gui_scene;
    void set_gui();

    int fps;

    // Interaction
    void mouse_click(scene_structure& scene, GLFWwindow* window, int button, int action, int mods);
    void mouse_move(scene_structure& scene, GLFWwindow* window);
    void keyboard_input(scene_structure& scene, GLFWwindow* window, int key, int scancode, int action, int mods);
};

#endif