#include "skybox.hpp"

using namespace vcl;

mesh create_skybox(float l);

void Skybox::setup() {

    skybox_texture =  create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/skybox_minecraft.png"),
                                         GL_REPEAT, GL_REPEAT );

    skybox = create_skybox(1.0f);
    skybox.uniform.shading = {1,0,0};
    skybox.uniform.transform.translation = {1, 1, 1};
    skybox.uniform.transform.scaling = 150.0f;
}


void Skybox::frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, bool wireframe) {

    glBindTexture(GL_TEXTURE_2D, skybox_texture);
    skybox.uniform.transform.translation = scene.camera.camera_position() + vec3(0,0,-25.0f);
    draw(skybox, scene.camera, shaders["mesh"]);
    if (wireframe) {
        draw(skybox, scene.camera, shaders["wireframe"]);
    }
    glBindTexture(GL_TEXTURE_2D, scene.texture_white);
}

mesh create_skybox(float l)
{
    mesh block;

    const vec3 p000 = {-l,-l,-l};
    const vec3 p001 = {-l,-l, l};
    const vec3 p010 = {-l, l,-l};
    const vec3 p011 = {-l, l, l};
    const vec3 p100 = { l,-l,-l};
    const vec3 p101 = { l,-l, l};
    const vec3 p110 = { l, l,-l};
    const vec3 p111 = { l, l, l};

    block.position = {
            p111, p011, p001, p101,
            p101, p001, p000, p100,
            p011, p001, p000, p010,
            p111, p110, p101, p100,
            p110, p010, p000, p100,
            p111, p011, p010, p110
    };

    block.connectivity = {
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
    block.texture_uv = {
            {u2,v1}, {u2,v0}, {u1,v0}, {u1,v1},
            {u1,v1}, {u0,v1}, {u0,v2}, {u1,v2},
            {u3,v1}, {u4,v1}, {u4,v2}, {u3,v2},
            {u2,v1}, {u2,v2}, {u1,v1}, {u1,v2},
            {u2,v2}, {u2,v3}, {u1,v3}, {u1,v2},
            {u2,v1}, {u3,v1}, {u3,v2}, {u2,v2},
    };

    return block;

}