#include "player.hpp"

using namespace vcl;

void Player::setup(float scale, std::map<std::string,GLuint>& shaders) {

    player_texture =  create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/player.png"),
                                             GL_REPEAT, GL_REPEAT );
    draw_player(scale);
    timer.scale = 0.5f;
    // Set the same shader for all the elements
    hierarchy.set_shader_for_all_elements(shaders["mesh"]);

    // Initialize helper structure to display the hierarchy skeleton
    hierarchy_visual_debug.init(shaders["segment_im"], shaders["mesh"]);
}

void Player::frame_draw(std::map<std::string, GLuint> &shaders, scene_structure &scene, gui_scene_structure gui_scene, Grid g) {

    timer.update();

    // Current time

    const float t = timer.t;
    std::cout << t << std::endl;

    mat3 const Symmetry = {-1,0,0, 0,1,0, 0,0,1};

    mat3 const R_arm = rotation_from_axis_angle_mat3({0,1,0}, 0.4f * std::sin(2*3.14f*(2* t-0.4f)) );
    mat3 const R_leg = rotation_from_axis_angle_mat3({0,1,0}, 0.8f * std::sin(2*3.14f*(2* t-0.4f)) );
    mat3 const R_head = rotation_from_axis_angle_mat3({0,0,1}, 0.3f * std::sin(2*3.14f*(t-0.6f)) );

    hierarchy["mov_head"].transform.rotation = R_head;

    hierarchy["mov_leg_left"].transform.rotation = R_leg;
    hierarchy["mov_leg_right"].transform.rotation = Symmetry * R_leg;

    hierarchy["mov_arm_right"].transform.rotation = R_arm;
    hierarchy["mov_arm_left"].transform.rotation = Symmetry * R_arm;

    hierarchy.update_local_to_global_coordinates();
    // The default display
    if(gui_scene.surface) {
        glBindTexture(GL_TEXTURE_2D, player_texture);
        draw(hierarchy, scene.camera, shaders["mesh"]);
        glBindTexture(GL_TEXTURE_2D, scene.texture_white);
    }

    if(gui_scene.wireframe) // Display the hierarchy as wireframe
        draw(hierarchy, scene.camera, shaders["wireframe"]);

    if(gui_scene.skeleton) // Display the skeleton of the hierarchy (debug)
        hierarchy_visual_debug.draw(hierarchy, scene.camera);
}

void Player::key_press(scene_structure &scene, GLFWwindow *window, int key, int scancode, int action, int mods) {

}

void Player::updatePosition(scene_structure &scene) {

}

void Player::move(float dx, float dy) {

}

void Player::turn(float d_angle) {

}

void Player::draw_player(float scale)
{
    // draw body
    const float size = 0.48f * scale;

    const float head_x = size + size/2.0f;
    const float head_y = 0.96f * scale;
    const float head_z = 0.96f * scale;

    const float body_y = head_y;
    const float body_z = 1.44f * scale;

    const float leg_y = head_y/2.0f;
    const float leg_z = body_z;

    const float arm_y = leg_y;
    const float arm_z =  body_z;

    mesh head = mesh_primitive_parallelepiped({0,0,0},{head_x,0,0},
                                                       {0,head_y,0},{0,0,head_z});
    mesh body = mesh_primitive_parallelepiped({0,0,0},{size,0,0},
                                                       {0,body_y,0},{0,0,body_z});
    mesh leg = mesh_primitive_parallelepiped({0,0,0},{size,0,0},
                                                       {0,leg_y,0},{0,0,leg_z});
    mesh arm = mesh_primitive_parallelepiped({0,0,0},{size,0,0},
                                                      {0,arm_y,0},{0,0,arm_z});
    mesh mov = mesh_primitive_sphere(0.01f);

    // Add texture
    const float x_a = 0.5f/4.0f;
    const float x_b = 0.5f/8.0f;

    const float y_a = 0.5f/2.0f;
    const float y_b = 0.5f/4.0f;


    leg.texture_uv = {
            {1*x_a,2*y_a+y_b}, {1*x_a,2*y_a}, {1*x_a+x_b,2*y_a}, {1*x_a+x_b,2*y_a+y_b},
            {x_b,4*y_a}, {x_b,2*y_a+y_b}, {x_a,2*y_a+y_b}, {x_a,4*y_a},
            {x_a,4*y_a}, {x_a,2*y_a+y_b}, {x_a+x_b,2*y_a+y_b}, {x_a+x_b,4*y_a},
            {2*x_a,4*y_a}, {2*x_a,2*y_a+y_b}, {x_a+x_b,2*y_a+y_b}, {x_a+x_b,4*y_a},
            {0,4*y_a}, {0,2*y_a+y_b}, {x_b,2*y_a+y_b}, {x_b,4*y_a},
            {1*x_b,2*y_a+y_b}, {1*x_b,2*y_a}, {1*x_a,2*y_a}, {1*x_a,2*y_a+y_b},
    };

    arm.texture_uv = {
            {6*x_a,2*y_a+y_b}, {6*x_a,2*y_a}, {6*x_a+x_b,2*y_a}, {6*x_a+x_b,2*y_a+y_b},
            {5*x_a+x_b,4*y_a}, {5*x_a+x_b,2*y_a+y_b}, {6*x_a,2*y_a+y_b}, {6*x_a,4*y_a},
            {6*x_a,4*y_a}, {6*x_a,2*y_a+y_b}, {6*x_a+x_b,2*y_a+y_b}, {6*x_a+x_b,4*y_a},
            {7*x_a,4*y_a}, {7*x_a,2*y_a+y_b}, {6*x_a+x_b,2*y_a+y_b}, {6*x_a+x_b,4*y_a},
            {5*x_a,4*y_a}, {5*x_a,2*y_a+y_b}, {5*x_a+x_b,2*y_a+y_b}, {5*x_a+x_b,4*y_a},
            {5*x_a+1*x_b,2*y_a+y_b}, {5*x_a+1*x_b,2*y_a}, {6*x_a,2*y_a}, {6*x_a,2*y_a+y_b},
    };

    body.texture_uv = {
            {3*x_a+x_b,2*y_a+y_b}, {3*x_a+x_b,2*y_a}, {4*x_a+x_b,2*y_a}, {4*x_a+x_b,2*y_a+y_b},
            {2*x_a+x_b,4*y_a}, {2*x_a+x_b,2*y_a+y_b}, {3*x_a+x_b,2*y_a+y_b}, {3*x_a+x_b,4*y_a},
            {3*x_a+x_b,4*y_a}, {3*x_a+x_b,2*y_a+y_b}, {4*x_a,2*y_a+y_b}, {4*x_a,4*y_a},
            {5*x_a,4*y_a}, {5*x_a,2*y_a+y_b}, {4*x_a,2*y_a+y_b}, {4*x_a,4*y_a},
            {2*x_a,4*y_a}, {2*x_a,2*y_a+y_b}, {2*x_a+x_b,2*y_a+y_b}, {2*x_a+x_b,4*y_a},
            {2*x_a+x_b,2*y_a+y_b}, {2*x_a+x_b,2*y_a}, {3*x_a+x_b,2*y_a}, {3*x_a+x_b,2*y_a+y_b},
    };

    head.texture_uv = {
            {2*x_a,1*y_a}, {2*x_a,0}, {3*x_a,0}, {3*x_a,1*y_a},
            {1*x_a,2*y_a}, {1*x_a,1*y_a}, {2*x_a,1*y_a}, {2*x_a,2*y_a},
            {2*x_a,2*y_a}, {2*x_a,1*y_a}, {3*x_a,1*y_a}, {3*x_a,2*y_a},
            {4*x_a,2*y_a}, {4*x_a,1*y_a}, {3*x_a,1*y_a}, {3*x_a,2*y_a},
            {0,2*y_a}, {0,1*y_a}, {1*x_a,1*y_a}, {1*x_a,2*y_a},
            {1*x_a,1*y_a}, {1*x_a,0}, {2*x_a,0}, {1*x_a,1*y_a},
    };


    hierarchy.add(body, "body");

    hierarchy.add(mov, "mov_head", "body", {head_x/2.0f,head_y/2.0f,body_z});
    hierarchy.add(mov, "mov_leg_right", "body", {size/2.0f,body_y-leg_y/2.0f,0});
    hierarchy.add(mov, "mov_leg_left", "body", {size/2.0f,body_y/2.0f-leg_y/2.0f,0});
    hierarchy.add(mov, "mov_arm_right", "body", {size/2.0f,-arm_y/2.0f,body_z});
    hierarchy.add(mov, "mov_arm_left", "body", {size/2.0f,body_y + arm_y/2.0f,body_z});

    hierarchy.add(head, "head", "mov_head", {-head_x/2.0f-size/4.0f,-head_y/2.0f, 0});

    hierarchy.add(leg, "leg_right", "mov_leg_right", {-size/2.0f, -leg_y/2.0f, -leg_z});
    hierarchy.add(leg, "leg_left", "mov_leg_left", {-size/2.0f, -leg_y/2.0f, -leg_z});

    hierarchy.add(arm, "arm_right", "mov_arm_right", {-size/2.0f, -arm_y/2.0f, -arm_z});
    hierarchy.add(arm, "arm_left", "mov_arm_left", {-size/2.0f, -arm_y/2.0f, -arm_z});

}

