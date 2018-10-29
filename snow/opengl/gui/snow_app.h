#pragma once
#include <map>
#include <vector>
#include <string>
// snow
#include "../../core/snow_core.h"
#include "snow_window.h"

namespace snow {

    struct Settings {
        int x, y;
        int width, height;
        std::string title;
        Settings() : x(SDL_WINDOWPOS_CENTERED), y(SDL_WINDOWPOS_CENTERED)
                   , width(1280), height(720), title("snow window") {}
    };

    class App {
    private:
        bool                                        mRunning;
        bool                                        mStop;
        SDL_Event                                   mEvent;
        std::map<std::string, AbstractWindow *>     mWindowPtrDict;
        std::map<std::string, Settings>             mWindowSettings;
    public:
        App(int glMajorVersion=3, int glMinorVersion=3, std::string glslVersion="");
        ~App();

        void quit() { mStop = true; }

        /***
         * Add window to app. Please let the app take care of the pointer,
         * just new a window and pass the ptr into this function.
         * */
        void addWindow(AbstractWindow *ptr);
        AbstractWindow * getWindow(std::string name) {
            auto it = mWindowPtrDict.find(name);
            if (it != mWindowPtrDict.end()) return it->second;
            else return nullptr;
        }
        void run();

        void _loadSettings();
        void _saveSettings();

        static bool Query(std::string title, std::string query);
        static uint32_t GetEventID(SDL_Event &);
    };
}
