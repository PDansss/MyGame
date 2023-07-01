#include <iostream>
#include <vector>
#include <SDL3/SDL.h>
#include "imgui.h"

#pragma once

#ifdef WIN32
#define ENGSPEC __declspec(dllexport)
#else
#define ENGSPEC
#endif

#ifdef __ANDROID__
#include <SDL3/SDL_main.h>
#define main SDL_main
#endif

using namespace std;

class ENGSPEC IEngine
{
public:
    // general
    virtual void         init(int x, int y, vector<float>& norm)          = 0;
    virtual bool         read_events()                                    = 0;
    virtual int          check_button(string button)                      = 0;
    virtual unsigned int get_time()                                       = 0;
    virtual void         clean()                                          = 0;
    virtual ~IEngine()                                                    = default;

    // render
    virtual unsigned int set_program(const char* vertex_shader,
                                     const char* fragment_shader) = 0;

    virtual unsigned int set_texture(const char* path) = 0;

    virtual void display() = 0;

    virtual void render_triangle(vector<float> atribute_vertex,
                                 vector<float> normolize_matrix,
                                 vector<float> shift_matrix,
                                 vector<float> buffer_matrix,
                                 unsigned int  texture,
                                 bool          blending) = 0;

    virtual void         texture_clean()    = 0;
    virtual unsigned int get_texture(int n) = 0;

    //    // ImGUI
    virtual void Render_gui(float*        vertecies,
                            unsigned int* indexes,
                            vector<float> matrix,
                            unsigned int  size) = 0;

    virtual void create_texture_gui()               = 0;
    virtual void init_gui()                         = 0;
    virtual void DearImGUI_Render(ImDrawData* data) = 0;
    virtual void next_frame_gui()                   = 0;
    virtual void shitdown_gui()                     = 0;

    virtual void draw_main_menu(bool& show_main_menu, bool& exit) = 0;
    virtual void draw_pause_menu(bool& is_paused, bool& is_exit)  = 0;
    virtual void draw_wiget_elements(bool& is_paused, int score)  = 0;

    // Audio
    virtual void play_audio_from_beginning(int num) = 0;
    virtual void play_audio(int num)                = 0;
    virtual void stop_audio(int num)                = 0;
};

ENGSPEC IEngine* create_engine();
ENGSPEC void     delete_engine(IEngine* engine);
