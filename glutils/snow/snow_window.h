#pragma once
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <iostream>
#include <string>
#include <vector>

namespace snow {
    class Window
    {
    private:
        int                 mWidth;
        int                 mHeight;
        bool                mIsFocused;
        SDL_Window *        mWindowPtr;
        SDL_GLContext       mGLContext;

        static bool         gIsGLADLoaded;
        static std::string  gGLSLVersion;
    public:
        /* init */
        static void Initialize(int major, int minor, std::string glslVersion);
        static void GLADInit();

        /* constructor */ 
        Window(int width=1280, int height=720, const char *title="SDLOpenGLWindow");
        ~Window();
        /* get */
        SDL_Window *        windowPtr() { return mWindowPtr; }
        SDL_GLContext       glContext() { return mGLContext; }
        /* functions */
        void                processEvent(SDL_Event &event);
        void                draw();
        void                glMakeCurrent()     { SDL_GL_MakeCurrent(mWindowPtr, mGLContext); }
        void                imguiMakeCurrent()  {}
    };
}