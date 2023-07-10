#include "object.hpp"
#include <sstream>

obstacles::obstacles(IEngine* eng, vector<float> norm, float scale_param_)
{
    engine           = eng;
    normolize_matrix = norm;

    background_texture = engine->get_texture("background");
    stone_wall         = engine->get_texture("stone_wall");
    bricks_wall        = engine->get_texture("brick_wall");
    brooken_texture    = engine->get_texture("broken_wall");

    scale_param = scale_param_;
}

void obstacles::set_obstacle(const char* path)
{
    char* src = nullptr;
    engine->get_char_source(path, src);
    istringstream iss(reinterpret_cast<const char*>(
        src)); // �������� std::istringstream � ��������� �������
    delete[] src;
    string type;
    string x, y;

    vector<obstacle> temp;

    while (iss >> x >> y >> type)
    {
        if (type == "stones")
        {
            temp.push_back({ stof(x) * scale_param,
                             stof(y) * scale_param,
                             true,
                             stone_wall,
                             false,
                             false });
        }
        else
        {
            temp.push_back({ stof(x) * scale_param,
                             stof(y) * scale_param,
                             true,
                             bricks_wall,
                             true,
                             false });
        }
    }
    my_obstacles = temp;
}

void obstacles::render()
{
    engine->render_triangle("background",
                            normolize_matrix,
                            matrices.scaling_matrix(scale_param, scale_param),
                            matrices.single_matrix(),
                            backgroung_color,
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

            engine->render_triangle("object",
                                    normolize_matrix,
                                    matrices.shift_matrix(centerX, centerY),
                                    matrices.single_matrix(),
                                    obstacle_color,
                                    tex,
                                    false);
        }
    }
}
vector<obstacle>* obstacles::get_obstacle()
{
    return &my_obstacles;
}
