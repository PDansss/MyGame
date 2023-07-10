#include <iostream>
#include <vector>

#include "my_math.hpp"
#include "imgui.h"

#pragma once

#ifdef WIN32
#define ENGSPEC __declspec(dllexport)
#else
#define ENGSPEC
#endif

using namespace std;

class ENGSPEC IEngine
{
public:
    virtual void         init(int x, int y, vector<float>& norm, float scale) = 0;
    virtual bool         read_events()                           = 0;
    virtual int          check_button(string button)             = 0;
    virtual unsigned int get_time()                              = 0;
    virtual void         clean()                                 = 0;
    virtual ~IEngine()                                           = default;

    virtual void display() = 0;

    virtual void render_triangle(string buffer,
                                 vector<float> normolize_matrix,
                                 vector<float> shift_matrix,
                                 vector<float> buffer_matrix,
                                 vector<float> color,
                                 unsigned int  texture,
                                 bool          blending) = 0;

    virtual void render_triangle(string buffer,
                                 string buffer2,
                                 int num, 
                                 vector<float> normolize_matrix,
                                 vector<float> shift_matrix) = 0;

    virtual void render_line(vector<float> attributes, 
                             vector<float> normolize_matrix,
                             vector<float> shift_matrix,
                             vector<float> buffer_matrix, 
                             vector<unsigned int> ind) = 0;
   
    virtual void set_buffer(unsigned int buffer, vector<float> data)        = 0;
    virtual void set_buffer(unsigned int buffer, vector<unsigned int> data) = 0;
    virtual unsigned int get_vertex_buffer(string texture)                  = 0;
    virtual unsigned int get_index_buffer(string texture)                   = 0;
    virtual unsigned int get_texture(string texture)                        = 0;

    virtual void draw_main_menu(bool& show_main_menu, bool& exit) = 0;
    virtual void draw_pause_menu(bool& is_paused, bool& is_exit)  = 0;
    virtual void draw_wiget_elements(bool& is_paused, int score)  = 0;

    virtual void play_audio_from_beginning(int num) = 0;
    virtual void play_audio(int num)                = 0;
    virtual void stop_audio(int num)                = 0;
};

ENGSPEC IEngine* create_engine();
ENGSPEC void     delete_engine(IEngine* engine);
