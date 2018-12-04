#include "snow_window.h"
#include "snow_text.h"

namespace snow {

std::string Shader::GLSLVersion = "";

bool        AbstractWindow::gIsGLADLoaded  = false;
std::string AbstractWindow::gGLSLVersion   = "";
void AbstractWindow::Initialize(int major, int minor, std::string glslVersion) {
    if (gGLSLVersion.length() > 0) {
        std::cout << "[SDLWindow]: SDL2 has been initialized already.";
        return;
    }
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
        snow::fatal("SDL2 initialization error: {0}", SDL_GetError());
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
    // set shader
    Shader::GLSLVersion = gGLSLVersion;
}

void AbstractWindow::Terminate() {
    gGLSLVersion = "";
}

AbstractWindow::~AbstractWindow() {
    SDL_GL_DeleteContext(mGLContext);
    SDL_DestroyWindow(mWindowPtr);
}

void AbstractWindow::GLADInit() {
    if (!gIsGLADLoaded) {
        // glad loading
        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
            snow::fatal("Failed to initialize GLAD");
        }
        gIsGLADLoaded = true;
    }
}

AbstractWindow::AbstractWindow(const char *title, int width, int height, int x, int y)
    : mWidth(width), mHeight(height), mRatio(-1.0)
{
    if (gGLSLVersion.length() == 0) {
        snow::fatal("[SDLWindow]: Please initialize or create an App before create a window.");
    }
    mTag   = title;
    mTitle = title;
    mWindowPtr = SDL_CreateWindow(title, x, y, width, height, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    if (mWindowPtr != nullptr) {
        mGLContext = SDL_GL_CreateContext(mWindowPtr);
        this->glMakeCurrent();
        GLADInit();
        mImGui.init(mWindowPtr, mGLContext, gGLSLVersion);
        Text::Initialize("../../../arial.ttf");
    }
    else {
        snow::fatal("[SDLWindow]: Failed to create window.");
    }
    this->hide();
}

void AbstractWindow::_processEvent(SDL_Event &event) {
    // update window size
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        this->_resize(event.window.data1, event.window.data2);
    }
    // gui process
    this->mImGui.processEvent(event);
    // custom process
    this->processEvent(event);
}

void AbstractWindow::_draw(snow::Image *image) {
    this->glMakeCurrent();
    this->mImGui.newFrame();
    // before custom draw, set view port();
    this->_viewport();
    this->draw();  // custom draw

    Text::RenderText("(C) LearnOpenGL.com", 25.0f, 25.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));

    this->mImGui.endFrame();
    if (image != nullptr) {
        image->resize(this->mWidth, this->mHeight, 4);
        glReadPixels(0, 0, this->mWidth, this->mHeight, GL_RGBA, GL_UNSIGNED_BYTE, image->data());
        snow::Image::Flip(*image, 0);
    }
    SDL_GL_SwapWindow(mWindowPtr);
}

void AbstractWindow::_resize(int w, int h) {
    mWidth  = w;
    mHeight = h;
    _viewport();
}

void AbstractWindow::_viewport() {
    auto area = this->validArea();
    glViewport((GLsizei)area[0], (GLsizei)area[1],
               (GLsizei)area[2], (GLsizei)area[3]);
}

std::vector<int> AbstractWindow::validArea() const {
    if (mRatio <= 0.0) { return { 0, 0, mWidth, mHeight}; }
    else {
        int glW = mWidth, glH = mHeight;
        int glL = 0, glT = 0;
        if (mRatio * mHeight > mWidth) { // smaller h
            glH = int((float)mWidth / mRatio);
            glT = (mHeight - glH) / 2;
        }
        else { // smaller w
            glW = int((float)mHeight * mRatio);
            glL = (mWidth - glW) / 2;
        }
        return {glL, glT, glW, glH};
    }
}

void AbstractWindow::resize(int w, int h) {
    this->glMakeCurrent();
    SDL_SetWindowSize(this->mWindowPtr, w, h);
    this->_resize(w, h);
}

glm::mat4 AbstractWindow::perspective(const CameraBase *camera) {
    return glm::perspective(glm::radians(camera->zoom()), this->ratio(), 0.1f, 100.0f);
}

void CameraWindow::_processEvent(SDL_Event &event) {
    AbstractWindow::_processEvent(event);
    mCamera.processMouseEvent(event);
}

void CameraWindow::_draw(snow::Image *image) {
    this->glMakeCurrent();
    this->mImGui.newFrame();
    // before custom draw, set view port();
    this->_viewport();
    this->draw();  // custom draw
    {   // draw arch ball and gui
        if (DrawArcball) {
            this->mCamera.arcballPtr()->draw(this->perspective(&mCamera));
            this->mCamera.setSpeedMove(MoveSpeed);
            this->mCamera.setSpeedZoom(ZoomSpeed);
            this->mCamera.setSpeedRotate(RotateSpeed);
        }
        ImGui::Begin("Camera");
        ImGui::Checkbox("Draw Arcball", &DrawArcball);
        ImGui::SliderFloat("Speed Move", &MoveSpeed,     0.5f, 10.f);
        ImGui::SliderFloat("Speed Zoom", &ZoomSpeed,     0.1f,  5.f);
        ImGui::SliderFloat("Speed Rotate", &RotateSpeed, 0.1f,  5.f);
        ImGui::End();
    }
    this->mImGui.endFrame();
    if (image != nullptr) {
        image->resize(this->mWidth, this->mHeight, 4);
        glReadPixels(0, 0, this->mWidth, this->mHeight, GL_RGBA, GL_UNSIGNED_BYTE, image->data());
        snow::Image::Flip(*image, 0);
    }
    SDL_GL_SwapWindow(mWindowPtr);
}

}
