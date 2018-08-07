#include <fstream>
#include <regex>
// snow
#include "snow_app.h"
#include "../core/snow_string.h"

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
        AbstractWindow::Initialize(major, minor, glslVersion);
        this->_loadSettings();
    }

    App::~App() {
        for (auto it = mWindowPtrDict.begin(); it != mWindowPtrDict.end(); ++it) {
            delete it->second;
        }
    }

    void App::addWindow(AbstractWindow *ptr) {
        if (ptr == nullptr) return;
        std::string name = SDL_GetWindowTitle(ptr->windowPtr());
        if (mWindowPtrDict.find(name) != mWindowPtrDict.end()) {
            std::cerr << "[App]: two windows have same name: " << name << std::endl;
            throw std::runtime_error("[App]: two windows have same name.");
        }
        mWindowPtrDict.insert(std::pair<std::string, AbstractWindow*>(name, ptr));
        auto it = mWindowSettings.find(name);
        if (it != mWindowSettings.end()) {
            const auto &sets = it->second;
            SDL_SetWindowSize(ptr->windowPtr(), sets.width, sets.height);
            SDL_SetWindowPosition(ptr->windowPtr(), sets.x, sets.y);
        }
        SDL_RaiseWindow(ptr->windowPtr());
    }

    void App::run() {
        // vsync for multi-windows
        SDL_GL_SetSwapInterval(-1);
        // raise the first added window
        SDL_RaiseWindow(mWindowPtrDict.begin()->second->windowPtr());

        mRunning = true;
        SDL_Event mEvent;
        while (mRunning) {
            while(SDL_PollEvent(&mEvent)) {
                uint32_t windowID = App::GetEventID(mEvent);
                for (auto it = mWindowPtrDict.begin(); it != mWindowPtrDict.end(); ++it) {
                    AbstractWindow *p = it->second;
                    if (windowID == 0 || windowID == SDL_GetWindowID(p->windowPtr()))
                        p->_processEvent(mEvent);
                    if (mEvent.type == SDL_QUIT || (mEvent.type == SDL_WINDOWEVENT &&
                                                    mEvent.window.event == SDL_WINDOWEVENT_CLOSE &&
                                                    windowID == SDL_GetWindowID(p->windowPtr())))
                        mRunning = false;
                }
            }
            if (!mRunning) {
                mRunning = !App::AskQuit();
                if (!mRunning) break;
            }

            /* draw */
            for (auto it = mWindowPtrDict.begin(); it != mWindowPtrDict.end(); ++it) {
                it->second->_draw();
            }
        }
        this->_saveSettings();
        SDL_Quit();
    }

    void App::_loadSettings() {

        auto parseSettings = [](std::string &line, std::ifstream &fin) -> Settings {
            Settings sets;
            line = line.substr(1, line.length() - 2);
            std::string str;
            std::getline(fin, str); Trim(str);
            sscanf(str.c_str(), "Pos=%d,%d", &sets.x, &sets.y);
            std::getline(fin, str); Trim(str);
            sscanf(str.c_str(), "Size=%d,%d", &sets.width, &sets.height);
            if (sets.x < 0) sets.x = SDL_WINDOWPOS_CENTERED;
            if (sets.y < 0) sets.y = SDL_WINDOWPOS_CENTERED;
            return sets;
        };

        std::ifstream fin("snowapp.ini");
        std::regex re_title("(\\[)(.*)(\\])");
        if (fin.is_open()) {
            std::string line;
            while (!fin.eof()) {
                std::getline(fin, line); Trim(line);
                if (std::regex_match(line, re_title)) {
                    // read title
                    Settings sets = parseSettings(line, fin);
                    mWindowSettings.insert(std::pair<std::string, Settings>(line, sets));
                }
            }
            fin.close();
        }
    }

    void App::_saveSettings() {
        std::ofstream fout("snowapp.ini");
        if (fout.is_open()) {
            for (auto it=mWindowPtrDict.begin(); it != mWindowPtrDict.end(); ++it) {
                SDL_Window *p = it->second->windowPtr();
                std::string title = SDL_GetWindowTitle(p);
                int x, y, w, h;
                SDL_GetWindowPosition(p, &x, &y);
                SDL_GetWindowSize(p, &w, &h);
                fout << "[" << title << "]\n"
                     << "Pos=" << x << "," << y << "\n"
                     << "Size=" << w << "," << h << "\n\n";
            }
            fout.close();
        }
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


    uint32_t App::GetEventID(SDL_Event &event) {
        switch (event.type) {
        case SDL_DOLLARGESTURE:
        case SDL_DOLLARRECORD:              return event.dgesture.touchId;
        case SDL_DROPFILE:
        case SDL_DROPTEXT:
        case SDL_DROPBEGIN:
        case SDL_DROPCOMPLETE:              return event.drop.windowID;
        case SDL_FINGERMOTION:
        case SDL_FINGERDOWN:
        case SDL_FINGERUP:                  return event.tfinger.touchId;
        case SDL_KEYDOWN:
        case SDL_KEYUP:                     return event.key.windowID;
        case SDL_MOUSEMOTION:               return event.motion.windowID;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:             return event.button.windowID;
        case SDL_MOUSEWHEEL:                return event.wheel.windowID;
        case SDL_TEXTEDITING:               return event.edit.windowID;
        case SDL_TEXTINPUT:                 return event.text.windowID;
        case SDL_USEREVENT:                 return event.user.windowID;
        case SDL_WINDOWEVENT:               return event.window.windowID;
        default:                            return 0;
        }
    }
}