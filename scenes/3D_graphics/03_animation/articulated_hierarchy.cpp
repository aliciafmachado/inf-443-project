
#include "articulated_hierarchy.hpp"


#ifdef SCENE_ARTICULATED_HIERARCHY


using namespace vcl;



void scene_model::setup_data(std::map<std::string,GLuint>& shaders, scene_structure& , gui_structure& )
{
    // exemple();
    pinguim(1.0f);
    // cat1();

    // Set the same shader for all the elements
    hierarchy.set_shader_for_all_elements(shaders["mesh"]);

    // Initialize helper structure to display the hierarchy skeleton
    hierarchy_visual_debug.init(shaders["segment_im"], shaders["mesh"]);

    timer.scale = 0.5f;
}




void scene_model::frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_structure& gui)
{
    timer.update();
    set_gui();

    // Current time
    const float t = timer.t;
    std::cout << t << std::endl;
    /** *************************************************************  **/
    /** Compute the (animated) transformations applied to the elements **/
    /** *************************************************************  **/
    draw_pinguim(t);
    // draw_exemple(t);
    // draw_cat1(t);

    /** ********************* **/
    /** Display the hierarchy **/
    /** ********************* **/

    if(gui_scene.surface) // The default display
        draw(hierarchy, scene.camera);

    if(gui_scene.wireframe) // Display the hierarchy as wireframe
        draw(hierarchy, scene.camera, shaders["wireframe"]);

    if(gui_scene.skeleton) // Display the skeleton of the hierarchy (debug)
        hierarchy_visual_debug.draw(hierarchy, scene.camera);
}

void scene_model::cat1(){

    const float radius_head1 = 0.24f;
    const float radius_head2 = 0.20f;
    const float radius_head3 = 0.20f;
    const float radius_body1 = 0.22f;
    const float radius_body2 = 0.24f;
    const float radius_body3 = 0.60f;
    const float size_tail = 0.4f;
    const float size_leg = 0.54f;

    mesh_drawable body = mesh_primitive_ellipsoid(radius_body1, radius_body2, radius_body3,{0,0,0}, 40, 40);
    mesh_drawable head = mesh_primitive_ellipsoid(radius_head1, radius_head2, radius_head3,{0,0,0}, 40, 40);

    mesh_drawable eye = mesh_drawable( mesh_primitive_sphere(0.03f, {0.0f, 0, 0}, 40, 40));
    eye.uniform.color = {0,0,0};

    mesh_drawable nose = mesh_drawable( mesh_primitive_sphere(0.02f, {0.0f, 0, 0}, 40, 40));
    nose.uniform.color = {0,0,0};

    mesh_drawable ear = mesh_drawable( mesh_primitive_cone(0.06f, {0,0,0},
                                                           {0.08f,0.14f,0}, 40, 40));
    ear.uniform.color = {0,0,0};

    mesh_drawable bigode = mesh_drawable(mesh_primitive_cylinder(0.001f, {0,0,0},
                                                                 {0.3f,0.0f,0}, 40, 40));
    bigode.uniform.color = {0,0,0};

    mesh_drawable tail_mov = mesh_primitive_sphere(0.001f);
    mesh_drawable tail = mesh_primitive_ellipsoid(0.07f, 0.04f, size_tail,{0,0,0}, 40, 40);

    mesh_drawable leg_mov = mesh_primitive_sphere(0.001f);
    mesh_drawable leg = mesh_primitive_cylinder(0.05f, {0,0,0},{0,-size_leg,0}, 40, 40);

    mesh_drawable feet_mov = mesh_primitive_sphere(0.001f);
    mesh_drawable feet = mesh_primitive_parallelepiped({0, 0, 0}, {0.1f, 0, 0}, {0, 0.008f, 0}, {0, 0, 0.14f});


    hierarchy.add(body, "body");

    hierarchy.add(tail_mov, "tail_mov", "body", {0.0f, 0.1f, -radius_body3+0.05f});
    hierarchy.add(tail, "tail", "tail_mov", size_tail * vec3{0.0f, 0, -0.6f });

    hierarchy.add(leg_mov, "leg_mov_left_head", "body", {radius_body1-0.14f, 0.1f-radius_body2/2.0f, radius_body3-0.2f});
    hierarchy.add(leg, "leg_left_head", "leg_mov_left_head", size_tail * vec3{0.0f, 0, 0 });
    hierarchy.add(leg_mov, "leg_mov_right_head", "body", {-radius_body1+0.14f, 0.1f-radius_body2/2.0f, radius_body3-0.2f});
    hierarchy.add(leg, "leg_right_head", "leg_mov_right_head", size_tail * vec3{0.0f, 0, 0 });
    hierarchy.add(leg_mov, "leg_mov_left_tail", "body", {radius_body1-0.14f, 0.1f-radius_body2/2.0f, -radius_body3+0.2f});
    hierarchy.add(leg, "leg_left_tail", "leg_mov_left_tail", size_tail * vec3{0.0f, 0, 0 });
    hierarchy.add(leg_mov, "leg_mov_right_tail", "body", {-radius_body1+0.14f, 0.1f-radius_body2/2.0f, -radius_body3+0.2f});
    hierarchy.add(leg, "leg_right_tail", "leg_mov_right_tail", size_tail * vec3{0.0f, 0, 0 });

    hierarchy.add(feet_mov, "feet_mov_left_head", "leg_left_head", {0, -size_leg, 0});
    hierarchy.add(feet, "feet_left_head", "feet_mov_left_head", vec3{-0.05f, 0, -0.05f});
    hierarchy.add(feet_mov, "feet_mov_right_head", "leg_right_head", {0, -size_leg, 0});
    hierarchy.add(feet, "feet_right_head", "feet_mov_right_head", vec3{-0.05f, 0, -0.05f});
    hierarchy.add(feet_mov, "feet_mov_left_tail", "leg_left_tail", {0, -size_leg, 0});
    hierarchy.add(feet, "feet_left_tail", "feet_mov_left_tail", vec3{-0.05f, 0, -0.05f});
    hierarchy.add(feet_mov, "feet_mov_right_tail", "leg_right_tail", {0, -size_leg, 0});
    hierarchy.add(feet, "feet_right_tail", "feet_mov_right_tail", vec3{-0.05f, 0, -0.05f});

    hierarchy.add(head, "head", "body", {0.0f, radius_body2-0.08f, radius_body3-0.08f});

    hierarchy.add(eye, "eye_left", "head" , radius_head1 * vec3( 0.39f, 0.2f, 0.7f));
    hierarchy.add(eye, "eye_right", "head", radius_head1 * vec3(-0.39f, 0.2f, 0.7f));

    hierarchy.add(nose, "nose", "head", radius_head1 * vec3(0, 0.1f, 0.77f));

    hierarchy.add(ear, "ear_left", "head" ,  {{ 0.5f* radius_head1, 0.74f* radius_head2, 0},
                                              {1,0,0.2f, 0,1,0, 0.2f,0,1}});
    hierarchy.add(ear, "ear_right", "head", {{ -0.5f* radius_head1, 0.74f* radius_head2, 0},
                                             {-1,0,0.2f, 0,1,0, 0.2f,0,1}});
    hierarchy.add(bigode, "bigode_right1", "head", {vec3(0.06f, -0.14f* radius_head2, -0.01f + radius_head3),
                                             {1,0.32f,0, 0.32f,1,0, 0,0,1}});
    hierarchy.add(bigode, "bigode_right2", "head", {vec3(0.06f, -0.14f* radius_head2, -0.01f + radius_head3),
                                                    {1,0,0, 0,1,0, 0,0,1}});
    hierarchy.add(bigode, "bigode_right3", "head", {vec3(0.06f, -0.14f* radius_head2, -0.01f + radius_head3),
                                                    {1,-0.32f,0, -0.32f,1,0, 0,0,1}});
    hierarchy.add(bigode, "bigode_left1", "head", {vec3(-0.06f, -0.14f* radius_head2, -0.01f + radius_head3),
                                                    {-1,0.32f,0, 0.32f,1,0, 0,0,1}});
    hierarchy.add(bigode, "bigode_left2", "head", {vec3(-0.06f, -0.14f* radius_head2, -0.01f + radius_head3),
                                                   {-1,0,0, 0,1,0, 0,0,1}});
    hierarchy.add(bigode, "bigode_left3", "head", {vec3(-0.06f, -0.14f* radius_head2, -0.01f + radius_head3),
                                                   {-1,-0.32f,0, -0.32f,1,0, 0,0,1}});
}

void scene_model::pinguim(float scale){

    const float r_body1 = 0.22f * scale;
    const float r_body2 = 0.45f * scale;
    const float r_body3 = 0.24f * scale;
    const float l_arm1 = 0.50f * scale;
    const float l_arm2 = 0.18f * scale;
    const float r_head = 0.17f * scale;
    const float r_eye = 0.03f * scale;
    const float r_nose = 0.05f * scale;
    const float l_nose = 0.13f * scale;

    mesh_drawable body = mesh_primitive_ellipsoid(r_body1, r_body2, r_body3,{0,0,0}, 40, 40);
    mesh_drawable head = mesh_primitive_sphere(r_head, {0.0f, 0.0f, 0.0f}, 40, 40);

    mesh_drawable eye = mesh_drawable( mesh_primitive_sphere(r_eye, {0.0f, 0, 0}, 40, 40));
    eye.uniform.color = {0,0,0};

    mesh_drawable nose = mesh_drawable( mesh_primitive_cone(r_nose, {0,0,0},
                                                            {0,l_nose,0}, 40, 40));
    nose.uniform.color = {1,0.6,0};

    mesh_drawable arm1 = mesh_primitive_quad({0,-0.2f,0.0f}, {0,0.2f,0}, {l_arm1,0.2f,0}, {l_arm1,-0.2f,0});
    mesh_drawable arm2 = mesh_primitive_quad({0,-0.2f,0.0f}, {0,0.2f,0}, {l_arm2,0.1f,0}, {l_arm2,-0.1f,0});

    mesh_drawable wing1_mov = mesh_primitive_sphere(0.01f);
    mesh_drawable wing2_mov = mesh_primitive_sphere(0.01f);

    hierarchy.add(body, "body");
    hierarchy.add(head, "head", "body", {0, r_body2 + 0.02f, r_body3- 0.07f});

    hierarchy.add(eye, "eye_left", "head" , r_head * vec3( 1/3.0f, 0.73f, 5.5/10.0f));
    hierarchy.add(eye, "eye_right", "head", r_head * vec3(-1/3.0f, 0.73f, 5.5/10.0f));

    hierarchy.add(nose, "nose", "head", {0.0f, r_head-0.02f, 0});

    hierarchy.add(wing1_mov, "wing1_right", "body", {0.1f, 0, 0});
    hierarchy.add(arm1, "arm1_right", "wing1_right");
    hierarchy.add(wing2_mov, "wing2_right", "arm1_right", {l_arm1, 0, 0});
    hierarchy.add(arm2, "arm2_right", "wing2_right");

    hierarchy.add(wing1_mov, "wing1_left", "body", {{-0.1f, 0, 0}, {-1,0,0, 0,1,0, 0,0,1}});
    hierarchy.add(arm1, "arm1_left", "wing1_left");
    hierarchy.add(wing2_mov, "wing2_left", "arm1_left", {{l_arm1, 0, 0}});
    hierarchy.add(arm2, "arm2_left", "wing2_left");

    std::cout << "Penguin ready" << std::endl;
}
void scene_model::exemple(){

    const float radius_body = 0.25f;
    const float radius_arm = 0.05f;
    const float length_arm = 0.2f;

    // The geometry of the body is a sphere
    mesh_drawable body = mesh_drawable( mesh_primitive_sphere(radius_body, {0,0,0}, 40, 40));

    // Geometry of the eyes: black spheres
    mesh_drawable eye = mesh_drawable(mesh_primitive_sphere(0.05f, {0,0,0}, 20, 20));
    eye.uniform.color = {0,0,0};

    // Shoulder part and arm are displayed as cylinder
    mesh_drawable shoulder = mesh_primitive_cylinder(radius_arm, {0,0,0}, {-length_arm,0,0});
    mesh_drawable arm = mesh_primitive_cylinder(radius_arm, {0,0,0}, {-length_arm/1.5f,-length_arm/1.0f,0});

    // An elbow displayed as a sphere
    mesh_drawable elbow = mesh_primitive_sphere(0.055f);

    // Build the hierarchy:
    // Syntax to add element
    // hierarchy.add(visual_element, element_name, parent_name, (opt)[translation, rotation])
    hierarchy.add(body, "body");

    // Eyes positions are set with respect to some ratio of the
    hierarchy.add(eye, "eye_left", "body" , radius_body * vec3( 1/3.0f, 1/2.0f, 1/1.5f));
    hierarchy.add(eye, "eye_right", "body", radius_body * vec3(-1/3.0f, 1/2.0f, 1/1.5f));

    // Set the left part of the body arm: shoulder-elbow-arm
    hierarchy.add(shoulder, "shoulder_left", "body", {-radius_body+0.05f,0,0}); // extremity of the spherical body
    hierarchy.add(elbow, "elbow_left", "shoulder_left", {-length_arm,0,0});     // place the elbow the extremity of the "shoulder cylinder"
    hierarchy.add(arm, "arm_bottom_left", "elbow_left");                        // the arm start at the center of the elbow

    // Set the right part of the body arm: similar to the left part excepted a symmetry is applied along x direction for the shoulder
    hierarchy.add(shoulder, "shoulder_right", "body",     {{radius_body-0.05f,0,0}, {-1,0,0, 0,1,0, 0,0,1}/*Symmetry*/ } );
    hierarchy.add(elbow, "elbow_right", "shoulder_right", {-length_arm,0,0});
    hierarchy.add(arm, "arm_bottom_right", "elbow_right");

}

void scene_model::draw_exemple(float t){
    // The body oscillate along the z direction
    hierarchy["body"].transform.translation = {0,0,0.2f*(1+std::sin(2*3.14f*t))};

    // Rotation of the shoulder around the y axis
    mat3 const R_shoulder = rotation_from_axis_angle_mat3({0,1,0}, std::sin(2*3.14f*(t-0.4f)) );
    // Rotation of the arm around the y axis (delayed with respect to the shoulder)
    mat3 const R_arm = rotation_from_axis_angle_mat3({0,1,0}, std::sin(2*3.14f*(t-0.6f)) );
    // Symmetry in the x-direction between the left/right parts
    mat3 const Symmetry = {-1,0,0, 0,1,0, 0,0,1};

    // Set the rotation to the elements in the hierarchy
    hierarchy["shoulder_left"].transform.rotation = R_shoulder;
    hierarchy["arm_bottom_left"].transform.rotation = R_arm;

    hierarchy["shoulder_right"].transform.rotation = Symmetry*R_shoulder; // apply the symmetry
    hierarchy["arm_bottom_right"].transform.rotation = R_arm; //note that the symmetry is already applied by the parent element

    hierarchy.update_local_to_global_coordinates();

}

void scene_model::draw_cat1(float t){

    // Symmetry in the x-direction between the left/right parts
    mat3 const Symmetry = {-1,0,0, 0,1,0, 0,0,1};

    mat3 const R_tail = rotation_from_axis_angle_mat3({1,0,0}, std::sin(2*3.14f*t/2.0f) );
    hierarchy["tail_mov"].transform.rotation = R_tail;

    mat3 const R_leg1 = rotation_from_axis_angle_mat3({1,0,0}, 0.7f * std::sin(2*3.14f*(0-0.7f)) );
    hierarchy["leg_mov_left_head"].transform.rotation = R_leg1;

    mat3 const R_leg2 = rotation_from_axis_angle_mat3({1,0,0}, 0.7f * std::sin(2*3.14f*(0-0.4f)) );
    hierarchy["leg_mov_left_tail"].transform.rotation = R_leg2;

    mat3 const R_leg3 = rotation_from_axis_angle_mat3({1,0,0}, 0.7f * std::sin(2*3.14f*(0-0.05f)) );
    hierarchy["leg_mov_right_head"].transform.rotation = R_leg3;

    mat3 const R_leg4 = rotation_from_axis_angle_mat3({1,0,0}, 0.7f * std::sin(2*3.14f*(0-0.85f)) );
    hierarchy["leg_mov_right_tail"].transform.rotation = R_leg4;

    hierarchy.update_local_to_global_coordinates();

}

void scene_model::draw_pinguim(float t){

    // Symmetry in the x-direction between the left/right parts
    mat3 const Symmetry = {-1,0,0, 0,1,0, 0,0,1};

    mat3 const R_arm1 = rotation_from_axis_angle_mat3({0,1,0}, 0.7f * std::sin(2*3.14f*(2* t-0.4f)) );
    hierarchy["wing1_right"].transform.rotation = R_arm1;
    hierarchy["wing1_left"].transform.rotation = Symmetry * R_arm1;

    mat3 const R_arm2 = rotation_from_axis_angle_mat3({0,1,0}, 1.2f * std::sin(2*3.14f*(2* t-0.4f)) );
    hierarchy["wing2_right"].transform.rotation = R_arm2;
    hierarchy["wing2_left"].transform.rotation = R_arm2;

    mat3 const R_head = rotation_from_axis_angle_mat3({1,0,0}, 0.5f * std::sin(2*3.14f*(t-0.6f)) );
    hierarchy["head"].transform.rotation = R_head;

    hierarchy.update_local_to_global_coordinates();

}
void scene_model::set_gui()
{
    ImGui::Text("Display: "); ImGui::SameLine();
    ImGui::Checkbox("Wireframe", &gui_scene.wireframe); ImGui::SameLine();
    ImGui::Checkbox("Surface", &gui_scene.surface);     ImGui::SameLine();
    ImGui::Checkbox("Skeleton", &gui_scene.skeleton);   ImGui::SameLine();

    ImGui::Spacing();
    ImGui::SliderFloat("Time", &timer.t, timer.t_min, timer.t_max);
    ImGui::SliderFloat("Time scale", &timer.scale, 0.1f, 3.0f);

}





#endif

