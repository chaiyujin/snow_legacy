#pragma once
#include "snow_window.h"
#include <map>
#include <vector>
#include <string>

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
        bool                                mRunning;
        SDL_Event                           mEvent;
        std::map<std::string, Window *>     mWindowPtrDict;
        std::map<std::string, Settings>     mWindowSettings;
    public:
        App(int glMajorVersion=3, int glMinorVersion=3, std::string glslVersion="");
        ~App();

        /***
         * Add window to app. Please let the app take care of the pointer,
         * just new a window and pass the ptr into this function.
         * 
         * 
         * */
        void addWindow(std::string name, Window *ptr);
        void run();

        void _loadSettings();
        void _saveSettings();

        static bool AskQuit();
    };
}
