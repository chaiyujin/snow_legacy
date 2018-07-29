#pragma once
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <iostream>
#include <string>

namespace snow {
    class Snow
    {
    private:
        ImVec4          clear_color;
        bool            done;
        SDL_Window      *window;
        SDL_GLContext   gl_context;
        Snow(int width, int height, std::string &title, ImVec4 clear_color, const char *glsl_version);

        static Snow * g_instance;
    public:
        static void Initialize(int width=1280, int height=720,
                            std::string title="Window",
                            ImVec4 clear_color=ImVec4(0.1, 0.1, 0.1, 1.0)) {
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
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
            // Create window with graphics context
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            
            g_instance = new Snow(width, height, title, clear_color, glsl_version);
        }

        static bool Alive() {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type == SDL_QUIT)
                    g_instance->done = true;
                if (event.type == SDL_WINDOWEVENT &&
                    event.window.event == SDL_WINDOWEVENT_CLOSE &&
                    event.window.windowID == SDL_GetWindowID(g_instance->window))
                    g_instance->done = true;
            }

            if (!g_instance->done) {
                // swap buffer and clear color
                auto &io = ImGui::GetIO();
                SDL_GL_SwapWindow(g_instance->window);
                SDL_GL_MakeCurrent(g_instance->window, g_instance->gl_context);
                glClearColor(g_instance->clear_color.x, g_instance->clear_color.y,
                             g_instance->clear_color.z, g_instance->clear_color.w);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
            }

            if (!g_instance->done) {
                // new gui frame
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplSDL2_NewFrame(g_instance->window);
                ImGui::NewFrame();
            }

            return !g_instance->done;
        }

        static void DrawGui() {
            auto &io = ImGui::GetIO();
            ImGui::Render();
            glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        static void Terminate() {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplSDL2_Shutdown();
            ImGui::DestroyContext();

            SDL_GL_DeleteContext(g_instance->gl_context);
            SDL_DestroyWindow(g_instance->window);
            SDL_Quit();
            
            delete g_instance;
            g_instance = nullptr;
        }
    };
}