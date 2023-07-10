#include <fstream>
#include <stack>
#include <vector>

#include "engine.hpp"
#include "my_math.hpp"

#pragma once

using namespace std;

struct obstacle
{
    float        x;
    float        y;
    bool         exist;
    unsigned int tex;
    bool         breakable;
    bool         change;
};

class obstacles
{
public:
    obstacles(IEngine* eng, vector<float> norm, float scale_param_);
    void              set_obstacle(const char* path);
    void              render();
    vector<obstacle>* get_obstacle();

private:
    IEngine*      engine = nullptr;
    my_math       matrices;
    vector<float> normolize_matrix;
    float         scale_param;
    unsigned int  background_texture, stone_wall, bricks_wall, brooken_texture;
    vector<obstacle> my_obstacles;

    vector<float> obstacle_color   = { 0.6f, 0.5f, 0.4f, 1.0f };
    vector<float> backgroung_color = { 0.75f, 0.75f, 0.75f, 1.0f };
};
