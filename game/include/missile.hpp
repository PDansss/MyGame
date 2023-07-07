#include "animation.hpp"
#include <stack>

#pragma once

struct missile
{
    float center_x;
    float center_y;
    float dx;
    float dy;
    float angle;
};

class Missile
{
public:
    void add_missile_to_stack(float x, float y, float angle);

    void draw_missile(IEngine* eng, vector<float> normolize_matrix);

    void update(IEngine* eng,
                vector<obstacle>& objects,
                vector<float>     normolize_matrix,
                float             scaling_coefficient,
                animation& anim);

    bool missile_collision_with_enemy(stack<missile>* stack,
                                      float           x,
                                      float           y,
                                      float           scale_coeff,
                                      vector<float>   normolize_matrix);

    bool missile_collision_with_player(vector<stack<missile>*> vector_missile_stack,
                                       float                   x,
                                       float                   y,
                                       float                   scale_coeff,
                                       vector<float>           normolize_matrix);

    stack<missile>* get_stack();

private:
    stack<missile> missiles;
    my_math        matrices;
    float          missile_speed = 25.f;
    vector<float> color = { 1.0f, 1.0f, 1.0f ,1.0f };
};
