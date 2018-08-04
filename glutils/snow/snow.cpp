#include "snow.h"

namespace snow {
    Snow * Snow::g_instance = nullptr;

    Snow::Snow(int width, int height, std::string &title, ImVec4 clear_color, const char *glsl_version) {
        SDL_DisplayMode current;
        SDL_GetCurrentDisplayMode(0, &current);        
        this->window = SDL_CreateWindow(title.c_str(),
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        width, height,
                                        SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
        this->gl_context = SDL_GL_CreateContext(this->window);
        SDL_GL_SetSwapInterval(1); // Enable vsync
        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
            throw(std::string("Failed to initialize GLAD"));
        }
        // Setup Dear ImGui binding
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        ImGui_ImplSDL2_InitForOpenGL(this->window, this->gl_context);
        ImGui_ImplOpenGL3_Init(glsl_version);
        // Setup style
        ImGui::StyleColorsDark();

        // Load Fonts
        // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them. 
        // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple. 
        // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
        // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
        // - Read 'misc/fonts/README.txt' for more instructions and details.
        // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
        //io.Fonts->AddFontDefault();
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
        //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
        //IM_ASSERT(font != NULL);

        this->done = false;
        this->clear_color = clear_color;
    }
}