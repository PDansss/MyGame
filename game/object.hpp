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
    obstacles(IEngine* eng, vector<float> norm, float scale_param_)
    {
        engine           = eng;
        normolize_matrix = norm;

        background_texture = eng->get_texture(13);
        stone_wall         = eng->get_texture(12);
        bricks_wall        = eng->get_texture(10);
        brooken_texture    = eng->get_texture(11);

        scale_param = scale_param_;
    }

    void set_obstacle(const char* path)
    {
        ifstream ifs(path); // Note the typo; the file can't be opened.
        float    x, y;
        string   type;

        if (!ifs.is_open())
        {
            cerr << "Can't open file: " << path << endl;
        }
        else
        {
            while (ifs >> x >> y >> type)
            {
                if (type == "stones")
                {
                    my_obstacles.push_back(
                        { x, y, true, stone_wall, false, false });
                }
                else
                {
                    my_obstacles.push_back(
                        { x, y, true, bricks_wall, true, false });
                }
            }
        }
    }

    void render()
    {
        engine->render_triangle(
            background_box,
            normolize_matrix,
            matrices.scaling_matrix(scale_param, scale_param),
            matrices.single_matrix(),
            background_texture,
            false);
        engine->render_triangle(
            background_box2,
            normolize_matrix,
            matrices.scaling_matrix(scale_param, scale_param),
            matrices.single_matrix(),
            background_texture,
            false);

        for (int i = 0; i < my_obstacles.size(); i++)
        {
            if (my_obstacles[i].exist)
            {
                float centerX = my_obstacles[i].x;
                float centerY = my_obstacles[i].y;

                unsigned int tex;
                if (my_obstacles[i].change)
                    tex = brooken_texture;
                else
                    tex = my_obstacles[i].tex;

                engine->render_triangle(
                    obstacle_box,
                    normolize_matrix,
                    matrices.shift_matrix(centerX * scale_param,
                                          centerY * scale_param),
                    matrices.single_matrix(),
                    tex,
                    false);

                engine->render_triangle(
                    obstacle_box2,
                    normolize_matrix,
                    matrices.shift_matrix(centerX * scale_param,
                                          centerY * scale_param),
                    matrices.single_matrix(),
                    tex,
                    false);
            }
        }
    }
    vector<obstacle>* get_obstacle() { return &my_obstacles; }

private:
    IEngine*      engine = nullptr;
    my_math       matrices;
    vector<float> normolize_matrix;
    float         scale_param;
    unsigned int  background_texture, stone_wall, bricks_wall, brooken_texture;
    vector<obstacle> my_obstacles;

    vector<float> obstacle_box = { -0.1f, 0.1f, 0.0f, 0.6f, 0.5f, 0.4f,
                                   0.0f,  0.0f, 0.1f, 0.1f, 0.0f, 0.6f,
                                   0.5f,  0.4f, 1.0f, 0.0f, 0.1f, -0.1f,
                                   0.0f,  0.6f, 0.5f, 0.4f, 1.0f, 1.0f };

    vector<float> obstacle_box2 = { -0.1f, 0.1f, 0.0f, 0.6f,  0.5f,  0.4f,
                                    0.0f,  0.0f, 0.1f, -0.1f, 0.0f,  0.6f,
                                    0.5f,  0.4f, 1.0f, 1.0f,  -0.1f, -0.1f,
                                    0.0f,  0.6f, 0.5f, 0.4f,  0.0f,  1.0f };

    vector<float> background_box = { -1.0f, 1.0f,  0.0f,  0.75f, 0.75f, 0.75f,
                                     0.0f,  0.0f,  1.0f,  1.0f,  0.0f,  0.7f,
                                     0.75f, 0.75f, 1.0f,  0.0f,  1.0f,  -1.0f,
                                     0.0f,  0.75f, 0.75f, 0.75f, 1.0f,  1.0f };

    vector<float> background_box2 = { -1.0f, 1.0f,  0.0f,  0.75f, 0.75f, 0.75f,
                                      0.0f,  0.0f,  1.0f,  -1.0f, 0.0f,  0.75f,
                                      0.75f, 0.75f, 1.0f,  1.0f,  -1.0f, -1.0f,
                                      0.0f,  0.75f, 0.75f, 0.75f, 0.0f,  1.0f };
};
