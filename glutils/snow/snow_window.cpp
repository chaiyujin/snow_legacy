#include "snow_window.h"

namespace snow {
    bool        AbstractWindow::gIsGLADLoaded  = false;
    std::string AbstractWindow::gGLSLVersion   = "";
    void AbstractWindow::Initialize(int major, int minor, std::string glslVersion) {
        if (gGLSLVersion.length() > 0) {
            std::cout << "[SDLWindow]: SDL2 has been initialized already.";
            return;
        }
        if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
            std::cerr << "Error: " << SDL_GetError() << std::endl;
            throw std::runtime_error("SDL2 initialization error.");
        }
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
        // Create window with graphics context
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        gGLSLVersion = glslVersion;
    }

    AbstractWindow::~AbstractWindow() {
        SDL_GL_DeleteContext(mGLContext);
        SDL_DestroyWindow(mWindowPtr);
        printf("~AbsWin\n");
    }

    void AbstractWindow::GLADInit() {
        if (!gIsGLADLoaded) {
            // glad loading
            if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
                std::cerr << "Failed to initialize GLAD" << std::endl;
                throw std::runtime_error("Failed to initialize GLAD");
            }
            gIsGLADLoaded = true;
        }
    }

    AbstractWindow::AbstractWindow(int width, int height, const char *title, int x, int y)
        : mWidth(width), mHeight(height), mIsFocused(true)
    {
        if (gGLSLVersion.length() == 0) {
            std::cerr << "[SDLWindow]: Please initialize or create an App before create a window.";
            throw std::runtime_error("[SDLWindow]: Please initialize or create an App before create a window.");
        }
        mWindowPtr = SDL_CreateWindow(title, x, y, width, height, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
        if (mWindowPtr != nullptr) {
            mGLContext = SDL_GL_CreateContext(mWindowPtr);
            GLADInit();
            mImGui.init(mWindowPtr, mGLContext, gGLSLVersion);
        }
        else {
            std::cerr << "[SDLWindow]: Failed to create window.";
            throw std::runtime_error("[SDLWindow]: Failed to create window.");
        }
    }

    void AbstractWindow::_processEvent(SDL_Event &event) {
        this->mImGui.processEvent(event);
        this->processEvent(event);
    }

    void AbstractWindow::_draw() {
        this->glMakeCurrent();
        this->mImGui.newFrame();
        this->draw();
        this->mImGui.endFrame();
        SDL_GL_SwapWindow(mWindowPtr);
    }
}