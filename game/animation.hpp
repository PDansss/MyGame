#include "object.hpp"

#pragma once

struct anim
{
    float        x;
    float        y;
    int          current_frame;
    unsigned int previos_time;
    float        scale;
};

class animation
{
public:
    void add_animation(float x, float y, float scale)
    {
        animation_stack.push({ x, y, 0, 0, scale });
    }
    void draw_animation(IEngine* eng, vector<float> normolize_matrix)
    {
        stack<anim> temp_anim;
        while (!animation_stack.empty())
        {
            float        x             = animation_stack.top().x;
            float        y             = animation_stack.top().y;
            int          current_frame = animation_stack.top().current_frame;
            unsigned int previos_time  = animation_stack.top().previos_time;
            float        scaling       = animation_stack.top().scale;

            eng->render_triangle(animation_box,
                                 matrices.shift_matrix(x, y),
                                 matrices.scaling_matrix(scaling, scaling),
                                 normolize_matrix,
                                 eng->get_texture(current_frame),
                                 false);
            eng->render_triangle(animation_box2,
                                 matrices.shift_matrix(x, y),
                                 matrices.scaling_matrix(scaling, scaling),
                                 normolize_matrix,
                                 eng->get_texture(current_frame),
                                 false);

            current_time = eng->get_time();
            if (current_time - previos_time > time_between_frames)
            {
                current_frame++;
                previos_time = current_time;
            }
            if (current_frame != 7)
            {
                temp_anim.push({ x, y, current_frame, previos_time, scaling });
            }
            animation_stack.pop();
        }
        animation_stack = temp_anim;
    }

private:
    stack<anim>   animation_stack;
    unsigned int  current_time, time_between_frames = 45;
    my_math       matrices;
    vector<float> animation_box = { -0.1f, 0.1f, 0.0f, 1.0f, 1.0f, 1.0f,
                                    0.0f,  0.0f, 0.1f, 0.1f, 0.0f, 1.0f,
                                    1.0f,  1.0f, 1.0f, 0.0f, 0.1f, -0.1f,
                                    0.0f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

    vector<float> animation_box2 = { -0.1f, 0.1f, 0.0f, 1.0f,  1.0f,  1.0f,
                                     0.0f,  0.0f, 0.1f, -0.1f, 0.0f,  1.0f,
                                     1.0f,  1.0f, 1.0f, 1.0f,  -0.1f, -0.1f,
                                     0.0f,  1.0f, 1.0f, 1.0f,  0.0f,  1.0f };
};
