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
    void add_animation(float x, float y, float scale);
    void draw_animation(IEngine* eng, vector<float> normolize_matrix);

private:
    stack<anim>  animation_stack;
    unsigned int current_time = 0, time_between_frames = 45;
    my_math      matrices;

    vector<float> color = { 1.0f, 1.0f, 1.0f, 1.0f };

    vector<string> animation_frames = { "explosion01", "explosion02",
                                        "explosion03", "explosion04",
                                        "explosion05", "explosion06",
                                        "explosion07" };
};
