#pragma once
#include <map>
#include "../tools/snow_shader.h"
#include "../../core/snow_core.h"
#include <ft2build.h>
#include FT_FREETYPE_H  

namespace snow {

struct Character {
    GLuint      mTextureID;
    glm::ivec2  mSize;
    glm::ivec2  mBearing;
    GLuint      mAdvance;
};

class Text {
    static FT_Library gFTLib;
    static FT_Face    gFTFace;
    static snow::Shader gShader;
    static std::map<GLchar, Character> gCharacters;
    static GLuint VAO, VBO;

public:
    static void Initialize(const char *font);
    static void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
};

}
