#include "snow_imgui.h"
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <iostream>

namespace snow {

    bool        ImGuiSDL2::gMousePressed[3] = { false, false, false };
    SDL_Cursor *ImGuiSDL2::gMouseCursors[ImGuiMouseCursor_COUNT] = { nullptr };
    char       *ImGuiSDL2::gClipboardTextData = nullptr;

    static bool CheckShader(GLuint handle, const char* desc)
    {
        GLint status = 0, log_length = 0;
        glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
        glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);
        if (status == GL_FALSE)
            fprintf(stderr, "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to compile %s!\n", desc);
        if (log_length > 0)
        {
            ImVector<char> buf;
            buf.resize((int)(log_length + 1));
            glGetShaderInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
            fprintf(stderr, "%s\n", buf.begin());
        }
        return status == GL_TRUE;
    }

    // If you get an error please report on github. You may try different GL context version or GLSL version.
    static bool CheckProgram(GLuint handle, const char* desc)
    {
        GLint status = 0, log_length = 0;
        glGetProgramiv(handle, GL_LINK_STATUS, &status);
        glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);
        if (status == GL_FALSE)
            fprintf(stderr, "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to link %s!\n", desc);
        if (log_length > 0)
        {
            ImVector<char> buf;
            buf.resize((int)(log_length + 1));
            glGetProgramInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
            fprintf(stderr, "%s\n", buf.begin());
        }
        return status == GL_TRUE;
    }

    void ImGuiSDL2::SetClipboardText(void *, const char *text) {
        SDL_SetClipboardText(text);
    }
    const char *ImGuiSDL2::GetClipboardText(void *) {
        if (gClipboardTextData)
            SDL_free(gClipboardTextData);
        gClipboardTextData = SDL_GetClipboardText();
        return gClipboardTextData;
    }

    ImGuiSDL2::ImGuiSDL2()
        : mTime(0)
        , mWindowPtr(nullptr)
        , mFontTexture(0)
        , mShaderHandle(0), mVertHandle(0), mFragHandle(0)
        , mAttribLocationTex(0), mAttribLocationProjMtx(0)
        , mAttribLocationPosition(0), mAttribLocationUV(0), mAttribLocationColor(0)
        , mVBOHandle(0), mElementsHandle(0) {}

    ImGuiSDL2::ImGuiSDL2(SDL_Window *windowPtr, SDL_GLContext glContext, const std::string &glslVersion)
        : ImGuiSDL2()
    {
        this->init(windowPtr, glContext, glslVersion);
    }

    ImGuiSDL2::~ImGuiSDL2() {
        this->activate();
        _destroyDeviceObjects();
        _destroyFontTextures();
        ImGui::DestroyContext(this->mImGuiContext);
    }

    void ImGuiSDL2::init(SDL_Window *windowPtr, SDL_GLContext glContext, const std::string &glslVersion) {
        if (mWindowPtr != nullptr) {
            std::cout << "[ImGuiSDL2]: The gui has been inited.\n";
            return;
        }
        mWindowPtr = windowPtr;
        // create context
        mImGuiContext = ImGui::CreateContext();
        this->activate();
        // opengl init
        strcpy(mGLSLVersionString, (glslVersion + "\n").c_str());
        // sdl2 init
        (void)glContext;
        _sdl2Init();
        // theme
        ImGui::StyleColorsDark();  // default color theme
    }

    void ImGuiSDL2::activate() {
        ImGui::SetCurrentContext(mImGuiContext);
    }

    void ImGuiSDL2::newFrame() {
        this->activate();
        // new frame for opengl
        if (!mFontTexture)
            this->_createDeviceObjects();
        // new frame for sdl2
        ImGuiIO& io = ImGui::GetIO();
        IM_ASSERT(io.Fonts->IsBuilt());     // Font atlas needs to be built, call renderer _NewFrame() function e.g. ImGui_ImplOpenGL3_NewFrame() 

        // Setup display size (every frame to accommodate for window resizing)
        int w, h;
        int display_w, display_h;
        SDL_GetWindowSize(mWindowPtr, &w, &h);
        SDL_GL_GetDrawableSize(mWindowPtr, &display_w, &display_h);
        io.DisplaySize = ImVec2((float)w, (float)h);
        io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

        // Setup time step (we don't use SDL_GetTicks() because it is using millisecond resolution)
        static Uint64 frequency = SDL_GetPerformanceFrequency();
        Uint64 current_time = SDL_GetPerformanceCounter();
        io.DeltaTime = mTime > 0 ? (float)((double)(current_time - mTime) / frequency) : (float)(1.0f / 60.0f);
        mTime = current_time;

        this->_updateMousePosAndButtons();
        this->_updateMouseCursor();
        ImGui::NewFrame();
    }

    void ImGuiSDL2::draw() {
        this->activate();
        ImGui::Render();
        ImGuiIO &io = ImGui::GetIO();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.2, 0.2, 0.2, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ImGuiSDL2::processEvent(SDL_Event &event) {
        this->activate();
        ImGui_ImplSDL2_ProcessEvent(&event);
    }

    void ImGuiSDL2::_sdl2Init() {
        ImGuiIO &io = ImGui::GetIO();
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;       // We can honor GetMouseCursor() values (optional)
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;        // We can honor io.WantSetMousePos requests (optional, rarely used)

        // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
        io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
        io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
        io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
        io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
        io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
        io.KeyMap[ImGuiKey_Insert] = SDL_SCANCODE_INSERT;
        io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
        io.KeyMap[ImGuiKey_Space] = SDL_SCANCODE_SPACE;
        io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
        io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
        io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
        io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
        io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
        io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
        io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
        io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;

        io.SetClipboardTextFn = ImGuiSDL2::SetClipboardText;
        io.GetClipboardTextFn = ImGuiSDL2::GetClipboardText;
        io.ClipboardUserData = NULL;

        gMouseCursors[ImGuiMouseCursor_Arrow] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
        gMouseCursors[ImGuiMouseCursor_TextInput] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
        gMouseCursors[ImGuiMouseCursor_ResizeAll] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
        gMouseCursors[ImGuiMouseCursor_ResizeNS] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
        gMouseCursors[ImGuiMouseCursor_ResizeEW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
        gMouseCursors[ImGuiMouseCursor_ResizeNESW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
        gMouseCursors[ImGuiMouseCursor_ResizeNWSE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
        gMouseCursors[ImGuiMouseCursor_Hand] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);

#ifdef _WIN32
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        SDL_GetWindowWMInfo(window, &wmInfo);
        io.ImeWindowHandle = wmInfo.info.win.window;
#else
        (void)mWindowPtr;
#endif
    }

    void ImGuiSDL2::_updateMouseCursor() {
        ImGuiIO &io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
            return;
        ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
        if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None)
        {
            // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
            SDL_ShowCursor(SDL_FALSE);
        }
        else
        {
            // // Show OS mouse cursor
            SDL_SetCursor(gMouseCursors[imgui_cursor] ? gMouseCursors[imgui_cursor] : gMouseCursors[ImGuiMouseCursor_Arrow]);
            SDL_ShowCursor(SDL_TRUE);
        }
    }

    void ImGuiSDL2::_updateMousePosAndButtons() {
        ImGuiIO& io = ImGui::GetIO();

        // Set OS mouse position if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
        if (io.WantSetMousePos)
            SDL_WarpMouseInWindow(mWindowPtr, (int)io.MousePos.x, (int)io.MousePos.y);
        else
            io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

        int mx, my;
        Uint32 mouse_buttons = SDL_GetMouseState(&mx, &my);
        io.MouseDown[0] = gMousePressed[0] || (mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;  // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
        io.MouseDown[1] = gMousePressed[1] || (mouse_buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
        io.MouseDown[2] = gMousePressed[2] || (mouse_buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
        gMousePressed[0] = gMousePressed[1] = gMousePressed[2] = false;

    #if SDL_HAS_CAPTURE_MOUSE && !defined(__EMSCRIPTEN__)
        SDL_Window* focused_window = SDL_GetKeyboardFocus();
        if (g_Window == focused_window)
        {
            // SDL_GetMouseState() gives mouse position seemingly based on the last window entered/focused(?)
            // The creation of a new windows at runtime and SDL_CaptureMouse both seems to severely mess up with that, so we retrieve that position globally.
            int wx, wy;
            SDL_GetWindowPosition(focused_window, &wx, &wy);
            SDL_GetGlobalMouseState(&mx, &my);
            mx -= wx;
            my -= wy;
            io.MousePos = ImVec2((float)mx, (float)my);
        }

        // SDL_CaptureMouse() let the OS know e.g. that our imgui drag outside the SDL window boundaries shouldn't e.g. trigger the OS window resize cursor. 
        // The function is only supported from SDL 2.0.4 (released Jan 2016)
        bool any_mouse_button_down = ImGui::IsAnyMouseDown();
        SDL_CaptureMouse(any_mouse_button_down ? SDL_TRUE : SDL_FALSE);
    #else
        if (SDL_GetWindowFlags(mWindowPtr) & SDL_WINDOW_INPUT_FOCUS)
            io.MousePos = ImVec2((float)mx, (float)my);
    #endif
    }

    void ImGuiSDL2::_createDeviceObjects() {
        // Backup GL state
        GLint last_texture, last_array_buffer, last_vertex_array;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

        // Parse GLSL version string
        int glsl_version = 130;
        sscanf(mGLSLVersionString, "#version %d", &glsl_version);

        const GLchar* vertex_shader_glsl_120 =
            "uniform mat4 ProjMtx;\n"
            "attribute vec2 Position;\n"
            "attribute vec2 UV;\n"
            "attribute vec4 Color;\n"
            "varying vec2 Frag_UV;\n"
            "varying vec4 Frag_Color;\n"
            "void main()\n"
            "{\n"
            "    Frag_UV = UV;\n"
            "    Frag_Color = Color;\n"
            "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
            "}\n";

        const GLchar* vertex_shader_glsl_130 =
            "uniform mat4 ProjMtx;\n"
            "in vec2 Position;\n"
            "in vec2 UV;\n"
            "in vec4 Color;\n"
            "out vec2 Frag_UV;\n"
            "out vec4 Frag_Color;\n"
            "void main()\n"
            "{\n"
            "    Frag_UV = UV;\n"
            "    Frag_Color = Color;\n"
            "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
            "}\n";

        const GLchar* vertex_shader_glsl_300_es =
            "precision mediump float;\n"
            "layout (location = 0) in vec2 Position;\n"
            "layout (location = 1) in vec2 UV;\n"
            "layout (location = 2) in vec4 Color;\n"
            "uniform mat4 ProjMtx;\n"
            "out vec2 Frag_UV;\n"
            "out vec4 Frag_Color;\n"
            "void main()\n"
            "{\n"
            "    Frag_UV = UV;\n"
            "    Frag_Color = Color;\n"
            "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
            "}\n";

        const GLchar* vertex_shader_glsl_410_core =
            "layout (location = 0) in vec2 Position;\n"
            "layout (location = 1) in vec2 UV;\n"
            "layout (location = 2) in vec4 Color;\n"
            "uniform mat4 ProjMtx;\n"
            "out vec2 Frag_UV;\n"
            "out vec4 Frag_Color;\n"
            "void main()\n"
            "{\n"
            "    Frag_UV = UV;\n"
            "    Frag_Color = Color;\n"
            "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
            "}\n";

        const GLchar* fragment_shader_glsl_120 =
            "#ifdef GL_ES\n"
            "    precision mediump float;\n"
            "#endif\n"
            "uniform sampler2D Texture;\n"
            "varying vec2 Frag_UV;\n"
            "varying vec4 Frag_Color;\n"
            "void main()\n"
            "{\n"
            "    gl_FragColor = Frag_Color * texture2D(Texture, Frag_UV.st);\n"
            "}\n";

        const GLchar* fragment_shader_glsl_130 =
            "uniform sampler2D Texture;\n"
            "in vec2 Frag_UV;\n"
            "in vec4 Frag_Color;\n"
            "out vec4 Out_Color;\n"
            "void main()\n"
            "{\n"
            "    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
            "}\n";

        const GLchar* fragment_shader_glsl_300_es =
            "precision mediump float;\n"
            "uniform sampler2D Texture;\n"
            "in vec2 Frag_UV;\n"
            "in vec4 Frag_Color;\n"
            "layout (location = 0) out vec4 Out_Color;\n"
            "void main()\n"
            "{\n"
            "    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
            "}\n";

        const GLchar* fragment_shader_glsl_410_core =
            "in vec2 Frag_UV;\n"
            "in vec4 Frag_Color;\n"
            "uniform sampler2D Texture;\n"
            "layout (location = 0) out vec4 Out_Color;\n"
            "void main()\n"
            "{\n"
            "    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
            "}\n";

        // Select shaders matching our GLSL versions
        const GLchar* vertex_shader = NULL;
        const GLchar* fragment_shader = NULL;
        if (glsl_version < 130)
        {
            vertex_shader = vertex_shader_glsl_120;
            fragment_shader = fragment_shader_glsl_120;
        }
        else if (glsl_version == 410)
        {
            vertex_shader = vertex_shader_glsl_410_core;
            fragment_shader = fragment_shader_glsl_410_core;
        }
        else if (glsl_version == 300)
        {
            vertex_shader = vertex_shader_glsl_300_es;
            fragment_shader = fragment_shader_glsl_300_es;
        }
        else
        {
            vertex_shader = vertex_shader_glsl_130;
            fragment_shader = fragment_shader_glsl_130;
        }

        // Create shaders
        const GLchar* vertex_shader_with_version[2] = { mGLSLVersionString, vertex_shader };
        mVertHandle = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(mVertHandle, 2, vertex_shader_with_version, NULL);
        glCompileShader(mVertHandle);
        CheckShader(mVertHandle, "vertex shader");

        const GLchar* fragment_shader_with_version[2] = { mGLSLVersionString, fragment_shader };
        mFragHandle = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(mFragHandle, 2, fragment_shader_with_version, NULL);
        glCompileShader(mFragHandle);
        CheckShader(mFragHandle, "fragment shader");

        mShaderHandle = glCreateProgram();
        glAttachShader(mShaderHandle, mVertHandle);
        glAttachShader(mShaderHandle, mFragHandle);
        glLinkProgram(mShaderHandle);
        CheckProgram(mShaderHandle, "shader program");

        mAttribLocationTex = glGetUniformLocation(mShaderHandle, "Texture");
        mAttribLocationProjMtx = glGetUniformLocation(mShaderHandle, "ProjMtx");
        mAttribLocationPosition = glGetAttribLocation(mShaderHandle, "Position");
        mAttribLocationUV = glGetAttribLocation(mShaderHandle, "UV");
        mAttribLocationColor = glGetAttribLocation(mShaderHandle, "Color");

        // Create buffers
        glGenBuffers(1, &mVBOHandle);
        glGenBuffers(1, &mElementsHandle);

        this->_createFontsTexture();

        // Restore modified GL state
        glBindTexture(GL_TEXTURE_2D, last_texture);
        glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
        glBindVertexArray(last_vertex_array);
    }

    void ImGuiSDL2::_createFontsTexture() {
        ImGuiIO& io = ImGui::GetIO();
        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

        // Upload texture to graphics system
        GLint last_texture;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        glGenTextures(1, &mFontTexture);
        glBindTexture(GL_TEXTURE_2D, mFontTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        // Store our identifier
        io.Fonts->TexID = (void *)(intptr_t)mFontTexture;

        // Restore state
        glBindTexture(GL_TEXTURE_2D, last_texture);
    }

    void ImGuiSDL2::_destroyDeviceObjects() {
        if (mVBOHandle) glDeleteBuffers(1, &mVBOHandle);
        if (mElementsHandle) glDeleteBuffers(1, &mElementsHandle);
        mVBOHandle = mElementsHandle = 0;

        if (mShaderHandle && mVertHandle) glDetachShader(mShaderHandle, mVertHandle);
        if (mVertHandle) glDeleteShader(mVertHandle);
        mVertHandle = 0;

        if (mShaderHandle && mFragHandle) glDetachShader(mShaderHandle, mFragHandle);
        if (mFragHandle) glDeleteShader(mFragHandle);
        mFragHandle = 0;

        if (mShaderHandle) glDeleteProgram(mShaderHandle);
        mShaderHandle = 0;
    }

    void ImGuiSDL2::_destroyFontTextures() {
        if (mFontTexture) {
            ImGuiIO& io = ImGui::GetIO();
            glDeleteTextures(1, &mFontTexture);
            io.Fonts->TexID = 0;
            mFontTexture = 0;
        }
    }
}
