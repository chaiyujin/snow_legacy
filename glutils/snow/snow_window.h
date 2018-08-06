#pragma once
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <iostream>
#include <string>
#include <vector>
#include "snow_imgui.h"

namespace snow {
    class AbstractWindow
    {
    private:
        int                 mWidth;
        int                 mHeight;
        bool                mIsFocused;
        SDL_Window *        mWindowPtr;
        SDL_GLContext       mGLContext;
        ImGuiSDL2           mImGui;

        static bool         gIsGLADLoaded;
        static std::string  gGLSLVersion;
    public:
        /* init */
        static void Initialize(int major, int minor, std::string glslVersion);
        static void GLADInit();

        /* constructor */ 
        AbstractWindow(int width=1280, int height=720, const char *title="SDLOpenGLWindow");
        virtual ~AbstractWindow();
        /* get */
        SDL_Window *        windowPtr() { return mWindowPtr; }
        SDL_GLContext       glContext() { return mGLContext; }
        /* functions */
        void                _processEvent(SDL_Event &event);
        void                _draw();
        void                glMakeCurrent()     { SDL_GL_MakeCurrent(mWindowPtr, mGLContext); }
        /* pure virual methods */
        virtual void        processEvent(SDL_Event &event) = 0;
        virtual void        draw() = 0;
    };

    class Window : public AbstractWindow
    {
    public:
        Window(int width=1280, int height=720, const char *title="SDLOpenGLWindow") : AbstractWindow(width, height, title) {}
        virtual void processEvent(SDL_Event &event) {}
        virtual void draw() {
            static bool show_demo_window = true;
            static bool show_another_window = false;
            // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
            // if (show_demo_window)
            //     ImGui::ShowDemoWindow(&show_demo_window);

            // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
            {
                static float f = 0.0f;
                static int counter = 0;

                ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

                ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
                ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Another Window", &show_another_window);

                ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f    

                if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                    counter++;
                ImGui::SameLine();
                ImGui::Text("counter = %d", counter);

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::End();
            }

            // 3. Show another simple window.
            if (show_another_window)
            {
                ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
                ImGui::Text("Hello from another window!");
                if (ImGui::Button("Close Me"))
                    show_another_window = false;
                ImGui::End();
            }

        }
    };
}
