#pragma once

// GUI parameters
struct gui_scene_structure
{
    bool surface = false;
    bool wireframe = false;
    bool skeleton = true;

    bool generate_surface = false;
    bool generate_dungeons = false;
    bool generate_trees = false;
    bool generate_river = false;

    float se = 0.2f;
    float height = 0.05f;
    float scaling = 1.0f;
    int octave = 7;
    float persistency = 0.4f;
    float frequency = 2.0f;
    float min_noise = 0.65f;
};

