#pragma once

#include "main/scene_base/base.hpp"

class Skybox {
public:
    void setup();
    void frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, bool wireframe);
private:
    GLuint skybox_texture;
    vcl::mesh_drawable skybox;
};