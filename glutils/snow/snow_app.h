#pragma once
#include "snow_window.h"
#include <map>
#include <vector>
#include <string>

namespace snow {
    class App {
    private:
        bool                                mRunning;
        SDL_Event                           mEvent;
        std::map<std::string, Window *>     mWindowPtrDict;
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

        static bool AskQuit();
    };
}
