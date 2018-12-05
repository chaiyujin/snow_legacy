#include "snow_text.h"
#include "../../fonts/fonts.h"

#define VERT_CODE ""\
    "layout (location = 0) in vec4 vertex;"\
    "out vec2 TexCoords;"\
    "uniform mat4 projection;"\
    "void main()"\
    "{"\
    "    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);"\
    "    TexCoords = vertex.zw;"\
    "}"

#define FRAG_CODE ""\
    "in vec2 TexCoords;"\
    "out vec4 color;"\
    "uniform sampler2D text;"\
    "uniform vec3 textColor;"\
    "void main()"\
    "{"\
    "    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);"\
    "    color = vec4(textColor, 1.0) * sampled;"\
    "}"

namespace snow {

void Text::initialize(const char *font, int fontHeight) {
    if (mInited) return;
    mInited = true;
    if (FT_Init_FreeType(&mFTLib))
        snow::fatal("FreeType: Could not init FreeType Library.");
    // get to get font in 'fonts'
    const unsigned char *mem;
    int size;
    if (getFont(font, &mem, &size)) {
        if (FT_New_Memory_Face(mFTLib, mem, size, 0, &mFTFace))
            snow::fatal("FreeType: Failed to load font.");
    }
    else {
        if (FT_New_Face(mFTLib, font, 0, &mFTFace))
            snow::fatal("FreeType: Failed to load font.");
    }
    FT_Set_Pixel_Sizes(mFTFace, 0, fontHeight);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
    
    for (GLubyte c = 0; c < 128; c++) {
        // Load character glyph 
        if (FT_Load_Char(mFTFace, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            mFTFace->glyph->bitmap.width,
            mFTFace->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            mFTFace->glyph->bitmap.buffer
        );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture, 
            glm::ivec2(mFTFace->glyph->bitmap.width, mFTFace->glyph->bitmap.rows),
            glm::ivec2(mFTFace->glyph->bitmap_left, mFTFace->glyph->bitmap_top),
            (GLuint)mFTFace->glyph->advance.x
        };
        mCharacters.insert(std::pair<GLchar, Character>(c, character));
    }

    FT_Done_Face(mFTFace);
    FT_Done_FreeType(mFTLib);

    mShader.buildFromCode(VERT_CODE, FRAG_CODE);

    glGenVertexArrays(1, &mVAO);
    glGenBuffers(1, &mVBO);
    glBindVertexArray(mVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

GLfloat Text::textLength(std::string text, GLfloat scale) {
    GLfloat x = 0.0;
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        Character ch = mCharacters[*c];
        x += (ch.mAdvance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    return x;
}

void Text::renderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, int win_width, int win_height) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    Shader &s = mShader;
    s.use();
    // Activate corresponding render state
    s.setMat4("projection", glm::ortho(0.0f, (float)win_width, 0.0f, (float)win_height));
    s.setVec3("textColor", color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(mVAO);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = mCharacters[*c];

        GLfloat xpos = x + ch.mBearing.x * scale;
        GLfloat ypos = y - (ch.mSize.y - ch.mBearing.y) * scale;

        GLfloat w = ch.mSize.x * scale;
        GLfloat h = ch.mSize.y * scale;
        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },            
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }           
        };
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.mTextureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.mAdvance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

}
