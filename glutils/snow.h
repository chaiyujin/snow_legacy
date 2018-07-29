#pragma once
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <iostream>
#include <string>

class Snow
{
private:
    ImVec4  clear_color;
    bool    done;
    SDL_Window *window;
    SDL_GLContext gl_context;
    Snow(int width, int height, std::string &title, ImVec4 clear_color) { 
        clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        done = false;
    }
    static Snow * g_instance;
public:
    static void initialize() {
        if (g_instance != nullptr) {
            std::cerr << "Initialize twice." << std::endl;
            throw std::runtime_error("SDL initialization error.");
        }
        // setup sdl2
        if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
            std::cerr << "Error: " << SDL_GetError() << std::endl;
            throw std::runtime_error("SDL initialization error.");
        }
        const char* glsl_version = "#version 330";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        // Create window with graphics context
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_DisplayMode current;
        SDL_GetCurrentDisplayMode(0, &current);        
        SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
        SDL_GLContext gl_context = SDL_GL_CreateContext(window);
        SDL_GL_SetSwapInterval(1); // Enable vsync
        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
            throw(std::string("Failed to initialize GLAD"));
        }
        // Setup Dear ImGui binding
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
        ImGui_ImplOpenGL3_Init(glsl_version);
        // Setup style
        ImGui::StyleColorsDark();

        g_instance->done = false;
        g_instance->clear_color = clear_color;
    }

    static bool alive() {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                g_instance->done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                g_instance->done = true;
        }
        return !g_instance->done;
    }

    static void swap_buffer() {
        ImGui::Render();
        SDL_GL_MakeCurrent(window, gl_context);
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    static void terminate() {

    }
};