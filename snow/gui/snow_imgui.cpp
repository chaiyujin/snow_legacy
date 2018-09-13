#include <iostream>
// snow
#include "snow_imgui.h"
#include <SDL2/SDL_syswm.h>

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
        // ImGui::StyleColorsDark();  // default color theme
        ImGui::StyleColorsLight();
        // ImGui::StyleColorsClassic();
    }

    void ImGuiSDL2::activate() {
        ImGui::SetCurrentContext(mImGuiContext);
    }

    void ImGuiSDL2::newFrame() {
        this->activate();
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

    void ImGuiSDL2::endFrame() {
        this->activate();
        ImGui::Render();
        ImGuiIO &io = ImGui::GetIO();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        this->_renderDrawData();
    }

    void ImGuiSDL2::processEvent(SDL_Event &event) {
        this->activate();
        ImGuiIO& io = ImGui::GetIO();
        bool wantMouse = io.WantCaptureMouse;
        bool wantKeyboard = io.WantCaptureKeyboard;

        switch (event.type) {
        case SDL_MOUSEWHEEL:
            {
                if (event.wheel.x > 0) io.MouseWheelH += 1;
                if (event.wheel.x < 0) io.MouseWheelH -= 1;
                if (event.wheel.y > 0) io.MouseWheel += 1;
                if (event.wheel.y < 0) io.MouseWheel -= 1;
                if (wantMouse) event.type = 0;  // filter
                return;
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if (event.button.button == SDL_BUTTON_LEFT) gMousePressed[0] = true;
                if (event.button.button == SDL_BUTTON_RIGHT) gMousePressed[1] = true;
                if (event.button.button == SDL_BUTTON_MIDDLE) gMousePressed[2] = true;
                if (wantMouse) event.type = 0;  // filter
                return;
            }
        case SDL_TEXTINPUT:
            {
                io.AddInputCharactersUTF8(event.text.text);
                return;
            }
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            {
                int key = event.key.keysym.scancode;
                IM_ASSERT(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown));
                io.KeysDown[key] = (event.type == SDL_KEYDOWN);
                io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
                io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
                io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
                io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
                if (wantKeyboard) event.type = 0;  // filter
                return;
            }
        }
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
        SDL_GetWindowWMInfo(mWindowPtr, &wmInfo);
        io.ImeWindowHandle = wmInfo.info.win.window;
#else
        (void)mWindowPtr;
#endif
    }

    void ImGuiSDL2::_updateMouseCursor() {
        SDL_Window *focus_window = SDL_GetMouseFocus();
        if (focus_window != mWindowPtr)
            return;

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

        SDL_Window *focus_window = SDL_GetMouseFocus();
        if (focus_window != mWindowPtr)
            return;
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

    void ImGuiSDL2::_renderDrawData() {
        ImDrawData *draw_data = ImGui::GetDrawData();
        // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
        ImGuiIO& io = ImGui::GetIO();
        int fb_width = (int)(draw_data->DisplaySize.x * io.DisplayFramebufferScale.x);
        int fb_height = (int)(draw_data->DisplaySize.y * io.DisplayFramebufferScale.y);
        if (fb_width <= 0 || fb_height <= 0)
            return;
        draw_data->ScaleClipRects(io.DisplayFramebufferScale);

        // Backup GL state
        GLenum last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
        glActiveTexture(GL_TEXTURE0);
        GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        GLint last_sampler; glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
        GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
        GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
        GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
        GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
        GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
        GLenum last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
        GLenum last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
        GLenum last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
        GLenum last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
        GLenum last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
        GLenum last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
        GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
        GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
        GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
        GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

        // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_SCISSOR_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // Setup viewport, orthographic projection matrix
        // Our visible imgui space lies from draw_data->DisplayPps (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayMin is typically (0,0) for single viewport apps.
        glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
        float L = draw_data->DisplayPos.x;
        float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
        float T = draw_data->DisplayPos.y;
        float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
        const float ortho_projection[4][4] =
        {
            { 2.0f/(R-L),   0.0f,         0.0f,   0.0f },
            { 0.0f,         2.0f/(T-B),   0.0f,   0.0f },
            { 0.0f,         0.0f,        -1.0f,   0.0f },
            { (R+L)/(L-R),  (T+B)/(B-T),  0.0f,   1.0f },
        };
        glUseProgram(mShaderHandle);
        glUniform1i(mAttribLocationTex, 0);
        glUniformMatrix4fv(mAttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
        if (glBindSampler) glBindSampler(0, 0); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.

        // Recreate the VAO every time 
        // (This is to easily allow multiple GL contexts. VAO are not shared among GL contexts, and we don't track creation/deletion of windows so we don't have an obvious key to use to cache them.)
        GLuint vao_handle = 0;
        glGenVertexArrays(1, &vao_handle);
        glBindVertexArray(vao_handle);
        glBindBuffer(GL_ARRAY_BUFFER, mVBOHandle);
        glEnableVertexAttribArray(mAttribLocationPosition);
        glEnableVertexAttribArray(mAttribLocationUV);
        glEnableVertexAttribArray(mAttribLocationColor);
        glVertexAttribPointer(mAttribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
        glVertexAttribPointer(mAttribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
        glVertexAttribPointer(mAttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col));

        // Draw
        ImVec2 pos = draw_data->DisplayPos;
        for (int n = 0; n < draw_data->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];
            const ImDrawIdx* idx_buffer_offset = 0;

            glBindBuffer(GL_ARRAY_BUFFER, mVBOHandle);
            glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementsHandle);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
            {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                if (pcmd->UserCallback)
                {
                    // User callback (registered via ImDrawList::AddCallback)
                    pcmd->UserCallback(cmd_list, pcmd);
                }
                else
                {
                    ImVec4 clip_rect = ImVec4(pcmd->ClipRect.x - pos.x, pcmd->ClipRect.y - pos.y, pcmd->ClipRect.z - pos.x, pcmd->ClipRect.w - pos.y);
                    if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
                    {
                        // Apply scissor/clipping rectangle
                        glScissor((int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));

                        // Bind texture, Draw
                        glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                        glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
                    }
                }
                idx_buffer_offset += pcmd->ElemCount;
            }
        }
        glDeleteVertexArrays(1, &vao_handle);

        // Restore modified GL state
        glUseProgram(last_program);
        glBindTexture(GL_TEXTURE_2D, last_texture);
        if (glBindSampler) glBindSampler(0, last_sampler);
        glActiveTexture(last_active_texture);
        glBindVertexArray(last_vertex_array);
        glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
        glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
        glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
        if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
        if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
        if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
        glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
        glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
    }
}
