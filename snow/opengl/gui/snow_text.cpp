#include "snow_text.h"

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

bool Text::gInited = false;
FT_Library Text::gFTLib;
FT_Face    Text::gFTFace;
snow::Shader Text::gShader;
GLuint Text::VAO=0, Text::VBO=0;
std::map<GLchar, Character> Text::gCharacters;

void Text::Initialize(const char *font) {
    if (gInited) return;
    gInited = true;
    if (FT_Init_FreeType(&gFTLib))
        snow::fatal("FreeType: Could not init FreeType Library.");
    if (FT_New_Face(gFTLib, font, 0, &gFTFace))
        snow::fatal("FreeType: Failed to load font.");
    FT_Set_Pixel_Sizes(gFTFace, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
    
    for (GLubyte c = 0; c < 128; c++) {
        // Load character glyph 
        if (FT_Load_Char(gFTFace, c, FT_LOAD_RENDER)) {
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
            gFTFace->glyph->bitmap.width,
            gFTFace->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            gFTFace->glyph->bitmap.buffer
        );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture, 
            glm::ivec2(gFTFace->glyph->bitmap.width, gFTFace->glyph->bitmap.rows),
            glm::ivec2(gFTFace->glyph->bitmap_left, gFTFace->glyph->bitmap_top),
            (GLuint)gFTFace->glyph->advance.x
        };
        gCharacters.insert(std::pair<GLchar, Character>(c, character));
    }

    FT_Done_Face(gFTFace);
    FT_Done_FreeType(gFTLib);

    gShader.buildFromCode(VERT_CODE, FRAG_CODE);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Text::RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    
    Shader &s = gShader;
    s.use();
    // Activate corresponding render state
    s.setMat4("projection", glm::ortho(0.0f, 800.0f, 0.0f, 600.0f));
    s.setVec3("textColor", color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = gCharacters[*c];

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
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
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
