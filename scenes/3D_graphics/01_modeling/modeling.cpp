
#include "modeling.hpp"


#ifdef SCENE_3D_GRAPHICS

// Add vcl namespace within the current one - Allows to use function from vcl library without explicitely preceeding their name with vcl::
using namespace vcl;

// Some constants

// -> Distance
const float dist_tree_tree = 0.4f + 0.4f + 0.3f;
const float dist_tree_champignons = 0.4f + 0.2f + 0.1f;
const float dist_champignons_champignons = 0.2f + 0.2f + 0.15f;

// -> Quantity
const int num_trees = 30;
const int num_champignons = 35;
const int num_bill = 24;

// Functions
mesh create_skybox();
mesh create_tree_foliage(float radius, float height, float z_offset);
mesh create_cone(float radius, float height, float z_offset);
mesh create_cylinder(float radius, float height);
mesh create_terrain(const gui_scene_structure& structure);
float evaluate_terrain_z(float u, float v, const gui_scene_structure& gui_scene);
vec3 evaluate_terrain(float u, float v, const gui_scene_structure& gui_scene);

void update_tree_position(std::vector<vcl::vec3> &tree_position, const gui_scene_structure& gui_scene);
void update_champignons_position(std::vector<vcl::vec3> &champignons_position, std::vector<vcl::vec3> tree_position,
                                 const gui_scene_structure& gui_scene);
void update_bill_grass_position(std::vector<vcl::vec3> champignons_position, std::vector<vcl::vec3> tree_position,
                                std::vector<vcl::vec3> &bill_position, const gui_scene_structure& gui_scene);

// Cardinal spline
static size_t index_at_value(float t, vcl::buffer<vec3t> const& v);

static vec3 cardinal_spline_interpolation(float t, float t0, float t1, float t2, float t3, const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, float K);
static vec3 cardinal_spline_interpolation_derivative(float t, float t0, float t1, float t2, float t3, const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, float K);
static vec3 cardinal_spline_interpolation(float t, vcl::buffer<vec3t> keyframes);
static vec3 cardinal_spline_interpolation_derivative(float t, vcl::buffer<vec3t> keyframes);

/** This function is called before the beginning of the animation loop
    It is used to initialize all part-specific data */
void scene_model::setup_data(std::map<std::string, GLuint>& shaders, scene_structure& scene, gui_structure& )
{
    // Create visual terrain surface
    terrain = create_terrain(gui_scene);
    terrain.uniform.color = {0.6f,0.85f,0.5f};
    terrain.uniform.shading.specular = 0.0f; // non-specular terrain material
    texture_terrain_id = create_texture_gpu(image_load_png("scenes/3D_graphics/02_texture/assets/grass.png"),
                                            GL_REPEAT, GL_REPEAT);

    // Tree
    tronc = create_cylinder(0.12f, 1.3f);
    tronc.uniform.color = {0.8f,0.52f,0.25f};
    tronc.uniform.shading.specular = 0.0f;
    foliage = create_tree_foliage(0.4f, 0.4f, 0.9f);
    foliage.uniform.color = {0.0f,0.5f,0.0f};
    foliage.uniform.shading.specular = 0.0f;
    update_tree_position(tree_position, gui_scene);

    // Champignons
    tronc_c = create_cylinder(0.05f, 0.18f);
    tronc_c.uniform.color = {0.76f,0.76f,0.81f};
    tronc_c.uniform.shading.specular = 0.0f;
    foliage_c = create_cone(0.13f, 0.06f, 0.16f);
    foliage_c.uniform.color = {0.5f,0.0f,0.0f};
    foliage_c.uniform.shading.specular = 0.0f;
    update_champignons_position(champignons_position, tree_position, gui_scene);

    // skybox
    skybox = create_skybox();
    skybox.uniform.shading = {1,0,0};
    skybox.uniform.transform.rotation = rotation_from_axis_angle_mat3({1,0,0},-3.014f/2.0f);
    skybox_texture_id = create_texture_gpu(image_load_png("scenes/3D_graphics/02_texture/assets/skybox.png"),
                                         GL_REPEAT, GL_REPEAT );

    // billboard
    bill_texture_id = create_texture_gpu(image_load_png("scenes/3D_graphics/02_texture/assets/billboard_grass.png"),
                                         GL_REPEAT, GL_REPEAT );
    bill_flower_texture_id = create_texture_gpu(image_load_png("scenes/3D_graphics/02_texture/assets/billboard_redflowers.png"),
                                                GL_REPEAT, GL_REPEAT );
    update_bill_grass_position(champignons_position, tree_position, bill_position, gui_scene);
    mesh surface_cpu;
    surface_cpu.position     = {{-0.2f,0,0}, { 0.2f,0,0}, { 0.2f, 0.4f,0}, {-0.2f, 0.4f,0}};
    surface_cpu.texture_uv   = {{0,1}, {1,1}, {1,0}, {0,0}};
    surface_cpu.connectivity = {{0,1,2}, {0,2,3}};

    surface = surface_cpu;
    surface.uniform.shading = {1,0,0}; // set pure ambiant component (no diffuse, no specular) - allow to only see the color of the textur

    // Setup initial camera mode and position
    scene.camera.camera_type = camera_control_spherical_coordinates;
    scene.camera.scale = 10.0f;
    scene.camera.apply_rotation(0,0,0,1.2f);

    // Pinguim
    create_trajectory();
    create_penguin(1.0f);

    // Set the same shader for all the elements
    hierarchy.set_shader_for_all_elements(shaders["mesh"]);

    // Initialize helper structure to display the hierarchy skeleton
    hierarchy_visual_debug.init(shaders["segment_im"], shaders["mesh"]);
    timer.scale = 0.5f;
}

/** This function is called at each frame of the animation loop.
    It is used to compute time-varying argument and perform data data drawing */
void scene_model::frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, gui_structure& )
{
    set_gui();

    glEnable( GL_POLYGON_OFFSET_FILL ); // avoids z-fighting when displaying wireframe

    // Before displaying a textured surface: bind the associated texture id
    glBindTexture(GL_TEXTURE_2D, texture_terrain_id);
    draw(terrain, scene.camera, shaders["mesh"]);
    if( gui_scene.wireframe ){ // wireframe if asked from the GUI
        glPolygonOffset( 1.0, 1.0 );
        draw(terrain, scene.camera, shaders["wireframe"]);
    }
    // After the surface is displayed it is safe to set the texture id to a white image
    //  Avoids to use the previous texture for another object
    glBindTexture(GL_TEXTURE_2D, scene.texture_white);

    // draw trees
    for (auto & position : tree_position) {
        tronc.uniform.transform.translation   = position;
        foliage.uniform.transform.translation = position;
        draw(tronc, scene.camera, shaders["mesh"]);
        draw(foliage, scene.camera, shaders["mesh"]);
    }

    // draw champignons
    for (auto & position : champignons_position) {
        tronc_c.uniform.transform.translation   = position;
        foliage_c.uniform.transform.translation = position;
        draw(tronc_c, scene.camera, shaders["mesh"]);
        draw(foliage_c, scene.camera, shaders["mesh"]);
    }

    // draw skybox
    glBindTexture(GL_TEXTURE_2D,skybox_texture_id);
    skybox.uniform.transform.scaling = 150.0f;
    skybox.uniform.transform.translation = scene.camera.camera_position() + vec3(0,0,-50.0f);
    draw(skybox, scene.camera, shaders["mesh"]);
    glBindTexture(GL_TEXTURE_2D,scene.texture_white);

    // draw penguin
    draw_trajectory(shaders, scene);
    draw_pinguim(shaders, scene);

    if( gui_scene.wireframe ){ // wireframe if asked from the GUI
        glPolygonOffset( 1.0, 1.0 );
        draw(terrain, scene.camera, shaders["wireframe"]);
        for (auto & position : tree_position) {
            tronc.uniform.transform.translation   = position;
            foliage.uniform.transform.translation = position;
            draw(tronc, scene.camera, shaders["wireframe"]);
            draw(foliage, scene.camera, shaders["wireframe"]);
        }
        for (auto & position : champignons_position) {
            tronc_c.uniform.transform.translation   = position;
            foliage_c.uniform.transform.translation = position;
            draw(tronc_c, scene.camera, shaders["wireframe"]);
            draw(foliage_c, scene.camera, shaders["wireframe"]);
        }
    }
    // ********************************************************** //
    // ********************* BILLBOARD GRASS ******************** //
    // ********************************************************** //
    // Enable use of alpha component as color blending for transparent elements
    //  new color = previous color + (1-alpha) current color
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Disable depth buffer writing
    //  - Transparent elements cannot use depth buffer
    //  - They are supposed to be display from furest to nearest elements
    glDepthMask(false);

    glBindTexture(GL_TEXTURE_2D, bill_texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // avoids sampling artifacts
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // avoids sampling artifacts

    // Display a billboard always facing the camera direction
    // ********************************************************** //
    for (int i=0; i < (int) bill_position.size() / 2; i++) {
        surface.uniform.transform.rotation = scene.camera.orientation;
        surface.uniform.transform.translation =  bill_position[i] + vec3{0.0,0,0.0f};
        draw(surface, scene.camera, shaders["mesh"]);
        if(gui_scene.wireframe)
            draw(surface, scene.camera, shaders["wireframe"]);
    }

    // ********************************************************** //
    // ********************* BILLBOARD FLOWER ******************* //
    // ********************************************************** //
    glBindTexture(GL_TEXTURE_2D, bill_flower_texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // avoids sampling artifacts
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // avoids sampling artifacts

    // Display a billboard always facing the camera direction
    // ********************************************************** //
    for (int i=(int) bill_position.size() / 2 + 1; i < (int) bill_position.size(); i++) {
        surface.uniform.transform.rotation = scene.camera.orientation;
        surface.uniform.transform.translation =  bill_position[i] + vec3{0.0,0,0.0f};
        draw(surface, scene.camera, shaders["mesh"]);
        if(gui_scene.wireframe)
            draw(surface, scene.camera, shaders["wireframe"]);
    }

    glBindTexture(GL_TEXTURE_2D, scene.texture_white);
    glDepthMask(true);

}

void scene_model::draw_trajectory(std::map<std::string,GLuint>& shaders, scene_structure& scene) {
    timer.update();
    const float t = timer.t;

    // ********************************************* //
    // Compute interpolated position at time t
    // ********************************************* //
    const int idx = index_at_value(t, keyframes);

    // Assume a closed curve trajectory
    const size_t N = keyframes.size();

    const vec3 p = cardinal_spline_interpolation(t, keyframes);
    trajectory.add_point(p);

    // Draw sphere at each keyframe position
    if(gui_scene.display_keyframe) {
        for (size_t k = 0; k < N; ++k) {
            const vec3 &p_keyframe = keyframes[k].p;
            sphere.uniform.transform.translation = p_keyframe;
            draw(sphere, scene.camera, shaders["mesh"]);
        }
    }


    // Draw segments between each keyframe
    if(gui_scene.display_polygon) {
        for (size_t k = 0; k < N - 1; ++k) {
            const vec3 &pa = keyframes[k].p;
            const vec3 &pb = keyframes[k + 1].p;

            segment_drawer.uniform_parameter.p1 = pa;
            segment_drawer.uniform_parameter.p2 = pb;
            segment_drawer.draw(shaders["segment_im"], scene.camera);
        }
    }

    // Draw moving point trajectory
    trajectory.draw(shaders["curve"], scene.camera);

    // Draw selected sphere in red
    if( picked_object!=-1 )
    {
        const vec3& p_keyframe = keyframes[picked_object].p;
        sphere.uniform.color = vec3(1,0,0);
        sphere.uniform.transform.scaling = 0.06f;
        sphere.uniform.transform.translation = p_keyframe;
        segment_drawer.draw(shaders["mesh"], scene.camera);
        sphere.uniform.color = vec3(1,1,1);
        sphere.uniform.transform.scaling = 0.05f;
    }
}

void scene_model::mouse_click(scene_structure& scene, GLFWwindow* window, int , int , int )
{
    // Mouse click is used to select a position of the control polygon
    // ******************************************************************** //

    // Cursor coordinates
    const vec2 cursor = glfw_cursor_coordinates_window(window);

    // Check that the mouse is clicked (drag and drop)
    const bool mouse_click_left  = glfw_mouse_pressed_left(window);
    const bool key_shift = glfw_key_shift_pressed(window);

    // Check if shift key is pressed
    if(mouse_click_left && key_shift)
    {
        // Create the 3D ray passing by the selected point on the screen
        const ray r = picking_ray(scene.camera, cursor);

        // Check if this ray intersects a position (represented by a sphere)
        //  Loop over all positions and get the intersected position (the closest one in case of multiple intersection)
        const size_t N = keyframes.size();
        picked_object = -1;
        float distance_min = 0.0f;
        for(size_t k=0; k<N; ++k)
        {
            const vec3 c = keyframes[k].p;
            const picking_info info = ray_intersect_sphere(r, c, 0.1f);

            if( info.picking_valid ) // the ray intersects a sphere
            {
                const float distance = norm(info.intersection-r.p); // get the closest intersection
                if( picked_object==-1 || distance<distance_min ){
                    distance_min = distance;
                    picked_object = k;
                }
            }
        }
    }

}

void scene_model::mouse_move(scene_structure& scene, GLFWwindow* window)
{

    const bool mouse_click_left  = glfw_mouse_pressed_left(window);
    const bool key_shift = glfw_key_shift_pressed(window);
    if(mouse_click_left && key_shift && picked_object!=-1)
    {
        // Translate the selected object to the new pointed mouse position within the camera plane
        // ************************************************************************************** //

        // Get vector orthogonal to camera orientation
        const mat4 M = scene.camera.camera_matrix();
        const vec3 n = {M(0,2),M(1,2),M(2,2)};

        // Compute intersection between current ray and the plane orthogonal to the view direction and passing by the selected object
        const vec2 cursor = glfw_cursor_coordinates_window(window);
        const ray r = picking_ray(scene.camera, cursor);
        vec3& p0 = keyframes[picked_object].p;
        const picking_info info = ray_intersect_plane(r,n,p0);

        // translate the position
        p0 = info.intersection;

    }
}

void scene_model::create_trajectory() {
    // Initial Keyframe data
    //keyframe_position = {{0,0,0}, {3,0,0}, {3,3,0}, {6,3,0}, {9,3,0}, {9,0,0}, {12,0,0}, {12,-3,0}, {0,-3,0}, {0,0,0}, {3,0,0}, {3,3,0}};
    const size_t N = 9;
    float cont = 0.0f;
    for (size_t i=0; i<N+3; i++) {
        const float u = 3*sin(i /(1.0*N) * 2*M_PI );
        const float v = 3*cos(i /(1.0*N) * 2*M_PI );
        keyframes.push_back({{u, v, 2}, cont});
        cont += 1.0f;
    }

    // Set timer bounds
    timer.t_min = keyframes[1].t;                   // first time of the keyframe
    timer.t_max = keyframes[keyframes.size()-2].t;  // last time of the keyframe
    timer.t = timer.t_min;

    // Prepare the visual elements
    sphere = mesh_primitive_sphere();
    sphere.uniform.color = {1,1,1};
    sphere.uniform.transform.scaling = 0.05f;

    segment_drawer.init();

    trajectory = curve_dynamic_drawable(100); // number of steps stored in the trajectory
    trajectory.uniform.color = {0,0,1};

    picked_object=-1;
}

void scene_model::draw_pinguim(std::map<std::string,GLuint>& shaders, scene_structure& scene){

    penguin_timer.update();
    const float t = timer.t;
    const float pt = penguin_timer.t;

    hierarchy["body"].transform.translation =  {0,0,0.06f*(1-std::sin(2*3.14f*pt))};

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

    const vec3 p = cardinal_spline_interpolation(t, keyframes);
    hierarchy["body"].transform.translation += p;
    const vec3 d = cardinal_spline_interpolation_derivative(t, keyframes);
    hierarchy["body"].transform.rotation = rotation_between_vector_mat3({0,1,0}, d );

    hierarchy.update_local_to_global_coordinates();

    if(gui_scene.surface) // The default display
        draw(hierarchy, scene.camera);

    if(gui_scene.wireframe) // Display the hierarchy as wireframe
        draw(hierarchy, scene.camera, shaders["wireframe"]);

    if(gui_scene.skeleton) // Display the skeleton of the hierarchy (debug)
        hierarchy_visual_debug.draw(hierarchy, scene.camera);

}

void scene_model::create_penguin(float scale){

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

    penguin_timer.scale = 0.8f;
    std::cout << "Penguin ready" << std::endl;
}

mesh create_skybox()
{
    const vec3 p000 = {-1,-1,-1};
    const vec3 p001 = {-1,-1, 1};
    const vec3 p010 = {-1, 1,-1};
    const vec3 p011 = {-1, 1, 1};
    const vec3 p100 = { 1,-1,-1};
    const vec3 p101 = { 1,-1, 1};
    const vec3 p110 = { 1, 1,-1};
    const vec3 p111 = { 1, 1, 1};

    mesh skybox;

    skybox.position = {
            p000, p100, p110, p010,
            p010, p110, p111, p011,
            p100, p110, p111, p101,
            p000, p001, p010, p011,
            p001, p101, p111, p011,
            p000, p100, p101, p001
    };


    skybox.connectivity = {
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
    skybox.texture_uv = {
            {u1,v1}, {u2,v1}, {u2,v2}, {u1,v2},
            {u1,v2}, {u2,v2}, {u2,v3}, {u1,v3},
            {u2,v1}, {u2,v2}, {u3,v2}, {u3,v1},
            {u1,v1}, {u0,v1}, {u1,v2}, {u0,v2},
            {u4,v1}, {u3,v1}, {u3,v2}, {u4,v2},
            {u1,v1}, {u2,v1}, {u2,v0}, {u1,v0}
    };


    return skybox;

}

mesh create_cylinder(float radius, float height){

    // number of triangles
    const size_t N = 40;

    //Initial position of a cylinder
    const float x_0 = 0.0f;
    const float y_0 = 0.0f;
    const float pi = 3.14159f;

    mesh cylinder; // temporary cylinder storage (CPU only)
    cylinder.position.resize(2*N);

    // Fill cylinder geometry
    float x, y;
    for(size_t ku=0; ku<N; ku += 2)
    {
        x = x_0 + radius * cos(360*ku*pi/(N*180));
        y = y_0 + radius * sin(360*ku*pi/(N*180));
        cylinder.position[ku] = {x,y,0};
        cylinder.position[ku+1] = {x,y,height};
    }

    // Generate triangle organization
    const unsigned int Ns = N;
    uint3 triangle;
    for(unsigned int ku=0; ku<Ns-2; ++ku)
    {
        triangle = {ku, ku+1, ku+2};
        cylinder.connectivity.push_back(triangle);
    }

    triangle = {0, Ns-2, Ns-1};
    cylinder.connectivity.push_back(triangle);

    triangle = {Ns-1, 0, 1};
    cylinder.connectivity.push_back(triangle);

    return cylinder;
}

mesh create_tree_foliage(float radius, float height, float z_offset)
{
    mesh m = create_cone(radius, height, z_offset);
    m.push_back( create_cone(radius, height, z_offset+height/2.1f) );
    m.push_back( create_cone(radius, height, z_offset+2*height/2.2f) );

    return m;
}

void update_champignons_position(std::vector<vcl::vec3> &champignons_position, std::vector<vcl::vec3> tree_position, const gui_scene_structure& gui_scene) {

    // min distance between
    const float dist_min_tree = dist_tree_champignons / 10;
    const float dist_min_champignons = dist_champignons_champignons / 10;
    bool dist;

    std::uniform_real_distribution<float> distrib(0.0,1.0);
    std::default_random_engine generator;

    float r_x, r_y;

    for (int i = 0; i < num_champignons; ++i) {
        dist = false;
        while (!dist) {

            dist = true;
            r_x = distrib(generator);
            r_y = distrib(generator);

            // check trees
            for (unsigned int j = 0; j < tree_position.size(); ++j) {
                auto calc = (float) sqrt(pow(r_x - (tree_position[j][0] / 20 + 0.5), 2) + pow(r_y - (tree_position[j][1] / 20 + 0.5), 2));
                if (calc < dist_min_tree) {
                    dist = false;
                }
            }

            // check champignons
            for (unsigned int j = 0; j < champignons_position.size(); ++j) {
                auto calc = (float) sqrt(pow(r_x - (champignons_position[j][0] / 20 + 0.5), 2) + pow(r_y - (champignons_position[j][1] / 20 + 0.5), 2));
                if (calc < dist_min_champignons) {
                    dist = false;
                }
            }
        }
        const float x = 20 * (r_x - 0.5f);
        const float y = 20 * (r_y - 0.5f);
        const float z = evaluate_terrain_z(r_x, r_y, gui_scene);
        champignons_position.push_back({x, y, z});
    }
}

void update_bill_grass_position(std::vector<vcl::vec3> champignons_position, std::vector<vcl::vec3> tree_position,
                                std::vector<vcl::vec3> &bill_position, const gui_scene_structure& gui_scene){
    // min distance between
    const float dist_min_tree = dist_tree_champignons / 10;
    const float dist_min_champignons = dist_champignons_champignons / 10;
    const float dist_min_bill = 0.05f;
    bool dist;

    std::uniform_real_distribution<float> distrib(0.0,1.0);
    std::default_random_engine generator;
    float r_x, r_y;

    for (int i = 0; i < num_bill; ++i) {
        dist = false;
        while (!dist) {

            dist = true;
            r_x = distrib(generator);
            r_y = distrib(generator);

            // check trees
            for (unsigned int j = 0; j < tree_position.size(); ++j) {
                auto calc = (float) sqrt(pow(r_x - (tree_position[j][0] / 20 + 0.5), 2) + pow(r_y - (tree_position[j][1] / 20 + 0.5), 2));
                if (calc < dist_min_tree) {
                    dist = false;
                }
            }

            // check champignons
            for (unsigned int j = 0; j < champignons_position.size(); ++j) {
                auto calc = (float) sqrt(pow(r_x - (champignons_position[j][0] / 20 + 0.5), 2) + pow(r_y - (champignons_position[j][1] / 20 + 0.5), 2));
                if (calc < dist_min_champignons) {
                    dist = false;
                }
            }

            // check billboard
            for (unsigned int j = 0; j < bill_position.size(); ++j) {
                auto calc = (float) sqrt(pow(r_x - (bill_position[j][0] / 20 + 0.5), 2) + pow(r_y - (bill_position[j][1] / 20 + 0.5), 2));
                if (calc < dist_min_bill) {
                    dist = false;
                }
            }
        }
        const float x = 20 * (r_x - 0.5f);
        const float y = 20 * (r_y - 0.5f);
        const float z = evaluate_terrain_z(r_x, r_y, gui_scene);
        bill_position.push_back({x, y, z});
    }

}
void update_tree_position(std::vector<vcl::vec3> &tree_position, const gui_scene_structure& gui_scene){

    // min distance between each tree is foliage radius + 0.3f
    const float dist_min_norm = 0.11f;
    bool dist;

    std::uniform_real_distribution<float> distrib(0.0,1.0);
    std::default_random_engine generator;

    float r_x, r_y;
    for (int i = 0; i < num_trees; ++i) {
        dist = false;
        while (!dist) {
            dist = true;
            r_x = distrib(generator);
            r_y = distrib(generator);
            for (unsigned int j = 0; j < tree_position.size(); ++j) {
                auto calc = (float) sqrt(pow(r_x - (tree_position[j][0] / 20 + 0.5), 2) + pow(r_y - (tree_position[j][1] / 20 + 0.5), 2));
                if (calc < dist_min_norm) {
                    dist = false;
                }
            }
        }
        const float x = 20 * (r_x - 0.5f);
        const float y = 20 * (r_y - 0.5f);
        const float z = evaluate_terrain_z(r_x, r_y, gui_scene);
        tree_position.push_back({x, y, z});
    }
}

mesh create_cone(float radius, float height, float z_offset){

    // number of triangles in the base
    const size_t N = 40;

    //Initial position of a cone
    const float x_0 = 0.0f;
    const float y_0 = 0.0f;
    const float pi = 3.14159f;

    mesh cone; // temporary cone storage (CPU only)
    cone.position.resize(N+2);

    // Fill cone geometry
    float x, y;

    // Position N and N+1 are reserved to the center points
    cone.position[N] = {x_0, y_0, z_offset};
    cone.position[N+1] = {x_0, y_0, z_offset + height};

    for(size_t ku=0; ku<N; ++ku)
    {
        x = x_0 + radius * cos(360*ku*pi/(N*180));
        y = y_0 + radius * sin(360*ku*pi/(N*180));

        cone.position[ku] = {x, y, z_offset};
    }

    // Generate triangle organization
    const unsigned int Ns = N+2;
    uint3 triangle;
    for(unsigned int ku=0; ku<Ns-2; ++ku)
    {
        triangle = {Ns-2, ku, ku+1};
        cone.connectivity.push_back(triangle);
        triangle = {Ns-1, ku, ku+1};
        cone.connectivity.push_back(triangle);
    }

    triangle = {Ns-1, Ns-3, 0};
    cone.connectivity.push_back(triangle);

    triangle = {Ns-2, Ns-3, 0};
    cone.connectivity.push_back(triangle);

    return cone;
}

// Evaluate height of the terrain for any (u,v) \in [0,1]
float evaluate_terrain_z(float u, float v, const gui_scene_structure& gui_scene)
{
    vec2 u0[] = {{0.0f, 0.0f}, {0.5f,0.5f}, {0.2f,0.7f}, {0.8f,0.7f}};
    float h0[] = {3.0f,-1.5f,1.0f,2.0f};
    float sigma0[] = {0.5f,0.15f,0.2f,0.2f};
    float d = 0;
    float z = 0;

    // get gui parameters
    const float scaling = gui_scene.scaling;
    const int octave = gui_scene.octave;
    const float persistency = gui_scene.persistency;
    const float height = gui_scene.height;

    // Evaluate Perlin noise
    const float noise = perlin(scaling*u, scaling*v, octave, persistency);

    for (unsigned int i = 0; i < sizeof(sigma0)/sizeof(sigma0[0]); i++){
        d = norm(vec2(u,v)-u0[i])/sigma0[i];
        z += h0[i]*std::exp(-d*d);
    }
    return z*noise*height;
}

// Evaluate 3D position of the terrain for any (u,v) \in [0,1]
vec3 evaluate_terrain(float u, float v, const gui_scene_structure& gui_scene)
{
    const float x = 20*(u-0.5f);
    const float y = 20*(v-0.5f);
    const float z = evaluate_terrain_z(u,v, gui_scene);

    return {x,y,z};
}

// Generate terrain mesh
mesh create_terrain(const gui_scene_structure& gui_scene)
{
    // Number of samples of the terrain is N x N
    const size_t N = 100;

    mesh terrain; // temporary terrain storage (CPU only)
    terrain.position.resize(N*N);
    terrain.texture_uv.resize(N*N);

    // Fill terrain geometry
    for(size_t ku=0; ku<N; ++ku)
    {
        for(size_t kv=0; kv<N; ++kv)
        {
            // Compute local parametric coordinates (u,v) \in [0,1]
            const float u = ku/(N-1.0f);
            const float v = kv/(N-1.0f);



            // Compute coordinates
            terrain.position[kv+N*ku] = evaluate_terrain(u,v, gui_scene);
            terrain.texture_uv[kv+N*ku] = {(float) 20 * u, (float) 20 * v};
        }
    }

    // Fill terrain texture

    // Generate triangle organization
    //  Parametric surface with uniform grid sampling: generate 2 triangles for each grid cell
    const unsigned int Ns = N;
    for(unsigned int ku=0; ku<Ns-1; ++ku)
    {
        for(unsigned int kv=0; kv<Ns-1; ++kv)
        {
            const unsigned int idx = kv + N*ku; // current vertex offset

            const uint3 triangle_1 = {idx, idx+1+Ns, idx+1};
            const uint3 triangle_2 = {idx, idx+Ns, idx+1+Ns};

            terrain.connectivity.push_back(triangle_1);
            terrain.connectivity.push_back(triangle_2);
        }
    }

    return terrain;
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

    ImGui::Text("Display: "); ImGui::SameLine();
    ImGui::Checkbox("keyframe", &gui_scene.display_keyframe); ImGui::SameLine();
    ImGui::Checkbox("polygon", &gui_scene.display_polygon);

    if( ImGui::Button("Print Keyframe") )
    {
        std::cout<<"keyframe_position={";
        for(size_t k=0; k<keyframes.size(); ++k)
        {
            const vec3& p = keyframes[k].p;
            std::cout<< "{"<<p.x<<"f,"<<p.y<<"f,"<<p.z<<"f}";
            if(k<keyframes.size()-1)
                std::cout<<", ";
        }
        std::cout<<"}"<<std::endl;
    }

    const float penguin_time_scale_min = 0.1f;
    const float penguin_time_scale_max = 3.0f;
    ImGui::SliderFloat("Penguin Time", &penguin_timer.t, penguin_timer.t_min, penguin_timer.t_max);
    ImGui::SliderFloat("Penguin Time scale", &penguin_timer.scale, penguin_time_scale_min, penguin_time_scale_max);
}


static size_t index_at_value(float t, vcl::buffer<vec3t> const& v)
{
    const size_t N = v.size();
    assert(v.size()>=2);
    assert(t>=v[0].t);
    assert(t<v[N-1].t);

    size_t k=0;
    while( v[k+1].t<t )
        ++k;
    return k;
}

static vec3 cardinal_spline_interpolation(float t, vcl::buffer<vec3t> keyframes) {
    const int idx = index_at_value(t, keyframes);

    // Cardianl Spline interpolation
    const float t0 = keyframes[idx-1].t; // = t_{i-1}
    const float t1 = keyframes[idx  ].t; // = t_i
    const float t2 = keyframes[idx+1].t; // = t_{i+1}
    const float t3 = keyframes[idx+2].t; // = t_{i+2}

    const vec3& p0 = keyframes[idx-1].p; // = p_{i-1}
    const vec3& p1 = keyframes[idx  ].p; // = p_i
    const vec3& p2 = keyframes[idx+1].p; // = p_{i+1}
    const vec3& p3 = keyframes[idx+2].p; // = p_{i+2}

    const float K = 0.5;
    const vec3 p = cardinal_spline_interpolation(t,t0,t1,t2,t3,p0,p1,p2,p3,K);

    return p;
}

static vec3 linear_interpolation(float t, float t1, float t2, const vec3& p1, const vec3& p2)
{
    const float alpha = (t-t1)/(t2-t1);
    const vec3 p = (1-alpha)*p1 + alpha*p2;

    return p;
}

static vec3 cardinal_spline_interpolation(float t, float t0, float t1, float t2, float t3, const vec3& p0,
                                          const vec3& p1, const vec3& p2, const vec3& p3, float K)
{
    const float s = (t-t1)/(t2-t1);

    const vec3 d1 = 2 * K * (p2 - p0)/(t2-t0);
    const vec3 d2 = 2 * K * (p3 - p1)/(t3-t1);
    const vec3 p = (2*s*s*s-3*s*s+1)*p1 + (s*s*s-2*s*s+s)*d1 + (-2*s*s*s+3*s*s)*p2 + (s*s*s-s*s)*d2;

    return p;
}

static vec3 cardinal_spline_interpolation_derivative(float t, float t0, float t1, float t2, float t3, const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, float K) {
    const float s = (t-t1)/(t2-t1);
    const float dsdt = 1.0f/(t2-t1);
    K = 0.25f;
    const vec3 d1 = 2 * K * ((t3-t1)/(t2-t0)) * (p2-p0);
    const vec3 d2 = 2 * K * ((t2-t0)/(t3-t1)) * (p3-p1);

    const vec3 d = (6*s*s*dsdt - 6*s*dsdt)*p1 + (3*s*s*dsdt - 4*s*dsdt + dsdt)*d1 + (-6*s*s*dsdt + 6*s*dsdt)*p2 + (3*s*s*dsdt - 2*s*dsdt)*d2;

    return d;
}

static vec3 cardinal_spline_interpolation_derivative(float t, vcl::buffer<vec3t> keyframes) {
    const size_t idx = index_at_value(t, keyframes);

    const float t0 = keyframes[idx-1].t; // = t_{i-1}
    const float t1 = keyframes[idx  ].t; // = t_i
    const float t2 = keyframes[idx+1].t; // = t_{i+1}
    const float t3 = keyframes[idx+2].t; // = t_{i+2}

    const vec3& p0 = keyframes[idx-1].p; // = p_{i-1}
    const vec3& p1 = keyframes[idx  ].p; // = p_i
    const vec3& p2 = keyframes[idx+1].p; // = p_{i+1}
    const vec3& p3 = keyframes[idx+2].p; // = p_{i+2}

    const float K = 0.5;
    const vec3 p = cardinal_spline_interpolation_derivative(t,t0,t1,t2,t3,p0,p1,p2,p3, K);
    /*
    for (int i = 0; i < 3; ++ i){
        std::cout << "p[" << i << "] = " << p[i] << std::endl;
    }
     */
    return p;
}

#endif

