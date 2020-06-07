
// Include VCL library
#include "vcl/vcl.hpp"

// Include common part for exercises
#include "main/helper_scene/helper_scene.hpp"

// Include exercises
#include "scenes/scenes.hpp"



// ************************************** //
// Global data declaration
// ************************************** //

// Storage for shaders indexed by their names
std::map<std::string,GLuint> shaders;

// General shared elements of the scene such as camera and its controler, visual elements, etc
scene_structure scene;

// The graphical interface. Contains Window object and GUI related variables
gui_structure gui;

// Part specific data - you will specify this object in the corresponding exercise part
scene_model scene_current;


// ************************************** //
// GLFW event listeners
// ************************************** //

void window_size_callback(GLFWwindow* /*window*/, int width, int height);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_click_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void keyboard_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// ************************************** //
// Start program
// ************************************** //

int main(int argc, char** argv)
{
    int key;
    if(argc == 1) {
        std::cout << "No argument passed!" << std::endl;
        std::cout << "Choosing random key . . ." << std::endl;
        key = time(NULL);
        std::cout << "The chosen key was " << key << std::endl;
    }

    else {
        key = std::stoi(argv[1]);
        std::cout << "You passed the key " << key << std::endl;
    }

    // ************************************** //
    // Initialization and data setup
    // ************************************** //

    // Initialize external libraries and window
    initialize_interface(gui);

    // Set GLFW events listener
    glfwSetCursorPosCallback(gui.window, cursor_position_callback );
    glfwSetMouseButtonCallback(gui.window, mouse_click_callback);
    glfwSetScrollCallback(gui.window, mouse_scroll_callback);
    glfwSetKeyCallback(gui.window, keyboard_input_callback);
    glfwSetWindowSizeCallback(gui.window, window_size_callback);

    load_shaders(shaders);
    
    setup_scene(scene, gui, shaders);

    opengl_debug();
    std::cout<<"*** Setup Data ***"<<std::endl;
    scene_current.setup_data(shaders, scene, gui, key);
    std::cout<<"\t [OK] Data setup"<<std::endl;
    opengl_debug();


    // ************************************** //
    // Animation loop
    // ************************************** //



    std::cout<<"*** Start GLFW animation loop ***"<<std::endl;
    std::cout<<""<<std::endl;
    std::cout<<"Welcome to Minecraft" <<std::endl;
    std::cout<<""<<std::endl;
    std::cout<<"Camera angle movement: Right mouse button or keyboard arrows"<<std::endl;
    std::cout<<"Player angle movement: Left mouse button or A, D"<<std::endl;
    std::cout<<"Player movement: W, S"<<std::endl;
    std::cout<<"Jump: SPACE"<<std::endl;
    std::cout<<"Break blocks: Get close and press LEFT CONTROL"<<std::endl;
    std::cout<<""<<std::endl;
    vcl::glfw_fps_counter fps_counter;
    while( !glfwWindowShouldClose(gui.window) )
    {
        opengl_debug();

        // Clear all color and zbuffer information before drawing on the screen
        clear_screen();opengl_debug();
        // Set a white image texture by default
        glBindTexture(GL_TEXTURE_2D,scene.texture_white);

        // Create the basic gui structure with ImGui
        gui_start_basic_structure(gui,scene);

        // Perform computation and draw calls for each iteration loop
        int fps_value = get_fps(fps_counter);
        scene_current.frame_draw(shaders, scene, gui, fps_value); opengl_debug();


        // Render GUI and update window
        ImGui::End();
        scene.camera_control.update = !(ImGui::IsAnyWindowFocused());
        vcl::imgui_render_frame(gui.window);

        update_fps_title(gui.window, gui.window_title, fps_counter);

        glfwSwapBuffers(gui.window);
        glfwPollEvents();
        opengl_debug();

    }
    std::cout<<"*** Stop GLFW loop ***"<<std::endl;

    // Cleanup ImGui and GLFW
    vcl::imgui_cleanup();

    glfwDestroyWindow(gui.window);
    glfwTerminate();

    return 0;
}

void window_size_callback(GLFWwindow* /*window*/, int width, int height)
{
    glViewport(0, 0, width, height);
    scene.camera.perspective.image_aspect = width / static_cast<float>(height);;
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    scene.camera_control.update_mouse_move(scene.camera, window, float(xpos), float(ypos));
    scene_current.mouse_move(scene, window);
}
void mouse_click_callback(GLFWwindow* window, int button, int action, int mods)
{
    ImGui::SetWindowFocus(nullptr);
    scene.camera_control.update_mouse_click(scene.camera, window, button, action, mods);
    scene_current.mouse_click(scene, window,button,action,mods);
}
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    scene_current.mouse_scroll(scene, window, float(xoffset), float(yoffset));
}
void keyboard_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    scene_current.keyboard_input(scene, window, key, scancode, action, mods);
}
