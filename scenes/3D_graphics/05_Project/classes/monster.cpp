#include "monster.hpp"

#include <cmath>

using namespace vcl;

void Monster::setup(float scale_, std::map<std::string,GLuint>& shaders, Grid* g_, Player* player_) {

    g =  g_;
    player = player_;
    // Scaling
    scale = scale_;
    head_x *= scale;
    head_y *= scale;
    head_z *= scale;
    body_x *= scale;
    body_y *= scale;
    body_z *= scale;
    leg_x *= scale;
    leg_y *= scale;
    leg_z *= scale;

    dist_feet = leg_z + g->step / 2.0f;
    dist_head = body_z + head_z;
    min_d = g->step * 18.0f;
    c_t = 0;

    find_player = false;
    bool find_start_place = false;

    distance = 0.8f * scale;
    error = leg_z / 10.0f;
    v = {0, 0, 0};

    std::uniform_int_distribution<int> start_x(0, g->Nx);
    std::uniform_int_distribution<int> start_y(0, g->Ny);
    generator.seed(10 * time(NULL));
    create_monster();

    x = start_x(generator);
    y = start_y(generator);
    z = g->surface_z[y][x];
    while (!find_start_place){
        if (g->blocks[z][y][x] == 0 && g->blocks[z+1][y][x] == 0 && g->blocks[z+2][y][x] == 0 && g->blocks[z+3][y][x] == 0 && g->blocks[z+4][y][x] == 0)
            find_start_place = true;
        else
            z += 1;
    }

    p = g->blocks_to_position(x, y, z) + vec3{0, 0, dist_feet};

    angle = 0;
    timer.scale = 0.1f;

    // Set the same shader for all the elements
    hierarchy.set_shader_for_all_elements(shaders["mesh"]);

    // Initialize helper structure to display the hierarchy skeleton
    hierarchy_visual_debug.init(shaders["segment_im"], shaders["mesh"]);

    hierarchy["body"].transform.translation = p;

}

void Monster::frame_draw(std::map<std::string, GLuint> &shaders, scene_structure &scene, gui_scene_structure gui_scene, int fps_) {

    if (fps_ == 0)
        fps = 10;
    else
        fps = fps_;

    timer.update();
    updatePosition(scene);

    // The default display
    if(gui_scene.surface) {
        glBindTexture(GL_TEXTURE_2D, monster_texture);
        draw(hierarchy, scene.camera, shaders["mesh"]);
        glBindTexture(GL_TEXTURE_2D, scene.texture_white);
    }

    hierarchy.update_local_to_global_coordinates();
    if(gui_scene.wireframe) // Display the hierarchy as wireframe
        draw(hierarchy, scene.camera, shaders["wireframe"]);

    if(gui_scene.skeleton) // Display the skeleton of the hierarchy (debug)
        hierarchy_visual_debug.draw(hierarchy, scene.camera);
}

void Monster::updatePosition(scene_structure &scene) {

    const float t = timer.t;
    c_t += t;
    float speed = g->step / (3*27.0f) * (400 / (float) fps);
    float speed_turn = M_PI / (3*16*27.0f) * (400 / (float) fps);

    std::uniform_real_distribution<float> at_l(-M_PI/2.0f, -M_PI/4.0f);
    std::uniform_real_distribution<float> at_r(M_PI/4.0f, M_PI/2.0f);
    std::uniform_int_distribution<int> s(0, 1);
    std::uniform_real_distribution<float> mt(250.0f * ((float)fps/400.0f), 500.0f * ((float)fps/400.0f));

    if (!jumping && !falling)
        close_player();

    if (!turning && !falling) {

        if ((int) (p.x / g->step) > g->Nx - 6 || (int) (p.y / g->step) > g->Ny - 6 ||
            (int) (p.x / g->step) < 6 || (int) (p.y / g->step) < 6) {
            turning = false;
            moving = true;
            if (angle_turn < 0)
                angle -= M_PI;
            else
                angle += M_PI;
        }
    }
    if (!find_player && !falling && !jumping){

        if (!turning) {
            // End of movement
            if (c_t > f_t_mov) {
                turning = true;
                moving = false;
                int side = s(generator);
                if (side == 0)
                    angle_turn = at_l(generator);
                else
                    angle_turn = at_r(generator);
                f_angle = angle + angle_turn;
            }
        }
        else if (angle_turn < 0) {

            if (angle >= f_angle && angle - speed_turn < f_angle){
                turning = false;
                moving = true;
                t_mov = mt(generator);
                f_t_mov = c_t + t_mov;
                hierarchy["body"].transform.rotation = rotation_from_axis_angle_mat3({0,0,-1}, angle);
               // hierarchy["mov_head"].transform.rotation = rotation_from_axis_angle_mat3({0,0,-1}, angle);
                l_angle = angle;

            }
        }else if (angle_turn >= 0){
            if (angle <= f_angle && angle + speed_turn > f_angle){
                turning = false;
                moving = true;
                t_mov = mt(generator);
                f_t_mov = c_t + t_mov;
                hierarchy["body"].transform.rotation = rotation_from_axis_angle_mat3({0,0,-1}, angle);
                l_angle = angle;
            }
        }

    }

    if (find_player){
        float a = p.x - player->p.x;
        float b = p.y - player->p.y;
        if (a <= 0 && b >= 0)
            angle = std::atan(std::abs(b/a));
        else if (a >= 0 && b >= 0)
            angle = M_PI/2.0f + std::atan(std::abs(a/b));
        else if (a >= 0 && b <= 0)
            angle = M_PI + std::atan(std::abs(b/a));
        else if (a <= 0 && b <= 0)
            angle = 3*M_PI/2.0f + std::atan(std::abs(a/b));
        if (jumping)
            jump(scale * 0.04f * (400 / (float) fps));
        else if (falling)
            fall(scale * 8500.0f * (400 / (float) fps));
        move(speed, scene);
        hierarchy["body"].transform.rotation = rotation_from_axis_angle_mat3({0,0,-1}, angle);
    }

    if (!find_player){
        if (moving) {
            if (jumping)
                jump(scale * 0.04f * (400 / (float) (2.3f * fps)));
            else if (falling)
                fall(scale * 8500.0f * (400 / (float) fps));
            move(speed, scene);
        }
        else if (turning){
            turn(speed_turn);
        }
    }


    if (!falling){
        z =  (int) ((p.z-leg_z +g->step/2.0f - error) / g->step);
        p.z = (float) z*g->step;
    }

    mat3 const R_head = rotation_from_axis_angle_mat3({0, 0, 1}, 0.8f * std::sin(3 * 3.14f * (t - 0.6f)));
    hierarchy["mov_head"].transform.rotation = R_head;
    hierarchy["body"].transform.translation = p;
    //scene.camera.translation = -p;

}

void Monster::jump(float initial_speed)
{

    v = v + vec3({0, 0, initial_speed});
    falling = true;
    jumping = false;
}


void Monster::fall(float m)
{
    const float dt = timer.update(); // dt: Elapsed time between last frame
    const vec3 g = {0.0f,0.0f,-9.81f};
    const vec3 F = m*g;

    // Numerical integration
    v = v + dt*F;
}


void Monster::move(float speed, scene_structure &scene)
{
    //scene.camera.translation = -p;
    bool up = check_up();
    bool down = check_down(scene);
    bool ahead = check_ahead(scene);

    v.x = speed * (float)std::cos(angle);
    v.y = speed * (float)-std::sin(angle);

    if(ahead and !falling){
        jumping = true;
    }else if (ahead){
        v.x = 0;
        v.y = 0;
    }

    if (falling){
        if (down && v.z <= 0){
            v.z = 0;
            falling = false;
        }
        if (up && v.z >= 0){
            v.z = 0;
        }
    }

    p = p + v;
    if (down){
        z =  (int) ((p.z-leg_z +g->step/2.0f - error) / g->step);
        p.z = (float) z*g->step + leg_z + g->step/2.0f;
    }

    if (!falling && !down){
        falling = true;
    }

}

void Monster::turn(float speed)
{
   // std::cout << "angle = " << angle << std::endl;
   // std::cout << "angle = " << angle << std::endl;
    if (angle_turn < 0)
        angle -= speed;
    else
        angle += speed;
    //hierarchy["mov_head"].transform.rotation = rotation_from_axis_angle_mat3({0,0,-1}, angle);
}

void Monster::create_monster()
{

    // draw body
    mesh head = mesh_primitive_parallelepiped({0,0,0},{head_x,0,0},
                                              {0,head_y,0},{0,0,head_z});
    mesh body = mesh_primitive_parallelepiped({0,0,0},{body_x,0,0},
                                              {0,body_y,0},{0,0,body_z});
    mesh leg = mesh_primitive_parallelepiped({0,0,0},{leg_x,0,0},
                                             {0,leg_y,0},{0,0,leg_z});
    mesh mov = mesh_primitive_sphere(scale * 0.01f);


    // Add texture
    const float x_a = 1.0f/8.0f;
    const float x_b = x_a/2.0f;

    const float y_a = 1.0f/4.0f;
    const float y_b = 1.0f/8.0f;
    const float y_c = (0.5f - y_b)/2.0f;

    // Texture
    monster_texture =  create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/creeper4.png"),
                                          GL_REPEAT, GL_REPEAT );

    head.texture_uv = {
            {2*x_a,1*y_a}, {2*x_a,0}, {3*x_a,0}, {3*x_a,1*y_a},
            {1*x_a,2*y_a}, {1*x_a,1*y_a}, {2*x_a,1*y_a}, {2*x_a,2*y_a},
            {2*x_a,2*y_a}, {2*x_a,1*y_a}, {3*x_a,1*y_a}, {3*x_a,2*y_a},
            {4*x_a,2*y_a}, {4*x_a,1*y_a}, {3*x_a,1*y_a}, {3*x_a,2*y_a},
            {0,2*y_a}, {0,1*y_a}, {1*x_a,1*y_a}, {1*x_a,2*y_a},
            {1*x_a,1*y_a}, {1*x_a,0}, {2*x_a,0}, {1*x_a,1*y_a},
    };

    body.texture_uv = {
            {3*x_a+x_b,2*y_a+y_b}, {3*x_a+x_b,2*y_a}, {4*x_a+x_b,2*y_a}, {4*x_a+x_b,2*y_a+y_b},
            {2*x_a+x_b,4*y_a}, {2*x_a+x_b,2*y_a+y_b}, {3*x_a+x_b,2*y_a+y_b}, {3*x_a+x_b,4*y_a},
            {3*x_a+x_b,4*y_a}, {3*x_a+x_b,2*y_a+y_b}, {4*x_a,2*y_a+y_b}, {4*x_a,4*y_a},
            {5*x_a,4*y_a}, {5*x_a,2*y_a+y_b}, {4*x_a,2*y_a+y_b}, {4*x_a,4*y_a},
            {2*x_a,4*y_a}, {2*x_a,2*y_a+y_b}, {2*x_a+x_b,2*y_a+y_b}, {2*x_a+x_b,4*y_a},
            {2*x_a+x_b,2*y_a+y_b}, {2*x_a+x_b,2*y_a}, {3*x_a+x_b,2*y_a}, {3*x_a+x_b,2*y_a+y_b},
    };

    leg.texture_uv = {
            {1*x_a,2*y_a+y_b}, {1*x_a,2*y_a}, {1*x_a+x_b,2*y_a}, {1*x_a+x_b,2*y_a+y_b},
            {x_b,2*y_a+y_b+y_c}, {x_b,2*y_a+y_b}, {x_a,2*y_a+y_b}, {x_a,2*y_a+y_b+y_c},
            {x_a,2*y_a+y_b+y_c}, {x_a,2*y_a+y_b}, {x_a+x_b,2*y_a+y_b}, {x_a+x_b,2*y_a+y_b+y_c},
            {2*x_a,2*y_a+y_b+y_c}, {2*x_a,2*y_a+y_b}, {x_a+x_b,2*y_a+y_b}, {x_a+x_b,2*y_a+y_b+y_c},
            {0,2*y_a+y_b+y_c}, {0,2*y_a+y_b}, {x_b,2*y_a+y_b}, {x_b,2*y_a+y_b+y_c},
            {1*x_b,2*y_a+y_b}, {1*x_b,2*y_a}, {1*x_a,2*y_a}, {1*x_a,2*y_a+y_b},
    };

    hierarchy.add(body, "body");
    hierarchy.add(mov, "mov_head", "body", {body_x/2.0f,body_y/2.0f,body_z});

    hierarchy.add(head, "head", "mov_head", {-head_x/2.0f,-head_y/2.0f, 0});

    hierarchy.add(leg, "leg_front", "body", {body_x, 0, -leg_z});
    hierarchy.add(leg, "leg_behind", "body", {-leg_x, 0, -leg_z});

}


bool Monster::check_ahead(scene_structure &scene)
{
    //std::cout << "ahead debug" << std::endl;

    float d = sqrt(body_x * body_x / 4.0f + body_y * body_y / 4.0f);
    float teta = atan((body_x/2.0f)/(body_y/2.0f));
    vec3 center = hierarchy["body"].transform.translation + vec3{d * (float)sin(angle+teta),d*(float)cos(angle+teta), 0 };
    vec3 p1 = center + vec3{0, 0, -leg_z + g->step + error};
    vec3 p2 = center + vec3{0, 0, g->step};
    vec3 p3 = center + vec3{0, 0, dist_head - error};

    /*
    std::cout << "p = " << p << std::endl;
    std::cout << "center = " << center << std::endl;
    std::cout << "p1 = " <<  p1 << std::endl;
    std::cout << "p2 = " <<  p2 << std::endl;
    std::cout << "p3 = " <<  p3 << std::endl;
    std::cout << "step = " << g->step << std::endl;
    std::cout << "z standing = " << (int) (p1[2] / g->step) << std::endl;
    std::cout << falling << std::endl;
    */
    vec3 ahead = vec3{ distance * (float) cos(angle), distance * (float)-sin(angle), 0};
   // scene.camera.translation = -center;
    //std::cout << "leg_z = " << leg_z << std::endl;
    //std::cout << "p1b = " << g->position_to_block(p1 + ahead) << std::endl;
    //std::cout << "p2b = " << g->position_to_block(p2 + ahead) << std::endl;
    //std::cout << "p3b = " << g->position_to_block(p3 + ahead) << std::endl;
    bool l = g->position_to_block(p1 + ahead) != 0 || g->position_to_block(p2 + ahead) != 0 || g->position_to_block(p3 + ahead) != 0;

    return l;
}

bool Monster::check_down(scene_structure &scene)
{
    //std::cout << "down debug" << std::endl;
    float d = sqrt(body_x * body_x / 4.0f + body_y * body_y / 4.0f);
    float teta = atan((body_x/2.0f)/(body_y/2.0f));
    vec3 center = hierarchy["body"].transform.translation + vec3{d * (float)sin(angle+teta),d*(float)cos(angle+teta), 0 };

    vec3 p1 = center + vec3{0, 0, -leg_z - error};

    //std::cout << "p = " << p << std::endl;
    //std::cout << "center = " << center << std::endl;
    //std::cout << "step = " << g.step << std::endl;
    //std::cout << "z standing = " << (p1[2] / g.step) << std::endl;
    //std::cout << "leg_z = " << leg_z << std::endl;
    //std::cout << g->position_to_block(p1) << std::endl;
    bool l = g->position_to_block(p1) != 0;
    return l;
}

bool Monster::check_up()
{
    float d = sqrt(body_x * body_x / 4.0f + body_y * body_y / 4.0f);
    //std::cout << "d = " << d << std::endl;
    float teta = atan((body_x/2.0f)/(body_y/2.0f));
    vec3 center = hierarchy["body"].transform.translation + vec3{d * (float)sin(angle+teta),d*(float)cos(angle+teta), 0 };
    vec3 p1 = center + vec3{0, 0, dist_head + error};
    bool l = g->position_to_block(p1) != 0;
    return l;
}

void Monster::close_player() {
    float d = sqrt((player->p.x - p.x) * (player->p.x - p.x) + (player->p.y - p.y) * (player->p.y - p.y) + (player->p.z - p.z) * (player->p.z - p.z));
    find_player = d < min_d;
}
