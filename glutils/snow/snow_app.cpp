#include "snow_app.h"

namespace snow {
    App::App(int major, int minor, std::string glslVersion)
        : mRunning(false)
    {
        // choose glsl version
        if (glslVersion.length() == 0) {
            glslVersion = "#version ";
            if (major < 3 || (major == 3 && minor < 3))
                glslVersion += "150";
            else if (major == 3 || (major > 3 && minor < 4))
                glslVersion += "330";
            else
                glslVersion += "440";
        }
        Window::Initialize(major, minor, glslVersion);
    }

    App::~App() {
        for (auto it = mWindowPtrDict.begin(); it != mWindowPtrDict.end(); ++it) {
            delete it->second;
        }
    }

    void App::addWindow(std::string name, Window *ptr) {
        if (mWindowPtrDict.find(name) != mWindowPtrDict.end()) {
            std::cerr << "[App]: two windows have same name: " << name << std::endl;
            throw std::runtime_error("[App]: two windows have same name.");
        }
        mWindowPtrDict.insert(std::pair<std::string, Window*>(name, ptr));
    }

    void App::run() {
        mRunning = true;
        SDL_Event mEvent;
        while (mRunning) {
            while(SDL_PollEvent(&mEvent)) {
                for (auto it = mWindowPtrDict.begin(); it != mWindowPtrDict.end(); ++it) {
                    Window *p = it->second;
                    p->_processEvent(mEvent);
                    if (mEvent.type == SDL_QUIT ||
                        (mEvent.type == SDL_WINDOWEVENT && 
                         mEvent.window.event == SDL_WINDOWEVENT_CLOSE &&
                         mEvent.window.windowID == SDL_GetWindowID(p->windowPtr())))
                        mRunning = false;
                }
            }
            if (!mRunning) {
                mRunning = !App::AskQuit();
                if (!mRunning) break;
            }

            /* draw */
            for (auto it = mWindowPtrDict.begin(); it != mWindowPtrDict.end(); ++it)
                it->second->_draw();
        }
        
        SDL_Quit();
    }

    bool App::AskQuit() {
        const SDL_MessageBoxButtonData buttons[] = {
            { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Yes" },
            { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "No" }
        };
        const SDL_MessageBoxColorScheme colorScheme = {
            { /* .colors (.r, .g, .b) */
                /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
                { 255, 255, 255 },
                /* [SDL_MESSAGEBOX_COLOR_TEXT] */
                {  10,  10,  10 },
                /* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
                {  10,  10,  10 },
                /* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
                { 255, 255, 255 },
                /* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
                { 243,   96, 96 }
            }
        };
        const SDL_MessageBoxData messageboxdata = {
            SDL_MESSAGEBOX_INFORMATION, /* .flags */
            NULL, /* .window */
            "Quit app", /* .title */
            "Confirm to quit app?", /* .message */
            SDL_arraysize(buttons), /* .numbuttons */
            buttons, /* .buttons */
            &colorScheme /* .colorScheme */
        };
        int buttonid;
        if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
            throw std::runtime_error("SDL message box error.");
        }
        return (buttonid == 0);
    }
}