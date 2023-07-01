#include <glad.h>
#include <map>

#include "engine.hpp"
#include "my_audio.hpp"
#include "my_math.hpp"
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

void* load_func(const char* name)
{
    SDL_FunctionPointer func_ptr = SDL_GL_GetProcAddress(name);
    return reinterpret_cast<void*>(func_ptr);
}

class engine : public IEngine
{
public:
    void init(int x, int y, vector<float>& norm)
    {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
        {
            throw runtime_error("Could not initialize!");
        }
        width  = x;
        height = y;

        window = SDL_CreateWindow("TANKS", width, height, SDL_WINDOW_OPENGL);

        SDL_SetWindowFullscreen(window, SDL_TRUE);
		
		int   widthN, heightN;
        SDL_GetWindowSizeInPixels(window, &widthN, &heightN);
				
		norm = math.matrix_multiplying(math.scaling_matrix((float)heightN/(float)widthN, 1), math.scaling_matrix(0.4f, 0.4f));

        if (window == nullptr)
        {
            throw runtime_error("Сould not create a window!");
        }
        cout << "Engine was initialized!\n";

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, OPENGL_CONTEXT);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_MAJOR_VERSION);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_MINOR_VERSION);
        opengl_context = SDL_GL_CreateContext(window);

        if (opengl_context == nullptr)
        {
            throw runtime_error("Engine could not create OpenGL ES context!");
        }

        if (!gladLoadGLES2Loader(load_func))
        {
            throw runtime_error(
                "Engine could not initialize OpenGL ES functions!");
        }

        main_program = set_program("./res/shaders/vertex_shader.txt",
                                   "./res/shaders/fragment_shader.txt");

        imgui_program = set_program("./res/shaders/imgui_vertex_shader.txt",
                                    "./res/shaders/imgui_fragment_shader.txt");

        SDL_AudioSpec device_info;

        SDL_AudioSpec want;
        want.freq     = 44100;
        want.format   = SDL_AUDIO_S16;
        want.channels = 2;
        want.samples  = 1024;
        want.callback = my_audio::MyCallBack;
        want.silence  = 0;
        want.userdata = nullptr;
        device        = SDL_OpenAudioDevice(nullptr, 0, &want, &device_info, 0);

        shoot.set_data("./res/music/shot.wav", device_info, false);
        fon.set_data("./res/music/game_music.wav", device_info, true);
        menu.set_data("./res/music/menu_music.wav", device_info, true);

        textures.resize(14);
        textures[0] = set_texture("./res/textures/explosion/01.png");
        textures[1] = set_texture("./res/textures/explosion/02.png");
        textures[2] = set_texture("./res/textures/explosion/03.png");
        textures[3] = set_texture("./res/textures/explosion/04.png");
        textures[4] = set_texture("./res/textures/explosion/05.png");
        textures[5] = set_texture("./res/textures/explosion/06.png");
        textures[6] = set_texture("./res/textures/explosion/07.png");

        textures[7]  = set_texture("./res/textures/tank_part_1.png");
        textures[8]  = set_texture("./res/textures/tank_part_2.png");
        textures[9]  = set_texture("./res/textures/missile.png");
        textures[10] = set_texture("./res/textures/brick_wall.png");
        textures[11] = set_texture("./res/textures/broken_wall.png");
        textures[12] = set_texture("./res/textures/stone_wall.png");
        textures[13] = set_texture("./res/textures/background.png");

        my_audio::sounds.push_back(&menu);
        my_audio::sounds.push_back(&fon);
        my_audio::sounds.push_back(&shoot);
        SDL_PlayAudioDevice(device);
    
        init_gui();
    }

    bool read_events()
    {
        SDL_Event Event;

        buttons.at("MOUSE_CLICK") = 0;
        buttons.at("MOUSE_WHEEL") = 0;

        while (SDL_PollEvent(&Event))
        {
            if (Event.type == SDL_EVENT_QUIT)
            {
                return false;
            }

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
            }

            if (Event.type == SDL_EVENT_MOUSE_WHEEL)
            {
                buttons.at("MOUSE_WHEEL") = Event.wheel.y;
            }
        }
        return true;
    }

    int check_button(string button) { return buttons.at(button); }

    unsigned int get_time() { return (unsigned int)SDL_GetTicks(); }

    void clean()
    {
        texture_clean();
        SDL_free(shoot.audio_buffer);
        SDL_free(fon.audio_buffer);
        SDL_free(menu.audio_buffer);
        SDL_PauseAudioDevice(device);
        SDL_CloseAudioDevice(device);
        glDeleteProgram(main_program);
        glDeleteProgram(imgui_program);
        SDL_DestroyWindow(window);
        SDL_Quit();
        cout << "Engine was cleaned!\n";
    }

    unsigned int set_program(const char* vertex_shader,
                             const char* fragment_shader)
    {
        GLint  linked;
        GLint  vrt_shader  = load_shader(GL_VERTEX_SHADER, vertex_shader);
        GLint  frag_shader = load_shader(GL_FRAGMENT_SHADER, fragment_shader);
        GLuint program     = glCreateProgram();

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

    unsigned int set_texture(const char* path)
    {
        vector<byte> info;
        ifstream     png_file(path, ios_base::binary);
        GLuint       texture_handle;
        if (png_file.good())
        {
            png_file.seekg(0, ios_base::end);
            int end_file = png_file.tellg();
            png_file.seekg(0, ios_base::beg);
            info.resize(end_file);
            png_file.read(reinterpret_cast<char*>(info.data()),
                          static_cast<streamsize>(info.size()));
        }
        else
        {
            cerr << "Something wrong with this file: " << path << endl;
            return false;
        }

        vector<byte>  image_pixels;
        unsigned long w = 0, h = 0;

        if (decodePNG(image_pixels, w, h, info.data(), info.size(), false))
        {
            cerr << "ERROR IN DECODING PNG FILE!!" << endl;
            return false;
        }

        glGenTextures(1, &texture_handle);
        glBindTexture(GL_TEXTURE_2D, texture_handle);
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
        return texture_handle;
    }

    unsigned int get_texture(int n) { return textures[n]; }

    void render_triangle(vector<float> atribute_vertex,
                         vector<float> normolize_matrix,
                         vector<float> shift_matrix,
                         vector<float> buffer_matrix,
                         unsigned int  texture,
                         bool          blending)
    {
        glUseProgram(main_program);
        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE, 32, atribute_vertex.data());

        glVertexAttribPointer(
            1, 3, GL_FLOAT, GL_FALSE, 32, atribute_vertex.data() + 3);

        glVertexAttribPointer(
            2, 2, GL_FLOAT, GL_FALSE, 32, atribute_vertex.data() + 6);

        glEnableVertexAttribArray(0);

        glEnableVertexAttribArray(1);

        glEnableVertexAttribArray(2);

        glActiveTexture(GL_TEXTURE0);

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
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    void texture_clean()
    {
        for (int i = 0; i < textures.size(); i++)
        {
            glDeleteTextures(1, &textures[i]);
        }
    }

    void display()
    {
        ImGui::Render();
        DearImGUI_Render(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    };

    void Render_gui(float*        vertecies,
                    unsigned int* indexes,
                    vector<float> matrix,
                    unsigned int  size)
    {
        glUseProgram(imgui_program);
        glVertexAttribPointer(
            0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, vertecies);

        glVertexAttribPointer(
            1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, vertecies + 2);

        glVertexAttribPointer(
            2, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 8, vertecies + 4);

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
        window_coords = false;
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

                menu.play = true;
            }

            ImGui::End();
            window_coords = false;
        }
    }

    void draw_wiget_elements(bool& is_paused, int score)
    {

        ImGuiIO& io    = ImGui::GetIO();
        float    scale = io.DisplaySize.y / io.DisplaySize.x;

        ImGui::SetNextWindowPos(ImVec2(0, 0));
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

        ///---------------------------------------------------------------------///

        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.79f, 0));
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

        window_coords = true;
    }

    void init_gui()
    {
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGuiIO& io = ImGui::GetIO();

        ImGuiStyle& style             = ImGui::GetStyle();
        style.FrameRounding           = 40.f;
        style.Colors[ImGuiCol_Button] = ImVec4(0.4f, 0.3f, 0.7f, 1.0f);

        io.FontGlobalScale     = 7; // IF FullHD it must be equl 7
        io.BackendPlatformName = "imgui";
        io.Fonts->AddFontDefault();
        create_texture_gui();
    }

    my_math math;

    void DearImGUI_Render(ImDrawData* data)
    {

        ImGuiIO& io = ImGui::GetIO();

        data->ScaleClipRects(io.DisplayFramebufferScale);

        float scale = io.DisplaySize.y / io.DisplaySize.x;

        vector<float> matrix;

        if (!window_coords)
        {
            matrix = math.matrix_multiplying(math.scaling_matrix(scale, 1),
                                             math.shift_matrix(-1.0f, 1.0f));
            matrix = math.matrix_multiplying(
                matrix,
                math.scaling_matrix(2.0f / io.DisplaySize.x,
                                    -2.0f / io.DisplaySize.y));
        }
        else
        {
            matrix = math.matrix_multiplying(
                math.shift_matrix(-1.0f, 1.0f),
                math.scaling_matrix(2.0f / io.DisplaySize.x,
                                    -2.0f / io.DisplaySize.y));
        }

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

                unsigned int color = vertex[j].col;

                vert[k] = (color & 0x000000FF) / 255.f;
                k++;
                vert[k] = ((color & 0x0000FF00) >> 8) / 255.f;
                k++;
                vert[k] = ((color & 0x00FF0000) >> 16) / 255.f;
                k++;
                vert[k] = ((color & 0xFF000000) >> 24) / 255.f;
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
        Uint32 mask = SDL_GetMouseState(&x, &y);

        io.MousePos.x = x;
        io.MousePos.y = y;

        io.MouseDown[0] = (mask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;

        ImGui::NewFrame();
    }

    void shitdown_gui() { ImGui::DestroyContext(); }

    void play_audio(int num) { my_audio::sounds[num]->play_audio(); }

    void play_audio_from_beginning(int num)
    {
        my_audio::sounds[num]->play_audio_from_beginning();
    };

    void stop_audio(int num) { my_audio::sounds[num]->stop_audio(); }

private:
    SDL_Window*   window         = nullptr;
    SDL_GLContext opengl_context = nullptr;
    int           width          = 1000;
    int           height         = 800;

    map<string, int> buttons{ { "A", 0 },           { "W", 0 },
                              { "S", 0 },           { "D", 0 },
                              { "MOUSE_WHEEL", 0 }, { "MOUSE_CLICK", 0 } };

    vector<unsigned int> textures;
    unsigned int         imgui_texture_handle;

    GLuint main_program;
    GLuint imgui_program;

    my_audio::my_audio shoot;
    my_audio::my_audio fon;
    my_audio::my_audio menu;

    bool window_coords = true;

    SDL_AudioDeviceID device;

    string get_source(const char* filename)
    {
        string   source;
        ifstream input_shader_src(filename);
        if (input_shader_src.good())
        {
            input_shader_src.seekg(0, ios::beg);
            source.assign(istreambuf_iterator<char>(input_shader_src),
                          istreambuf_iterator<char>());
            return source;
        }
        else
        {
            cerr << "Something wrong with this file " << filename << endl;
            return "";
        }
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
