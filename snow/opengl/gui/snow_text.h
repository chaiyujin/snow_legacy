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
    bool            mInited;
    FT_Library      mFTLib;
    FT_Face         mFTFace;
    snow::Shader    mShader;
    GLuint          mVAO, mVBO;
    std::map<GLchar, Character> mCharacters;

    void _doneInit();

public:
    const static glm::vec3 BaseColor;
    const static glm::vec3 HighColor;

    Text() : mInited(false) {}
    void initialize(const char *font, int fontHeight=48);
    void renderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, int win_width, int win_height);
    GLfloat textLength(std::string text, GLfloat scale);
};

}
