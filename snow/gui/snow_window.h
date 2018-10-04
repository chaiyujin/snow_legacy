#pragma once
#include <iostream>
#include <string>
#include <vector>
// thidr-party
#include <SDL2/SDL.h>
#include <glad/glad.h>
// snow
#include "snow_imgui.h"
#include "../core/snow_core.h"
#include "../tools/snow_arcball.h"
#include "../tools/snow_camera.h"

namespace snow {
    class AbstractWindow
    {
    protected:
        int                 mWidth;
        int                 mHeight;
        float               mRatio;
        SDL_Window *        mWindowPtr;
        SDL_GLContext       mGLContext;
        ImGuiSDL2           mImGui;
        std::string         mTitle;

        static bool         gIsGLADLoaded;
        static std::string  gGLSLVersion;
    public:
        /* init */
        static void Initialize(int major, int minor, std::string glslVersion);
        static void Terminate();
        static void GLADInit();

        /* constructor */ 
        AbstractWindow(const char *title="SDLOpenGLWindow", int width=1280, int height=720,
                       int x=SDL_WINDOWPOS_CENTERED, int y=SDL_WINDOWPOS_CENTERED);
        virtual ~AbstractWindow();
        /* get */
        SDL_Window *        windowPtr() { return mWindowPtr; }
        SDL_GLContext       glContext() { return mGLContext; }
        int                 width() const { return mWidth; }
        int                 height() const { return mHeight; }
        /* functions */
        void                glMakeCurrent()     { SDL_GL_MakeCurrent(mWindowPtr, mGLContext); }
        glm::mat4           perspective(const CameraBase *camera);
        virtual void        _processEvent(SDL_Event &event);
        virtual void        _draw(snow::Image *image=nullptr);
        /* pure virual methods */
        virtual void        processEvent(SDL_Event &event) = 0;
        virtual void        draw() = 0;
    };

    class ExampleWindow : public AbstractWindow {
    private:
        float f;
        int   counter;
    public:
        ExampleWindow(const char *title="SDLOpenGLWindow", int width=1280, int height=720,
                      int x=SDL_WINDOWPOS_CENTERED, int y=SDL_WINDOWPOS_CENTERED)
            : AbstractWindow(title, width, height, x, y), f(0.f), counter(0) {}
        virtual void processEvent(SDL_Event &event) {}
        virtual void draw() {
            // Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
            {
                ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
                ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
                ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f    
                if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                    counter++;
                ImGui::SameLine();
                ImGui::Text("counter = %d", counter);
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::End();
            }
        }
    };

    class CameraWindow : public AbstractWindow {
    protected:
        ArcballCamera mCamera;
        // gui
        bool  DrawArcball;
        float MoveSpeed, RotateSpeed, ZoomSpeed;
    public:
        CameraWindow(const char *title)
            : AbstractWindow(title)
            , DrawArcball(true)
            , MoveSpeed(5.f), RotateSpeed(1.f), ZoomSpeed(1.f)
        {}

        virtual void _processEvent(SDL_Event &event);
        virtual void _draw(snow::Image *image=nullptr);
        virtual void processEvent(SDL_Event &event) {}
        virtual void draw() {}
    };
}
