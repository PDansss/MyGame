#include <glad.h>
#include <unordered_map>

#include "engine.hpp"
#include "my_audio.hpp"
#include "picopng.hpp"

#ifdef _WIN32
#define OPENGL_MAJOR_VERSION 4
#define OPENGL_MINOR_VERSION 3
#define OPENGL_CONTEXT SDL_GL_CONTEXT_PROFILE_COMPATIBILITY
#else
    #define OPENGL_MAJOR_VERSION 3
    #define OPENGL_MINOR_VERSION 0
    #define OPENGL_CONTEXT SDL_GL_CONTEXT_PROFILE_ES
#endif

#ifdef __ANDROID__
#include <android/log.h>
#include <GLES3/gl3.h>

void logToLogcat(const char* message) {
    __android_log_print(ANDROID_LOG_ERROR, "MyApp", "%s", message);
}
#endif


void check_gl_errors();

void* load_opengl_func(const char* name)
{
    SDL_FunctionPointer func_ptr = SDL_GL_GetProcAddress(name);
    return reinterpret_cast<void*>(func_ptr);
}

class engine : public IEngine
{
public:
    void init(int x, int y, vector<float>& norm, float scale)
    {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
        {
            throw runtime_error("Could not initialize!");
        }

        const SDL_DisplayMode* DM = SDL_GetCurrentDisplayMode(1);

        width  = DM->w;
        height = DM->h;

        window = SDL_CreateWindow("TANKS", width, height, SDL_WINDOW_OPENGL);

        SDL_SetWindowFullscreen(window, SDL_TRUE);
		
        SDL_GetWindowSizeInPixels(window, &width_in_pixels, &height_in_pixels);
       
		norm = math.matrix_multiplying(math.scaling_matrix((float)height_in_pixels/(float)width_in_pixels, 1), math.scaling_matrix(scale, scale));

        if (window == nullptr)
        {
            throw runtime_error("Сould not create a window!");
        }
        cout << "Engine was initialized!\n";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,  OPENGL_CONTEXT);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_MAJOR_VERSION);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_MINOR_VERSION);

        opengl_context = SDL_GL_CreateContext(window);

        if (opengl_context == nullptr)
        {
            throw runtime_error("Engine could not create OpenGL ES context!");
        }
        if (!gladLoadGLES2Loader(load_opengl_func))
        {
            throw runtime_error(
                "Engine could not initialize OpenGL ES functions!");
        }
        glViewport(0, 0, width, height);

        main_program = set_program("res/shaders/main_vertex_shader.txt",
                                   "res/shaders/main_fragment_shader.txt");

        imgui_program = set_program("res/shaders/imgui_vertex_shader.txt",
                                    "res/shaders/imgui_fragment_shader.txt");

        morphing_program = set_program("res/shaders/morphing_vertex_shader.txt",
                                       "res/shaders/morphing_fragment_shader.txt");

        line_program = set_program("res/shaders/line_vertex_shader.txt",
                                   "res/shaders/line_fragment_shader.txt");

        SDL_AudioSpec device_info, want;

        want.freq     = 44100;
        want.format   = SDL_AUDIO_S16;
        want.channels = 2;
        want.samples  = 1024;
        want.callback = my_audio::MyCallBack;
        want.silence  = 0;
        want.userdata = nullptr;
        device        = SDL_OpenAudioDevice(nullptr, 0, &want, &device_info, 0);

        shoot.set_data("res/music/shot.wav", device_info, false);
        fon.set_data("res/music/game_music.wav", device_info, true);
        menu.set_data("res/music/menu_music.wav", device_info, true);

        set_texture("res/textures/explosion/01.png", textures.at("explosion01"));
        set_texture("res/textures/explosion/02.png", textures.at("explosion02"));
        set_texture("res/textures/explosion/03.png", textures.at("explosion03"));
        set_texture("res/textures/explosion/04.png", textures.at("explosion04"));
        set_texture("res/textures/explosion/05.png", textures.at("explosion05"));
        set_texture("res/textures/explosion/06.png", textures.at("explosion06"));
        set_texture("res/textures/explosion/07.png", textures.at("explosion07"));
        set_texture("res/textures/tank_part_1.png" , textures.at("tank_part_1"));
        set_texture("res/textures/tank_part_2.png" , textures.at("tank_part_2"));
        set_texture("res/textures/missile.png"     , textures.at("missile"));
        set_texture("res/textures/brick_wall.png"  , textures.at("brick_wall"));
        set_texture("res/textures/broken_wall.png" , textures.at("broken_wall"));
        set_texture("res/textures/stone_wall.png"  , textures.at("stone_wall"));
        set_texture("res/textures/background.png"  , textures.at("background"));
        set_texture("res/textures/arrow.png"       , textures.at("arrow_up"));
        set_texture("res/textures/arrow_down.png"  , textures.at("arrow_down"));

        gen_buffers(vertex_buffers.at("tank"));
        gen_buffers(vertex_buffers.at("missile"));
        gen_buffers(vertex_buffers.at("animation"));
        gen_buffers(vertex_buffers.at("object"));
        gen_buffers(vertex_buffers.at("background"));
        gen_buffers(vertex_buffers.at("arrow"));
        gen_buffers(vertex_buffers.at("figure1"));
        gen_buffers(vertex_buffers.at("figure2"));
        gen_buffers(index_buffers.at("indexes"));

        set_buffer(get_vertex_buffer("arrow"), arrows_vertecies);

        my_audio::sounds.push_back(&menu);
        my_audio::sounds.push_back(&fon);
        my_audio::sounds.push_back(&shoot);
        SDL_PlayAudioDevice(device);
    
        init_gui();
    }

    bool read_events()
    {
        next_frame_gui();
        SDL_Event Event;
        buttons.at("MOUSE_CLICK") = 0;
        buttons.at("MOUSE_UP") = 0;
        buttons.at("MOUSE_WHEEL") = 0;

        while (SDL_PollEvent(&Event))
        {
            if (Event.type == SDL_EVENT_QUIT)
            {
                return false;
            }

#ifdef __ANDROID__
            if (Event.type == SDL_EVENT_FINGER_DOWN)
            {
                if (Event.tfinger.x * width_in_pixels > width_in_pixels * 0.05 &&
                    Event.tfinger.x * width_in_pixels < width_in_pixels * 0.05 + width_in_pixels * 0.15 &&
                    Event.tfinger.y * height_in_pixels > height_in_pixels * 0.6 &&
                    Event.tfinger.y * height_in_pixels < height_in_pixels * 0.6 + height_in_pixels * 0.15) {
                    buttons.at("S") = 1;
                    left_down_arrow_color[3] = 0.6f;
                }
                else if (Event.tfinger.x * width_in_pixels > width_in_pixels * 0.8 &&
                    Event.tfinger.x * width_in_pixels < width_in_pixels * 0.8 + width_in_pixels * 0.15 &&
                    Event.tfinger.y * height_in_pixels > height_in_pixels * 0.6 &&
                    Event.tfinger.y * height_in_pixels < height_in_pixels * 0.6 + height_in_pixels * 0.15) {
                    buttons.at("D") = 1;
                    right_down_arrow_color[3] = 0.6f;
                }

                else if (Event.tfinger.x * width_in_pixels > width_in_pixels * 0.05 &&
                    Event.tfinger.x * width_in_pixels < width_in_pixels * 0.05 + width_in_pixels * 0.15 &&
                    Event.tfinger.y * height_in_pixels > height_in_pixels * 0.2 &&
                    Event.tfinger.y * height_in_pixels < height_in_pixels * 0.2 + height_in_pixels * 0.15) {
                    buttons.at("W") = 1;
                    left_up_arrow_color[3] = 0.6f;
                }

                else if (Event.tfinger.x * width_in_pixels > width_in_pixels * 0.8f &&
                    Event.tfinger.x * width_in_pixels < width_in_pixels * 0.8f + width_in_pixels * 0.15 &&
                    Event.tfinger.y * height_in_pixels > height_in_pixels * 0.2 &&
                    Event.tfinger.y * height_in_pixels < height_in_pixels * 0.2 + height_in_pixels * 0.15) {
                    buttons.at("A") = 1;
                    right_up_arrow_color[3] = 0.6f;
                }
                else
                    buttons.at("MOUSE_CLICK") = 1;
            }
             if (Event.type == SDL_EVENT_FINGER_UP) {
                 buttons.at("MOUSE_UP") = 1;
                 if (Event.tfinger.x * width_in_pixels > width_in_pixels * 0.05 &&
                     Event.tfinger.x * width_in_pixels <
                     width_in_pixels * 0.05 + width_in_pixels * 0.15 &&
                     Event.tfinger.y * height_in_pixels > height_in_pixels * 0.6 &&
                     Event.tfinger.y * height_in_pixels <
                     height_in_pixels * 0.6 + height_in_pixels * 0.15) {
                     buttons.at("S") = 0;
                     left_down_arrow_color[3] = 1.0f;
                 } else if (Event.tfinger.x * width_in_pixels > width_in_pixels * 0.8 &&
                            Event.tfinger.x * width_in_pixels <
                            width_in_pixels * 0.8 + width_in_pixels * 0.15 &&
                            Event.tfinger.y * height_in_pixels > height_in_pixels * 0.6 &&
                            Event.tfinger.y * height_in_pixels <
                            height_in_pixels * 0.6 + height_in_pixels * 0.15) {
                     buttons.at("D") = 0;
                     right_down_arrow_color[3] = 1.0f;
                 } else if (Event.tfinger.x * width_in_pixels > width_in_pixels * 0.05 &&
                            Event.tfinger.x * width_in_pixels <
                            width_in_pixels * 0.05 + width_in_pixels * 0.15 &&
                            Event.tfinger.y * height_in_pixels > height_in_pixels * 0.2 &&
                            Event.tfinger.y * height_in_pixels <
                            height_in_pixels * 0.2 + height_in_pixels * 0.15) {
                     buttons.at("W") = 0;
                     left_up_arrow_color[3] = 1.0f;
                 } else if (Event.tfinger.x * width_in_pixels > width_in_pixels * 0.8f &&
                            Event.tfinger.x * width_in_pixels <
                            width_in_pixels * 0.8f + width_in_pixels * 0.15 &&
                            Event.tfinger.y * height_in_pixels > height_in_pixels * 0.2 &&
                            Event.tfinger.y * height_in_pixels <
                            height_in_pixels * 0.2 + height_in_pixels * 0.15) {
                     buttons.at("A") = 0;
                     right_up_arrow_color[3] = 1.0f;
                 }
             }
             if(Event.type == SDL_EVENT_FINGER_MOTION){
                 if(Event.tfinger.dx > 0)
                     buttons.at("MOUSE_WHEEL") = 0.5f;
                 else
                     buttons.at("MOUSE_WHEEL") = -0.5f;
             }
#else
            if (Event.type == SDL_EVENT_KEY_DOWN)
            {
                if (Event.key.keysym.sym == SDLK_w)
                {
                    buttons.at("W") = 1;
                }
                else if (Event.key.keysym.sym == SDLK_a)
                {
                    buttons.at("A") = 1;
                }
                else if (Event.key.keysym.sym == SDLK_s)
                {
                    buttons.at("S") = 1;
                }
                else if (Event.key.keysym.sym == SDLK_d)
                {
                    buttons.at("D") = 1;
                }
            }

            else if (Event.type == SDL_EVENT_KEY_UP)
            {
                if (Event.key.keysym.sym == SDLK_w)
                {
                    buttons.at("W") = 0;
                }
                else if (Event.key.keysym.sym == SDLK_a)
                {
                    buttons.at("A") = 0;
                }
                else if (Event.key.keysym.sym == SDLK_s)
                {
                    buttons.at("S") = 0;
                }
                else if (Event.key.keysym.sym == SDLK_d)
                {
                    buttons.at("D") = 0;
                }
            }
            if (Event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
            {
                buttons.at("MOUSE_CLICK") = 1;
                buttons.at("MOUSE_UP") = 1;
            }

            if (Event.type == SDL_EVENT_MOUSE_WHEEL)
            {
                buttons.at("MOUSE_WHEEL") = Event.wheel.y;
            }
#endif
        }
        return true;
    }

    float check_button(string button) { return buttons.at(button); }

    unsigned int get_time() { return (unsigned int)SDL_GetTicks(); }

    void clean()
    {
        shitdown_gui();

        delete_buffers(vertex_buffers.at("tank"));
        delete_buffers(vertex_buffers.at("missile"));
        delete_buffers(vertex_buffers.at("animation"));
        delete_buffers(vertex_buffers.at("object"));
        delete_buffers(vertex_buffers.at("background")); 
        delete_buffers(vertex_buffers.at("arrow"));
        delete_buffers(vertex_buffers.at("figure1"));
        delete_buffers(vertex_buffers.at("figure2"));
        delete_buffers(index_buffers.at("indexes"));

        texture_clean();

        SDL_free(shoot.audio_buffer);
        SDL_free(fon.audio_buffer);
        SDL_free(menu.audio_buffer);
        SDL_PauseAudioDevice(device);
        SDL_CloseAudioDevice(device);

        glDeleteProgram(main_program);
        glDeleteProgram(imgui_program);
        glDeleteProgram(morphing_program);
        glDeleteProgram(line_program);

        SDL_DestroyWindow(window);
        SDL_Quit();
        cout << "Engine was cleaned!\n";
    }

    void set_buffer(unsigned int buffer, vector<float> data) {
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        check_gl_errors();
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
        check_gl_errors();
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        check_gl_errors();

    }
    
    void set_buffer(unsigned int buffer, vector<unsigned int> data) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
        check_gl_errors();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
        check_gl_errors();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        check_gl_errors();
    }
    
    unsigned int get_vertex_buffer(string buffer_name) {
        return vertex_buffers.at(buffer_name);
    }
    unsigned int get_index_buffer(string buffer_name) {
        return index_buffers.at(buffer_name);
    }
    unsigned int get_texture(string texture) { return textures.at(texture); }

    void render_triangle(string buffer,
                         vector<float> normolize_matrix,
                         vector<float> shift_matrix,
                         vector<float> buffer_matrix,
                         vector<float> obstacle_color,
                         unsigned int  texture,
                         bool          blending) {
        glUseProgram(main_program);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers.at(buffer));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, get_index_buffer("indexes"));

        glUseProgram(main_program);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 20, 0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 20, (void*)(3 * sizeof(float)));

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glActiveTexture(GL_TEXTURE0);

        glUniform4fv(
            glGetUniformLocation(main_program, "figure1_color"),
            1,
            obstacle_color.data());

        glUniformMatrix3fv(
            glGetUniformLocation(main_program, "normolize_matrix"),
            1,
            GL_TRUE,
            normolize_matrix.data());
        glUniformMatrix3fv(glGetUniformLocation(main_program, "shift_matrix"),
            1,
            GL_TRUE,
            shift_matrix.data());
        glUniformMatrix3fv(glGetUniformLocation(main_program, "buffer_matrix"),
            1,
            GL_TRUE,
            buffer_matrix.data());

        glBindTexture(GL_TEXTURE_2D, texture);

        if (blending)
        {
            glEnable(GL_BLEND);

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void render_triangle(string buffer,
                         string buffer2,
                         int num,
                         vector<float> normolize_matrix,
                         vector<float> shift_matrix) {

        glUseProgram(morphing_program);

        glUniformMatrix3fv(glGetUniformLocation(morphing_program, "norm_matrix"),
            1,
            GL_TRUE,
            normolize_matrix.data());

        glUniformMatrix3fv(glGetUniformLocation(morphing_program, "shift_matrix"),
            1,
            GL_TRUE,
            shift_matrix.data());

        glUniform1f(glGetUniformLocation(morphing_program, "theta"), sin(get_time() * 0.007));

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers.at(buffer));
        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE, 24, 0);
        check_gl_errors();
        glVertexAttribPointer(
            1, 3, GL_FLOAT, GL_FALSE, 24, (void*)(3 * sizeof(float)));
        check_gl_errors();

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers.at(buffer2));
        glVertexAttribPointer(
            2, 3, GL_FLOAT, GL_FALSE, 24, 0);
        check_gl_errors();
        glVertexAttribPointer(
            3, 3, GL_FLOAT, GL_FALSE, 24, (void*)(3 * sizeof(float)));
        check_gl_errors();
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glEnableVertexAttribArray(0);
        check_gl_errors();
        glEnableVertexAttribArray(1);
        check_gl_errors();
        glEnableVertexAttribArray(2);
        check_gl_errors();
        glEnableVertexAttribArray(3);
        check_gl_errors();
        glDrawArrays(GL_TRIANGLE_FAN, 0, num);
        check_gl_errors();
    }

    
    void render_line(vector<float> attributes,
                     vector<float> normolize_matrix,
                     vector<float> shift_matrix,
                     vector<float> buffer_matrix,
                     vector<unsigned int> ind) {
        glUseProgram(line_program);
        check_gl_errors();
        glUniformMatrix3fv(
            glGetUniformLocation(line_program, "normolize_matrix"),
            1,
            GL_TRUE,
            normolize_matrix.data());
        check_gl_errors();
        glUniformMatrix3fv(glGetUniformLocation(line_program, "shift_matrix"),
            1,
            GL_TRUE,
            shift_matrix.data());
        check_gl_errors();
        glUniformMatrix3fv(glGetUniformLocation(line_program, "buffer_matrix"),
            1,
            GL_TRUE,
            buffer_matrix.data());
        check_gl_errors();
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, attributes.data());
        check_gl_errors();
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, attributes.data() + 3);

        check_gl_errors();
        glEnableVertexAttribArray(0);
        check_gl_errors();
        glEnableVertexAttribArray(1);
        check_gl_errors();

        check_gl_errors();
        glDrawElements(GL_LINE_STRIP, ind.size(), GL_UNSIGNED_INT, ind.data());
    }

    void display()
    {
        ImGui::Render();
        DearImGUI_Render(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    };

    void Render_gui(float*        tank_vertecies,
                    unsigned int* indexes,
                    vector<float> matrix,
                    unsigned int  size)
    {
        glUseProgram(imgui_program);
        glVertexAttribPointer(
            0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, tank_vertecies);
        glVertexAttribPointer(
            1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, tank_vertecies + 2);
        glVertexAttribPointer(
            2, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 8, tank_vertecies + 4);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glUniformMatrix3fv(glGetUniformLocation(imgui_program, "proj_mat"),
                           1,
                           GL_TRUE,
                           matrix.data());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, imgui_texture_handle);
        glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, indexes);
        glUseProgram(0);
    }

    void create_texture_gui()
    {
        ImGuiIO&       io     = ImGui::GetIO();
        unsigned char* pixels = nullptr;
        int            width  = 0;
        int            height = 0;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        glGenTextures(1, &imgui_texture_handle);
        glBindTexture(GL_TEXTURE_2D, imgui_texture_handle);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGBA,
                     width,
                     height,
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void draw_main_menu(bool& show_main_menu, bool& exit)
    {
        stop_audio(1);
        stop_audio(2);
        play_audio(0);
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);

        ImGui::Begin("MainMenu",
                     0,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoDecoration);

        ImGui::SetCursorPos(
            ImVec2(io.DisplaySize.x * 0.5f - io.DisplaySize.x * 0.045f,
                   io.DisplaySize.y * 0.15f));
        ImGui::TextWrapped("Menu");

        ImGui::SetCursorPos(
            ImVec2(io.DisplaySize.x * 0.375f, io.DisplaySize.y * 0.35f));
        if (ImGui::Button(
                "Play",
                ImVec2(io.DisplaySize.x * 0.25f, io.DisplaySize.y * 0.1f)))
        {
            show_main_menu = false;
        }

        ImGui::SetCursorPos(
            ImVec2(io.DisplaySize.x * 0.375f, io.DisplaySize.y * 0.5f));
        if (ImGui::Button(
                "Exit",
                ImVec2(io.DisplaySize.x * 0.25f, io.DisplaySize.y * 0.1f)))
        {
            exit = true;
        }
        ImGui::End();
    }

    void draw_pause_menu(bool& is_paused, bool& is_exit)
    {
        stop_audio(0);
        play_audio(1);
        ImGuiIO& io = ImGui::GetIO();
        if (is_paused)
        {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(io.DisplaySize);

            ImGui::Begin("PauseMenu",
                         0,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoDecoration);

            ImGui::SetCursorPos(
                ImVec2(io.DisplaySize.x * 0.5f - io.DisplaySize.x * 0.055f,
                       io.DisplaySize.y * 0.15f));
            ImGui::Text("PAUSE");

            ImGui::SetCursorPos(
                ImVec2(io.DisplaySize.x * 0.375f, io.DisplaySize.y * 0.35f));
            if (ImGui::Button(
                    "Continue",
                    ImVec2(io.DisplaySize.x * 0.25f, io.DisplaySize.y * 0.1f)))
            {
                std::cout << "First!" << std::endl << std::flush;
                is_paused = false;
            }

            ImGui::SetCursorPos(
                ImVec2(io.DisplaySize.x * 0.375f, io.DisplaySize.y * 0.52f));
            if (ImGui::Button(
                    "Exit",
                    ImVec2(io.DisplaySize.x * 0.25f, io.DisplaySize.y * 0.1f)))
            {
                std::cout << "Second!" << std::endl;
                is_exit   = true;
                is_paused = false;

                fon.play          = false;
                fon.continue_play = false;
                fon.offset        = 0;
                menu.play         = true;
            }

            ImGui::End();
        }
    }

    void draw_widgets(bool& is_paused, int score)
    {

        ImGuiIO& io    = ImGui::GetIO();
        float    scale = io.DisplaySize.y / io.DisplaySize.x;

        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.02f, io.DisplaySize.y * 0.02f));
        ImGui::SetNextWindowSize(
            ImVec2(io.DisplaySize.x * 0.07f * scale, io.DisplaySize.y * 0.07f));
        ImGui::Begin(
            "PauseButtom",
            0,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

        ImGui::SetCursorPos(ImVec2(0, 0));

        ImGui::SetWindowFontScale(0.5f);
        if (ImGui::Button("X",
                          ImVec2(ImGui::GetContentRegionAvail().x,
                                 ImGui::GetContentRegionAvail().y)))
        {
            std::cout << "First!" << std::endl << std::flush;
            is_paused = true;
        }
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.79f, io.DisplaySize.y * 0.02f));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x * 0.2f * scale,
                                        io.DisplaySize.y * 0.1f * scale));

        ImGui::Begin(
            "Score",
            0,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

        ImGui::SetCursorPos(ImVec2(0, 0));
        ImGui::SetWindowFontScale(0.35f);
        ImGui::Text("SCORE: %d", score);

        ImGui::End();
		#ifdef __ANDROID__
        draw_control_arrows();
		#endif
    }

    void init_gui()
    {
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGuiIO& io = ImGui::GetIO();

        ImGuiStyle& style             = ImGui::GetStyle();
        style.FrameRounding           = 40.f;
        style.Colors[ImGuiCol_Button] = ImVec4(0.4f, 0.3f, 0.7f, 1.0f);

        io.FontGlobalScale     = 7;
        io.BackendPlatformName = "imgui";
        io.Fonts->AddFontDefault();
        create_texture_gui();
    }

    void DearImGUI_Render(ImDrawData* data)
    {

        ImGuiIO& io = ImGui::GetIO();

        data->ScaleClipRects(io.DisplayFramebufferScale);

        float scale = io.DisplaySize.y / io.DisplaySize.x;

        vector<float> matrix;

        matrix = math.matrix_multiplying(
            math.shift_matrix(-1.0f, 1.0f),
            math.scaling_matrix(2.0f / io.DisplaySize.x,
                                -2.0f / io.DisplaySize.y));

        for (int i = 0; i < data->CmdListsCount; i++)
        {
            const ImDrawList* cmd_list = data->CmdLists[i];

            ImDrawVert*  vertex = cmd_list->VtxBuffer.Data;
            unsigned int num    = cmd_list->VtxBuffer.Size;

            ImDrawIdx*   index   = cmd_list->IdxBuffer.Data;
            unsigned int num_ind = cmd_list->IdxBuffer.Size;

            float*        vert = new float[num * 8];
            unsigned int* indx = new unsigned int[num_ind];

            int k = 0;

            for (unsigned int j = 0; j < num; j++)
            {
                vert[k] = vertex[j].pos.x;
                k++;
                vert[k] = vertex[j].pos.y;
                k++;
                vert[k] = vertex[j].uv.x;
                k++;
                vert[k] = vertex[j].uv.y;
                k++;

                unsigned int obstacle_color = vertex[j].col;

                vert[k] = (obstacle_color & 0x000000FF) / 255.f;
                k++;
                vert[k] = ((obstacle_color & 0x0000FF00) >> 8) / 255.f;
                k++;
                vert[k] = ((obstacle_color & 0x00FF0000) >> 16) / 255.f;
                k++;
                vert[k] = ((obstacle_color & 0xFF000000) >> 24) / 255.f;
                k++;
            }
            for (unsigned int j = 0; j < num_ind; j++)
            {
                indx[j] = static_cast<unsigned int>(index[j]);
            }

            Render_gui(vert, indx, matrix, num_ind);

            delete[] vert;
            delete[] indx;
        }
    }
    
    void next_frame_gui()
    {
        ImGuiIO& io = ImGui::GetIO();

        int   width, height, p_width, p_height;
        float scale = io.DisplaySize.y / io.DisplaySize.x;

        SDL_GetWindowSize(window, &width, &height);
        io.DisplaySize = ImVec2(float(width), float(height));

        SDL_GetWindowSizeInPixels(window, &p_width, &p_height);
        io.DisplayFramebufferScale =
            ImVec2(float(p_width / width), float(p_height / height));

        float  x, y;
        SDL_GetMouseState(&x, &y);
        io.MousePos.x = x;
        io.MousePos.y = y;

        io.MouseDown[0] = buttons.at("MOUSE_UP");
        ImGui::NewFrame();
    }

    void shitdown_gui() { 
        ImGui::DestroyContext(); 
    }

    void play_audio(int num) { my_audio::sounds[num]->play_audio(); }

    void play_audio_from_beginning(int num)
    {
        my_audio::sounds[num]->play_audio_from_beginning();
    };

    void stop_audio(int num) { my_audio::sounds[num]->stop_audio(); }

    void get_char_source(const char* path, char*& data) {
        SDL_RWops* src = SDL_RWFromFile(path, "r");
        if (src != NULL) {
            unsigned int size = SDL_RWsize(src);
            data = new char[size + 1];
            size_t bytesRead = SDL_RWread(src, data, size);
            data[size] = '\0';
        }
        else {
            //logToLogcat(SDL_GetError());
        }
    }

private:
    SDL_Window*   window         = nullptr;
    SDL_GLContext opengl_context = nullptr;
    int           width          = 1920;
    int           height         = 1080;

    int   width_in_pixels = 1900, height_in_pixels = 680;

    unordered_map<string, float> buttons{ { "A", 0 },           { "W", 0 },
                                        { "S", 0 },           { "D", 0 },
                                        { "MOUSE_WHEEL", 0 }, { "MOUSE_CLICK", 0 },
                                        { "MOUSE_UP",0} };

    unordered_map<string, unsigned int> textures{{ "explosion01",0 } , { "explosion02", 0 },
                                                { "explosion03", 0 } , { "explosion04", 0 },
                                                { "explosion05", 0 } , { "explosion06", 0 },
                                                { "explosion07", 0 } , { "tank_part_1", 0 },
                                                { "tank_part_2", 0 } , { "missile"    , 0 },
                                                { "brick_wall" , 0 } , { "broken_wall", 0 },
                                                { "stone_wall" , 0 } , { "background", 0  },
                                                { "arrow_up",    0 },  { "arrow_down",  0 }};

    unordered_map<string, unsigned int> vertex_buffers{ {"tank"      ,0}, {"missile",0},
                                                        {"animation" ,0}, {"object" ,0},
                                                        {"background",0}, {"arrow"  ,0 },
                                                        {"figure1"   ,0}, {"figure2",0} };

    unordered_map<string, unsigned int> index_buffers{ { "indexes",0 } };

    vector<float> arrows_vertecies = {
        0.0f,                    0.0f,                     0.0f, 0.0f, 0.0f,
        width * 0.15f, 0.0f,                     0.0f, 1.0f, 0.0f,
        width * 0.15f, height * 0.15f, 0.0f, 1.0f, 1.0f,
        0.0f,                    height * 0.15f, 0.0f, 0.0f, 1.0f };


    vector<float> arrow_color = { 1.0f,1.0f,1.0f,1.0f };

    vector<float> left_down_arrow_color = { 0.6f,0.6f,0.6f,1.0f };
    vector<float> right_down_arrow_color = { 0.6f,0.6f,0.6f,1.0f };
    vector<float> left_up_arrow_color = { 0.6f,0.6f,0.6f,1.0f };
    vector<float> right_up_arrow_color = { 0.6f,0.6f,0.6f,1.0f };

    my_math math;

    unsigned int imgui_texture_handle;

    GLuint main_program;
    GLuint imgui_program;
    GLuint morphing_program;
    GLuint line_program;

    my_audio::my_audio shoot;
    my_audio::my_audio fon;
    my_audio::my_audio menu;

    SDL_AudioDeviceID device;

    string get_source(const char* filename)
    {
        char* code = nullptr;
        get_char_source(filename, code);
        string source(code);
        delete[] code;
        return source;
    }

    GLuint load_shader(GLenum shader_type, const char* file)
    {
        string      src    = get_source(file);
        const char* source = src.c_str();

        GLuint shader = glCreateShader(shader_type);
        GLint  compiled;
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);

        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled)
        {
            GLint infolen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infolen);
            if (infolen > 1)
            {
                char* error = new char[infolen];
                glGetShaderInfoLog(shader, infolen, NULL, error);
                std::cerr << error << std::endl;
                delete[] error;
            }
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    unsigned int set_program(const char* vertex_shader,
                             const char* fragment_shader)
    {
        GLint  linked;
        GLint  vrt_shader = load_shader(GL_VERTEX_SHADER, vertex_shader);
        GLint  frag_shader = load_shader(GL_FRAGMENT_SHADER, fragment_shader);
        GLuint program = glCreateProgram();

        glAttachShader(program, vrt_shader);
        glAttachShader(program, frag_shader);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (!linked)
        {
            GLint infolen = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infolen);
            if (infolen > 1)
            {
                char* error = new char[infolen];
                glGetProgramInfoLog(program, infolen, NULL, error);
                cerr << error << endl;
                delete[] error;
            }
            glDeleteProgram(program);
            return 0;
        }
        glDeleteShader(vrt_shader);
        glDeleteShader(frag_shader);
        return program;
    }

    void set_texture(const char* path, unsigned int& handle)
    {
        SDL_RWops* src = SDL_RWFromFile(path, "rb");
        vector<byte> info;
        GLuint       texture_handle;

        if (src != NULL) {
            unsigned int size = SDL_RWsize(src);
            info.resize(size);
            size_t bytesRead = SDL_RWread(src, info.data(), size);
        }
        else
        {
            cerr << "Something wrong with this file: " << path << ": " << endl;
            return;
        }

        vector<byte>  image_pixels;
        unsigned long w = 0, h = 0;

        if (decodePNG(image_pixels, w, h, info.data(), info.size(), false))
        {
            cerr << "ERROR IN DECODING PNG FILE!!" << endl;
            return;
        }

        glGenTextures(1, &handle);
        glBindTexture(GL_TEXTURE_2D, handle);
        glTexImage2D(GL_TEXTURE_2D,
            0,
            GL_RGBA,
            w,
            h,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            image_pixels.data());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    void texture_clean()
    {
        for (auto& n : textures) {
            glDeleteTextures(1, &n.second);
        }
    }

    void draw_control_arrows() {

        vector<float> m = math.shift_matrix(-1.0f, 1.0f);

        render_triangle(
                "arrow",
                m,
                math.scaling_matrix(2.0f / width, -2.0f / height),
                math.shift_matrix(width * 0.05f, height * 0.6f),
                left_down_arrow_color,
                get_texture("arrow_down"),
                false);

        render_triangle("arrow",
                        m,
                        math.scaling_matrix(2.0f / width, -2.0f / height),
                        math.shift_matrix(width * 0.8f, height * 0.6f),
                        right_down_arrow_color,
                        get_texture("arrow_down"),
                        false);

        render_triangle("arrow",
                        m,
                        math.scaling_matrix(2.0f / width, -2.0f / height),
                        math.shift_matrix(width * 0.05f, height * 0.2f),
                        left_up_arrow_color,
                        get_texture("arrow_up"),
                        false);

        render_triangle("arrow",
                        m,
                        math.scaling_matrix(2.0f / width, -2.0f / height),
                        math.shift_matrix(width * 0.8f, height * 0.2f),
                        right_up_arrow_color,
                        get_texture("arrow_up"),
                        false);
    }

    void gen_buffers(unsigned int& buffer_handel) {
        glGenBuffers(1, &buffer_handel);
    }
    
    void delete_buffers(unsigned int& buffer_handel) {
        glDeleteBuffers(1, &buffer_handel);
    }
};

static bool is_exist = false;

IEngine*    create_engine()
{
    if (is_exist)
    {
        throw runtime_error("Engine exists!");
    }
    is_exist = true;
    cout << "Create Engine!\n";
    return new engine();
}

void delete_engine(IEngine* engine)
{
    if (is_exist)
    {
        if (engine == nullptr)
        {
            throw runtime_error("Engine is nullptr");
        }
        else
        {
            is_exist = false;
            delete engine;
            cout << "Engine was deleted!\n";
        }
    }
    else
    {
        throw runtime_error("Engine does not exist!");
    }
}

void check_gl_errors()
{
    GLenum num_of_error = glGetError();
    if (num_of_error != GL_NO_ERROR)
    {
        switch (num_of_error)
        {
        case GL_INVALID_ENUM:
            cerr << "Error: GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            cerr << "Error: GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            cerr << "Error: GL_INVALID_OPERATION";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            cerr << "Error: GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            cerr << "Error: GL_OUT_OF_MEMORY";
            break;
        case GL_STACK_OVERFLOW:
            cerr << "Error: GL_STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            cerr << "Error: GL_STACK_UNDERFLOW";
            break;
        }
    }
}