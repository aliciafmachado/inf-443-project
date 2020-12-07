#include "player.hpp"

using namespace vcl;

void Player::setup(float scale_, std::map<std::string,GLuint>& shaders, Grid* g_,  gui_scene_structure* gui) {
    gui_scene = gui;
    g =  g_;
    // Scaling
    scale = scale_;

    size *= scale;
    head_x *= scale;
    head_y *= scale;
    head_z *= scale;
    body_y *= scale;
    body_z *= scale;
    leg_y *= scale;
    leg_z *= scale;
    arm_y *= scale;
    arm_z *=  scale;

    dist_feet = leg_z + g->step / 2.0f;

    dist_head = body_z + head_z;

    bool find_start_place = false;

    distance = 1.2f * scale;
    error = leg_z / 10.0f;
    v = {0, 0, 0};

    std::uniform_int_distribution<int> start_x(0, g->Nx);
    std::uniform_int_distribution<int> start_y(0, g->Ny);
    std::default_random_engine generator;
    generator.seed(time(NULL));
    create_player();

    x = start_x(generator);
    y = start_y(generator);

    while (!find_start_place){

        z = g->surface_z[y][x];
        if (g->blocks[z+1][y][x] == 0 && g->blocks[z+2][y][x] == 0 && g->blocks[z+3][y][x] == 0 && g->blocks[z+4][y][x] == 0)
            find_start_place = true;
        else
            z += 1;
    }
    p = g->blocks_to_position(x, y, z) + vec3{0, 0, dist_feet};
    //std::cout << "player" << std::endl;
    //std::cout << "x = " << x << ", y = " << y << std::endl;
    angle = 0;
    camera_angle = 0;
    timer.scale = 0.5f;
    // Set the same shader for all the elements
    hierarchy.set_shader_for_all_elements(shaders["mesh"]);

    // Initialize helper structure to display the hierarchy skeleton
    hierarchy_visual_debug.init(shaders["segment_im"], shaders["mesh"]);

    hierarchy["body"].transform.translation = p;
}

void Player::frame_draw(std::map<std::string, GLuint> &shaders, scene_structure &scene, gui_scene_structure gui_scene, int fps_)
{
    if (fps_ == 0)
        fps = 10;
    else
        fps = fps_;
    timer.update();
    updatePosition(scene);
    updateCamera(scene);

    // The default display
    if(gui_scene.surface) {
        glBindTexture(GL_TEXTURE_2D, player_texture);
        draw(hierarchy, scene.camera, shaders["mesh"]);
        glBindTexture(GL_TEXTURE_2D, scene.texture_white);
    }

    hierarchy.update_local_to_global_coordinates();
    if(gui_scene.wireframe) // Display the hierarchy as wireframe
        draw(hierarchy, scene.camera, shaders["wireframe"]);

    if(gui_scene.skeleton) // Display the skeleton of the hierarchy (debug)
        hierarchy_visual_debug.draw(hierarchy, scene.camera);
}

void Player::keyboard_input(scene_structure &scene, GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (!falling) {
        moving_up = (glfwGetKey(window, GLFW_KEY_W));
        moving_down = (glfwGetKey(window, GLFW_KEY_S));
        jumping = (glfwGetKey(window, GLFW_KEY_SPACE));
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)){
        const vec2 cursor = glfw_cursor_coordinates_window(window);

        // Create the 3D ray passing by the selected point on the screen
        const ray r = picking_ray(scene.camera, cursor);
        float d = g->step/2.0f;

        while (d < 6 * g->step){
            const vec3 b = r.p + r.u * d;
            int box = g->position_to_block(b);
            if (box != 0 ){
                if (box != 4)
                    g->delete_block(b);
                break;
            }
            d += g->step/2.0f;
        }
    }
    moving_left = (glfwGetKey(window, GLFW_KEY_A));
    moving_right = (glfwGetKey(window, GLFW_KEY_D));

    moving_camera_up = (glfwGetKey(window, GLFW_KEY_UP));
    moving_camera_down = (glfwGetKey(window, GLFW_KEY_DOWN));
    moving_camera_left = (glfwGetKey(window, GLFW_KEY_LEFT));
    moving_camera_right = (glfwGetKey(window, GLFW_KEY_RIGHT));
}

void Player::updatePosition(scene_structure &scene) {
    const float t = timer.t;

    moving = moving_up || moving_down || jumping || falling;
    bool turning = moving_left || moving_right;
    float speed = g->step / 27.0f * (400 / (float) fps);
    float speed_turn = M_PI / (16*27.0f) * (400 / (float) fps);
    if (moving || turning) {

        mat3 const Symmetry = {-1, 0, 0, 0, 1, 0, 0, 0, 1};

        mat3 const R_arm = rotation_from_axis_angle_mat3({0, 1, 0}, 0.4f * std::sin(2 * 3.14f * (2 * t - 0.4f)));
        mat3 const R_leg = rotation_from_axis_angle_mat3({0, 1, 0}, 0.8f * std::sin(2 * 3.14f * (2 * t - 0.4f)));
        mat3 const R_head = rotation_from_axis_angle_mat3({0, 0, 1}, 0.3f * std::sin(2 * 3.14f * (t - 0.6f)));

        hierarchy["mov_head"].transform.rotation = R_head;

        hierarchy["mov_leg_left"].transform.rotation = Symmetry * R_leg;
        hierarchy["mov_leg_right"].transform.rotation = R_leg;

        hierarchy["mov_arm_right"].transform.rotation = R_arm;
        hierarchy["mov_arm_left"].transform.rotation = Symmetry * R_arm;

        if (diving){
            if (jumping){
                jump(scale * (float) fps  * ((float) fps * -0.000183f+0.039301f) / 3.0f);
                falling = false;
            }else{
                fall(scale * (float) fps  * (173040.0411f*(float)pow((double)fps, (double)-1.8751f)) / 5.0f);
            }
        }
        else if (jumping)
            jump(scale * (float) fps  * (-0.000183f*(float)fps+0.039301f));
        else if (falling)
            fall(scale * (float) fps * (173040.0411f*(float)pow((double)fps, (double)-1.8751f)));
        if (moving)
            move(speed);
        if (turning)
            turn(speed_turn);
    }else{

        mat3 const R_arm = rotation_from_axis_angle_mat3({0, 1, 0}, 0);
        mat3 const R_leg = rotation_from_axis_angle_mat3({0, 1, 0}, 0);
        mat3 const R_head = rotation_from_axis_angle_mat3({0, 0, 1}, 0);

        hierarchy["mov_head"].transform.rotation = R_head;

        hierarchy["mov_leg_left"].transform.rotation = R_leg;
        hierarchy["mov_leg_right"].transform.rotation = R_leg;

        hierarchy["mov_arm_right"].transform.rotation = R_arm;
        hierarchy["mov_arm_left"].transform.rotation = R_arm;

        v = vec3{0, 0,0};
    }

    if (!falling && !diving){
        z =  (int) ((p.z-leg_z +g->step/2.0f - error) / g->step);
        p.z = (float) z*g->step + leg_z + g->step/2.0f;
    }


    hierarchy["body"].transform.translation = p;
}

void Player::jump(float initial_speed)
{
    v = v + vec3({0, 0, initial_speed});
    falling = true;
    jumping = false;
}


void Player::fall(float m)
{

    const float dt = timer.update(); // dt: Elapsed time between last frame
    const vec3 g = {0.0f,0.0f,-9.81f};
    const vec3 F = m*g;

    // Numerical integration
    v = v + dt*F;

}


void Player::move(float speed)
{

    bool up = check_up();
    bool water = check_water();
    int down = check_down();
    int ahead = check_ahead();
    int behind = check_behind();


    if (moving_up){
        if(ahead == 1){
            v.x = 0;
            v.y = 0;
        }else if (ahead == 0){
            v.x = speed * (float)cos(angle);
            v.y = speed * (float)-sin(angle);
        }
        else{
            v.x = speed/3.0f * (float)cos(angle);
            v.y = speed/3.0f * (float)-sin(angle);
        }
    }

    if (moving_down){
        if(behind == 1){
            v.x = 0;
            v.y = 0;
        }else if (behind == 0){
            v.x = speed*(float)-cos(angle);
            v.y = speed*(float) sin(angle);
        }else{
            v.x = speed/3.0f*(float)-cos(angle);
            v.y = speed/3.0f*(float) sin(angle);
        }
    }

    if (falling){
        if (down != 0 && down != 4 && v.z < 0){
            v.z = 0;
            falling = false;
        }
        if (up && v.z > 0){
            v.z = 0;
        }
    }

    p = p + v;
    if (down != 0 && down != 4){
        z =  (int) ((p.z-leg_z +g->step/2.0f - error) / g->step);
        p.z = (float) z*g->step + leg_z + g->step/2.0f;
    }

    if (down != 4){
        diving = false;
    }

    if (!falling && down == 0){
        falling = true;
    }

    if (down == 4){
        diving = true;
    }

}

void Player::turn(float speed)
{

    if (moving_left)
        angle -= speed;
    if (moving_right)
        angle += speed;
    hierarchy["body"].transform.rotation = rotation_from_axis_angle_mat3({0,0,-1}, angle);
}

void Player::create_player()
{

    // draw body
    mesh head = mesh_primitive_parallelepiped({0,0,0},{head_x,0,0},
                                              {0,head_y,0},{0,0,head_z});
    mesh body = mesh_primitive_parallelepiped({0,0,0},{size,0,0},
                                              {0,body_y,0},{0,0,body_z});
    mesh leg = mesh_primitive_parallelepiped({0,0,0},{size,0,0},
                                             {0,leg_y,0},{0,0,leg_z});
    mesh arm = mesh_primitive_parallelepiped({0,0,0},{size,0,0},
                                             {0,arm_y,0},{0,0,arm_z});
    mesh mov = mesh_primitive_sphere(scale * 0.01f);

    // Add texture
    const float x_a = 0.5f/4.0f;
    const float x_b = 0.5f/8.0f;

    const float y_a = 0.5f/2.0f;
    const float y_b = 0.5f/4.0f;

    // Texture
    player_texture =  create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/player.png"),
                                         GL_REPEAT, GL_REPEAT );

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


    mesh_drawable head_d = head;
    mesh_drawable body_d = body;
    mesh_drawable leg_d = leg;
    mesh_drawable mov_d = mov;
    mesh_drawable arm_d = arm;

    head_d.uniform.shading = {0.4f, 0.3f, 0.1f};
    body_d.uniform.shading = {0.4f, 0.3f, 0.1f};
    leg_d.uniform.shading = {0.4f, 0.3f, 0.1f};
    mov_d.uniform.shading = {0.4f, 0.3f, 0.1f};
    arm_d.uniform.shading = {0.4f, 0.3f, 0.1f};

    hierarchy.add(body_d, "body");

    hierarchy.add(mov_d, "mov_head", "body", {head_x/2.0f,head_y/2.0f,body_z});
    hierarchy.add(mov_d, "mov_leg_right", "body", {size/2.0f,body_y-leg_y/2.0f,0});
    hierarchy.add(mov_d, "mov_leg_left", "body", {size/2.0f,body_y/2.0f-leg_y/2.0f,0});
    hierarchy.add(mov_d, "mov_arm_right", "body", {size/2.0f,-arm_y/2.0f,body_z});
    hierarchy.add(mov_d, "mov_arm_left", "body", {size/2.0f,body_y + arm_y/2.0f,body_z});

    hierarchy.add(head_d, "head", "mov_head", {-head_x/2.0f-size/4.0f,-head_y/2.0f, 0});

    hierarchy.add(leg_d, "leg_right", "mov_leg_right", {-size/2.0f, -leg_y/2.0f, -leg_z});
    hierarchy.add(leg_d, "leg_left", "mov_leg_left", {-size/2.0f, -leg_y/2.0f, -leg_z});

    hierarchy.add(arm_d, "arm_right", "mov_arm_right", {-size/2.0f, -arm_y/2.0f, -arm_z});
    hierarchy.add(arm_d, "arm_left", "mov_arm_left", {-size/2.0f, -arm_y/2.0f, -arm_z});
}

int Player::check_ahead()
{
    float d = sqrt(size * size / 4.0f + body_y * body_y / 4.0f);
    float teta = atan((size/2.0f)/(body_y/2.0f));

    vec3 center = p + vec3{d * (float)sin(angle+teta),d*(float)cos(angle+teta), 0 };
    vec3 p1 = center + vec3{0, 0, -leg_z +g->step/2.0f + error};
    vec3 p2 = center + vec3{0, 0, 0};
    vec3 p3 = center + vec3{0, 0, dist_head - error};
    vec3 ahead = vec3{ distance * (float) cos(angle), distance * (float)-sin(angle), 0};
    
    int r;
    bool l = g->position_to_block(p1 + ahead) != 0 || g->position_to_block(p2 + ahead) != 0 || g->position_to_block(p3 + ahead) != 0;
    
    if (!l)
        r = 0;
    
    else if (g->position_to_block(p1 + ahead) == 4 || g->position_to_block(p2 + ahead) == 4 || g->position_to_block(p3 + ahead) == 4)
       r = 2;
    
    else
        r = 1;
    
    return r;
}

int Player::check_behind()
{
    float d = sqrt(size * size / 4.0f + body_y * body_y / 4.0f);
    float teta = atan((size/2.0f)/(body_y/2.0f));
    vec3 center = p + vec3{d * (float)sin(angle+teta),d*(float)cos(angle+teta), 0 };
    vec3 p1 = center + vec3{0, 0, -leg_z +g->step/2.0f + error};
    vec3 p2 = center + vec3{0, 0, 0};
    vec3 p3 = center + vec3{0, 0, dist_head - error};
    vec3 behind = vec3{ distance * (float)-cos(angle), distance * (float)sin(angle), 0};
    int r;
    bool l = g->position_to_block(p1 + behind) != 0 || g->position_to_block(p2 + behind) != 0 || g->position_to_block(p3 + behind) != 0;
    if (!l)
        r = 0;
    else if (g->position_to_block(p1 + behind) == 4 || g->position_to_block(p2 + behind) == 4 || g->position_to_block(p3 + behind) == 4)
        r = 2;
    else
        r = 1;
    return r;
}

int Player::check_down()
{
    //std::cout << "down debug" << std::endl;
    float d = sqrt(size * size / 4.0f + body_y * body_y / 4.0f);
    float teta = atan((size/2.0f)/(body_y/2.0f));
    vec3 center = p + vec3{d * (float)sin(angle+teta),d*(float)cos(angle+teta), 0 };
    vec3 p1 = center + vec3{0, 0, -leg_z +g->step/2.0f - error};
    return g->position_to_block(p1);
}

bool Player::check_up()
{
    float d = sqrt(size * size / 4.0f + body_y * body_y / 4.0f);
    float teta = atan((size/2.0f)/(body_y/2.0f));
    vec3 center = p + vec3{d * (float)sin(angle+teta),d*(float)cos(angle+teta), 0 };
    vec3 p1 = center + vec3{0, 0, dist_head + error};
    bool l = g->position_to_block(p1) != 0;
    return l;
}

bool Player::check_water()
{
    float d = sqrt(size * size / 4.0f + body_y * body_y / 4.0f);
    float teta = atan((size/2.0f)/(body_y/2.0f));
    vec3 center = p + vec3{d * (float)sin(angle+teta),d*(float)cos(angle+teta), 0 };
    vec3 p1 = center + vec3{0, 0, 0};
    bool l = g->position_to_block(p1) == 4;
    return l;
}

void Player::updateCamera(scene_structure &scene) {

    float speed_turn_camera = M_PI / (1.8f*16*27.0f) * 400 / ((float) fps);

    if (moving_left) {
        camera_angle += speed_turn_camera;
        scene.camera.apply_rotation(speed_turn_camera, 0, 0, 0);
    } else if (moving_right) {
        camera_angle -= speed_turn_camera;
        scene.camera.apply_rotation(-speed_turn_camera, 0, 0, 0);
    }

    if (moving_camera_up) {
        scene.camera.apply_rotation(0, 0.01f, 0, 0);
    } else if (moving_camera_down) {
        scene.camera.apply_rotation(0, -0.01f, 0, 0);
    }

    if (moving_camera_right) {
        camera_angle -= 0.05f;
        scene.camera.apply_rotation(0.05f, 0, 0, 0);
    } else if (moving_camera_left) {
        camera_angle += 0.05f;
        scene.camera.apply_rotation(-0.05f, 0, 0, 0);
    }

    float dc = sqrt(size * size / 4.0f + body_y * body_y / 4.0f);
    float teta = atan((size/2.0f)/(body_y/2.0f));
    float d = distance + g->step/1.4f;
    vec3 ahead = vec3{ d * (float) cos(angle), d * (float)-sin(angle), d};
    vec3 center = vec3{dc * (float)sin(angle+teta),dc*(float)cos(angle+teta), 0 };
    scene.camera.translation = - p - center- ahead;

    if (-scene.camera.orientation[0] >= 0 and -scene.camera.orientation[2] >= 0){
        angle = asin(scene.camera.orientation[5]);
    }else if(-scene.camera.orientation[0] >= 0 and -scene.camera.orientation[2] <= 0){
        angle = M_PI + asin(scene.camera.orientation[0]);
    }else if(-scene.camera.orientation[0] <= 0 and -scene.camera.orientation[2] <= 0){
        angle = -acos(-scene.camera.orientation[2]);
    }else{
        angle = asin(scene.camera.orientation[5]);
    }
    hierarchy["body"].transform.rotation = rotation_from_axis_angle_mat3({0,0,-1}, angle);
}

void Player::mouse_click(scene_structure& scene, GLFWwindow* window, int button, int action, int mods) {
    // Mouse click is used to select a position of the control polygon
    // ******************************************************************** //

    // Cursor coordinates
    const vec2 cursor = glfw_cursor_coordinates_window(window);
    bool mouse_click_right = (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS);

    if (mouse_click_right){
        // Create the 3D ray passing by the selected point on the screen
        const ray r = picking_ray(scene.camera, cursor);
        float d = g->step/2.0f;
        const vec3 b = r.p + r.u * d;

        while (d < 6 * g->step){
            const vec3 b = r.p + r.u * d;
            int box = g->position_to_block(b);
            if (box != 0 ){
                if (box != 4)
                    g->delete_block(b);
                break;
            }
            d += g->step/2.0f;
        }
    }
}