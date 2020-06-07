#include "grid.hpp"

using namespace vcl;

#define BLOCK_TYPES 7
#define NONE 0
#define GRASS 1
#define DIRTY 2
#define STONE 3
#define WATER 4
#define WOOD 5
#define LEAVE 6
#define SAND 7
#define WATER_HEIGHT 56
#define MIN_WATER 50
#define QTD_FLOWERS 20

float evaluate_terrain_z(float u, float v, const gui_scene_structure& gui_scene);

void Grid::setup()
{
    gen.seed(time(NULL));
    block = create_block(step / 2, false);
    block_simple = create_block(step / 2, true);
    block_simple.uniform.shading = {0.4f, 0.4f, 0.8f};
    block.uniform.shading = {0.4f, 0.4f, 0.8f};
    billboard = create_billboard(step / 2);
    billboard.uniform.shading = {0.0f, 0.0f, 1.0f};

    block_textures = new GLuint[BLOCK_TYPES];
    block_textures[0] = create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/grass.png"),
                                             GL_REPEAT, GL_REPEAT );
    block_textures[1] = create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/dirty.png"),
                                             GL_REPEAT, GL_REPEAT );
    block_textures[2] = create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/rock.png"),
                                             GL_REPEAT, GL_REPEAT );
    block_textures[3] = create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/water.png"),
                                             GL_REPEAT, GL_REPEAT );
    block_textures[4] = create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/wood.png"),
                                             GL_REPEAT, GL_REPEAT );
    block_textures[5] = create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/leave.png"),
                                             GL_REPEAT, GL_REPEAT );
    block_textures[6] = create_texture_gpu(image_load_png("scenes/3D_graphics/05_Project/texture/sand.png"),
                                             GL_REPEAT, GL_REPEAT );
}

/*bool sort_function(std::pair<int, float> i, std::pair<int, float> j) {
    return i.second > j.second;
}

void sort_order(std::vector<vec3> &tr, vec3 cam_pos) {
    std::vector<std::pair<int, float>> sorted;
    for(int i = 0; i < tr.size(); i++) {
        float dist = pow(tr[i][0] - cam_pos[0], 2) + pow(tr[i][1] - cam_pos[1], 2) 
        + pow(tr[i][2] - cam_pos[2], 2);

        sorted.push_back(std::pair<int, float> (i, dist));
    }

    std::sort(sorted.begin(), sorted.end(), sort_function);

    float aux;
    std::vector<vec3> tr_copy(tr);
    for(int i = 0; i < tr.size(); i++) {
        tr[i] = tr_copy[sorted[i].first];
    }
}*/

void Grid::frame_draw(std::map<std::string,GLuint>& shaders, scene_structure& scene, bool wireframe, int fps) {

    feed_translations();

    for(int i = 1; i <= BLOCK_TYPES; i++) {
        if(translations[i].size() != 0) {
            if(i == STONE) {
                block.uniform.shading = {0.5f, 0.3f, 0.7f};
            }
            auto vec = translations[i];
            int n = vec.size();
            vec3* v = &vec[0];
            if(i == 7) {
                draw_instanced(block_simple, scene.camera, shaders["mesh_array"], block_textures[i-1], v, n);
            }
            else {
                draw_instanced(block, scene.camera, shaders["mesh_array"], block_textures[i-1], v, n);
                /*if(wireframe)
                    draw_instanced(block, scene.camera, shaders["mesh_array"], block_textures[i-1], v, n);*/
            }
            if(i == STONE) {
                block.uniform.shading = {0.4f, 0.4f, 0.8f};
            }
            glBindTexture(GL_TEXTURE_2D, scene.texture_white);
        }
    }

    //print billboards - flowers and possibly water
    /*int count = 0;
    while(count < QTD_FLOWERS) {
        billboard.uniform.transform.rotation = scene.camera.orientation;
        billboard.uniform.transform.translation = {0.25f,0,-0.5f};
        if()
    }*/
    /*if (wireframe)
        draw(block, scene.camera, shaders["wireframe"]);*/
}
void Grid::create_grid(gui_scene_structure gui, std::default_random_engine g)
{
    gen = g;

    // Inialize vector with blocks type 1
    blocks = std::vector<std::vector<std::vector<int>>>(Nz, std::vector<std::vector<int>>(Ny, std::vector<int>(Nx, 0)));
    draw_blocks = std::vector<std::vector<std::vector<bool>>>(Nz, std::vector<std::vector<bool>>(Ny, std::vector<bool>(Nx, false)));
    surface_z = std::vector<std::vector<int>>(Ny, std::vector<int>(Nx, Nz_surface));

    for (int k=0; k<Nz; ++k){
        for (int j=0; j<Ny; ++j){
            for (int i=0; i<Nx; ++i){
                if (k <= Nz_dungeon)
                    blocks[k][j][i] = STONE;
                if (k > Nz_dungeon && k < Nz_surface)
                    blocks[k][j][i] = DIRTY;
                if (k == Nz_surface)
                    blocks[k][j][i] = GRASS;
                if (i == 0 || j == 0 || k == 0) {
                    draw_blocks[k][j][i] = true;
                }
                else if (i == (Nx-1) || j == (Ny-1) || k == (Nz_surface)) {
                    draw_blocks[k][j][i] = true;
                }
            }
        }
    }
    if( gui.generate_surface )
        generate_surface(gui);
    if( gui.generate_dungeons )
        generate_dungeons(gui);
    if( gui.generate_river )
        generate_lake(gui);
    
    create_enter_dungeon(gui);

    if( gui.generate_trees )
        generate_trees(gui);

    // Avoid drawing hidden blocks
    for (int k=1; k<Nz_dungeon-1; ++k) {
        for (int j = 1; j < Ny-1; ++j) {
            for (int i = 1; i < Nx-1; ++i) {
                if (blocks[k+1][j][i] == NONE || blocks[k-1][j][i] == NONE ||
                    blocks[k][j+1][i] == NONE || blocks[k][j-1][i] == NONE ||
                    blocks[k][j][i+1] == NONE || blocks[k][j][i-1] == NONE){}
                else {
                    draw_blocks[k][j][i] = false;
                }

            }
        }
    }
    feed_translations();
}

void Grid::feed_translations() {
    // create translations vector
    for(int i = 1; i <= BLOCK_TYPES; i++) {
        translations[i] = {};
    }
    int count = 0;

    for (int k=0; k<Nz; ++k){
        for (int j=0; j<Ny; ++j){
            for (int i=0; i<Nx; ++i){
                if (blocks[k][j][i] != 0 && draw_blocks[k][j][i]) {
                    //block.uniform.transform.translation   = {i * step, j * step, k * step};
                    translations[blocks[k][j][i]].push_back({i * step, j * step, k * step});
                    count++; // TODO readd billboards
                }
            }
        }
    }
}

void Grid::generate_surface(gui_scene_structure gui)
{

    // Fill terrain geometry
    for(size_t kv=0; kv<Ny; ++kv)
    {

        for(size_t ku=0; ku<Nx; ++ku)
        {

            // Compute local parametric coordinates (u,v) \in [0,1]
            const float u = ku/(Nx-1.0f);
            const float v = kv/(Ny-1.0f);
            const float z = evaluate_terrain_z(v,u, gui);

            const int num_blocks = z / step;

            const int block_z = Nz_surface + num_blocks;
            blocks[block_z][kv][ku] = 1;
            surface_z[kv][ku] = block_z;
            draw_blocks[block_z][kv][ku] = true;

            if (num_blocks > 0){
                for (size_t kz=1; kz < num_blocks; ++ kz){
                    blocks[kz + Nz_surface][kv][ku] = DIRTY;
                    draw_blocks[kz + Nz_surface][kv][ku] = true;
                }
                surface_z[kv][ku] = block_z;
            }else{
                for (int kz=0; kz > num_blocks; -- kz){
                    blocks[kz + Nz_surface][kv][ku] = NONE;
                    draw_blocks[kz + Nz_surface][kv][ku] = false;
                }
                for (int kz=1; kz < 5; ++ kz){
                    blocks[-kz + block_z][kv][ku] = DIRTY;
                    draw_blocks[-kz + block_z][kv][ku] = true;
                }
                surface_z[kv][ku] = block_z;
            }
            if(surface_z[kv][ku] > Nz_surface) {
                blocks[Nz_surface][kv][ku] = DIRTY;
            }
        }
    }

}

void Grid::generate_dungeons(gui_scene_structure gui)
{
    std::uniform_int_distribution<int> lim_z(2, 4);
    std::uniform_real_distribution<float> f(1.3f, 3.2f);
    std::uniform_real_distribution<float> s(0.9f, 3.0f);
    std::uniform_real_distribution<float> h(25.0f, 35.0f);
    std::uniform_int_distribution<int> o(3, 5);
    std::uniform_real_distribution<float> mn(0.5f, 0.7f);
    std::uniform_real_distribution<float> p(0.3f, 0.45f);

    float min_noise = mn(gen);
    float scaling = s(gen);
    int octave =  o(gen);
    float persistency = p(gen);
    float frequency_gain = f(gen);
    float height = h(gen);
    int show_blocks = 0;

    for (int j=0; j<Ny; ++j){
        for (int i=0; i<Nx; ++i){
            const int lim = surface_z[j][i] - lim_z(gen);
            for (int k=1; k<lim; ++k){
                const float u = i/(Nx-1.0f);
                const float v = j/(Ny-1.0f);
                const float w = k/(Nz-1.0f);

                const float p = perlin(scaling*v, scaling*u, scaling*w*height, octave, persistency, frequency_gain);

                if (p > min_noise){
                    show_blocks += 1;
                    draw_blocks[k][j][i] = true;
                    blocks[k][j][i] = STONE;
                }
                else{
                    draw_blocks[k][j][i] = false;
                    blocks[k][j][i] = NONE;
                }
            }
        }
    }
}

void Grid::create_enter_dungeon(gui_scene_structure gui) {
    bool created = false;
    std::uniform_int_distribution<int> distrib_x(5, (int) Nx-5);
    std::uniform_int_distribution<int> distrib_y(5, (int) Ny-5);
    std::uniform_real_distribution<float> take_block(0.0, 1.0);
    int p_x, p_y, p_z;
    p_x = distrib_x(gen);
    p_y = distrib_y(gen);
    p_z = surface_z[p_y][p_x];
    while(!created) {
        if(!near_block(p_x, p_y, p_z + 1, WATER, 5, true) && blocks[p_z+1][p_y][p_x] == 0) {
            do {
                p_z--;
                for(int i = -4; i <= 4; i++) {
                    for(int j = -4; j <= 4; j++) {
                        float value;
                        if(blocks[p_z+1][p_y][p_x] == GRASS)
                            value = 1.0;
                        else {
                            value = 1.0 - ((float)(i * i + j * j))/16;
                        }
                        if(take_block(gen) < value) {
                            draw_blocks[p_z+1][p_y+j][p_x+i] = false;
                            blocks[p_z+1][p_y+j][p_x+i] = NONE;
                        }
                    }
                }
            } while(blocks[p_z][p_y][p_x] != STONE);
            created = true;
            break;
        }
        p_x = distrib_x(gen);
        p_y = distrib_y(gen);
        p_z = surface_z[p_y][p_x];
    }

}

// Generate trees
void Grid::generate_trees(gui_scene_structure gui)
{
    int num_trees = gui.trees;

    std::uniform_int_distribution<int> distrib_x(3, (int) Nx-4);
    std::uniform_int_distribution<int> distrib_y(3, (int) Ny-4);

    // For tree:
    std::uniform_int_distribution<int> size(3, 6);

    // For foliage:
    std::uniform_int_distribution<int> d_n_x(3, 4);
    std::uniform_int_distribution<int> d_n_y(3, 4);
    std::uniform_int_distribution<int> d_n_z(3, 4);
    std::uniform_real_distribution<float> sc(2.0, 4.0);
    std::uniform_real_distribution<float> pe(0.4, 0.55);

    int p_x, p_y, p_z;
    int n = 0;

    while (n < num_trees) {
        p_x = distrib_x(gen);
        p_y = distrib_y(gen);
        p_z = surface_z[p_y][p_x];

        // First, we see if the condition to draw a tree is satisfied
        if(blocks[p_z][p_y][p_x] == 1 && !near_block(p_x, p_y, p_z + 1, WOOD, 2, true) && blocks[p_z+1][p_y][p_x] == 0) {
            
            int s = size(gen);
            for (int t=1; t<=s; t++){
                blocks[p_z+t][p_y][p_x] = 5;
                draw_blocks[p_z+t][p_y][p_x] = true;
            }

            float scaling = sc(gen);
            int octave = 7;
            float persistency = pe(gen);
            float frequency_gain = 2.0f;
            float min_noise = 0.65f;
            int show_blocks = 0;
            int n_x = d_n_x(gen);
            int n_y = d_n_y(gen);
            int n_z = d_n_z(gen);

            int height = s + p_z - 1;
            for (int k = 1; k < n_z; k++){
                for (int j = 0; j < n_y; j++){
                    for (int i = 0; i < n_x; i++){

                        const float u = i/(n_x-1.0f);
                        const float v = j/(n_y-1.0f);
                        const float w = k/(n_z-1.0f);
                        const float p = perlin(scaling*u, scaling*v, w, octave, persistency, frequency_gain);

                        if (p > min_noise && blocks[k + height][j+p_y - n_y / 2][i + p_x - n_x / 2] != 6){
                            show_blocks += 1;
                            draw_blocks[k + height][j + p_y - n_y / 2][i + p_x - n_x / 2] = true;
                            blocks[k + height][j + p_y - n_y / 2][i + p_x - n_x / 2] = 6;
                        }
                    }
                }
            }
            n++;
        }
    } 
}

void Grid::generate_lake(gui_scene_structure gui)
{
    int count = 0;
    int initialize = 0;
    for(int x = 0; x < Nx; x++) {
        for(int y = 0; y < Ny; y++) {
            if(surface_z[y][x] < WATER_HEIGHT) {
                initialize++;
                for(int h = WATER_HEIGHT; h > surface_z[y][x]; h--) {
                    if(initialize < MIN_WATER || near_block(x, y, h, WATER, 1, false)) {
                        count++;
                        draw_blocks[h][y][x] = true;
                        blocks[h][y][x] = 4;
                        int numbers[3] = {h,y,x};
                        // transform near blocks in sand
                        int sand = 2;

                        for(int incx = -sand; incx <= sand; incx ++) {
                            for(int incy = -sand; incy <= sand; incy ++) {
                                for(int incz = -sand; incz <= sand; incz++) {
                                    if(h + incz < Nz && h + incz > 0 && y + incy < Ny &&
                                    y + incy > 0 && x + incx < Nx && x + incx > 0)
                                        if(blocks[h + incz][y + incy][x + incx] == GRASS) {
                                            blocks[h + incz][y + incy][x + incx] = SAND;
                                        }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

bool Grid::near_block(float x, float y, float z, int block_type, int dist, bool only_surface) {
    bool near = false;
    int height_min = -dist;
    if(only_surface)
        height_min = 0;
    for(int incx = -dist; incx <= dist; incx ++) {
        for(int incy = -dist; incy <= dist; incy ++) {
            for(int incz = height_min; incz <= dist; incz++) {
                if(z + incz < Nz && z + incz > 0 && y + incy < Ny &&
                y + incy > 0 && x + incx < Nx && x + incx > 0)
                    if(blocks[z + incz][y + incy][x + incx] == block_type) {
                        near = true;
                        break;
                    }
            }
        }
    }
    return near;
}

void Grid::generate_flowers(gui_scene_structure gui) {
    // TODO
}

int Grid::position_to_block(vec3 p)
{
    int x = p[0] / step;
    int y = p[1] / step;
    int z = p[2] / step;
    return blocks[z][y][x];
}

void Grid::delete_block(vec3 p){
    int x = p[0] / step;
    int y = p[1] / step;
    int z = p[2] / step;

    //std::cout << "x = " << x << ", y = " << y << ", z = " << z << std::endl;
    blocks[z][y][x] = 0;

    draw_blocks[z][y][x] = false;
    draw_blocks[z+1][y][x] = true;
    draw_blocks[z-1][y][x] = true;
    draw_blocks[z][y+1][x] = true;
    draw_blocks[z][y-1][x] = true;
    draw_blocks[z][y][x+1] = true;
    draw_blocks[z][y][x-1] = true;
}

vec3 Grid::blocks_to_position(int x, int y, int z) {
    float u = x * step;
    float v = y * step;
    float w = z * step;

    return vec3{u, v, w};
}

void Grid::join_surface_dungeon() {

}

float evaluate_terrain_z(float u, float v, const gui_scene_structure& gui)
{
    const float Se = gui.se;
    const float scaling = gui.scaling;
    const int octave = gui.octave;
    const float persistency = gui.persistency;
    const float frequency_gain = gui.frequency;

    const float p = perlin(scaling * u, scaling * v, octave, persistency, frequency_gain);
    const float z = p * pow(fabs(p), Se) * gui.height;

    return z;
}