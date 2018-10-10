#pragma once
#include <string>
// thidr-party
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <imgui.h>

namespace snow {

    class ImGuiSDL2
    {
    private:
        /* static */
        static bool         gMousePressed[3];
        static SDL_Cursor  *gMouseCursors[ImGuiMouseCursor_COUNT];
        static char        *gClipboardTextData;
        static void         SetClipboardText(void *, const char *text);
        static const char  *GetClipboardText(void *);

        uint64_t            mTime;
        /* imgui context */
        ImGuiContext       *mImGuiContext;
        /* sdl2 window */
        SDL_Window         *mWindowPtr;
        /* opengl */
        char                mGLSLVersionString[32];
        GLuint              mFontTexture;
        GLuint              mShaderHandle, mVertHandle, mFragHandle;
        int                 mAttribLocationTex, mAttribLocationProjMtx;
        int                 mAttribLocationPosition, mAttribLocationUV, mAttribLocationColor;
        uint32_t            mVBOHandle, mElementsHandle;
    public:
        ImGuiSDL2();
        ImGuiSDL2(SDL_Window *windowPtr, SDL_GLContext glContext, const std::string &glslVersion);
        ~ImGuiSDL2();

        void init(SDL_Window *windowPtr, SDL_GLContext glContext, const std::string &glslVersion);
        void activate();
        void newFrame();
        void endFrame();
        // some mouse and keyboard event are filtered, because it's for ImGui not for app.
        void processEvent(SDL_Event &event);
        void _sdl2Init();
        void _createFontsTexture();
        void _createDeviceObjects();
        void _destroyDeviceObjects();
        void _destroyFontTextures();
        void _renderDrawData();
        void _updateMouseCursor();
        void _updateMousePosAndButtons();
    };
}