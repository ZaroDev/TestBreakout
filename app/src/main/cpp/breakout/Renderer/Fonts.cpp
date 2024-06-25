

#include "Fonts.h"
#include "Core/AndroidOut.h"
#include <FileSystem/FileSystem.h>
#include <filesystem>

static const char *vertex = R"vertex(#version 300 es
in vec3 inPosition;
in vec2 inUV;

out vec2 fragUV;

uniform mat4 uProjection;

void main()
{
    gl_Position = uProjection * vec4(inPosition, 1.0);
    fragUV = inUV;
}
)vertex";

static const char *fragment = R"fragment(#version 300 es
precision mediump float;

in vec2 fragUV;

uniform sampler2D uTexture;
uniform vec3 uColor;

out vec4 outColor;

void main() {
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(uTexture, fragUV).r);
    outColor = vec4(uColor, 1.0) * sampled;
}
)fragment";
const char* getErrorMessage(FT_Error err)
{
#undef FTERRORS_H_
#define FT_ERRORDEF( e, v, s )  case e: return s;
#define FT_ERROR_START_LIST     switch (err) {
#define FT_ERROR_END_LIST       }
#include FT_ERRORS_H
    return "(Unknown error)";
}

void Fonts::initialize() {
    m_Shader = std::unique_ptr<Shader>(
            Shader::loadShader(vertex, fragment, "inPosition", "inUV", "uProjection", "uModel",
                               "uColor"));
    assert(m_Shader);

    if (FT_Init_FreeType(&m_Library)) {
        aout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }
}

Fonts::~Fonts() {
    FT_Done_FreeType(m_Library);
}

void Fonts::loadFont(const std::string &path) {


    FT_Face face;
    FILE *f = android_fopen(path.c_str(), "r");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

    char *string = new char[fsize + 1];
    fread(string, fsize, 1, f);
    fclose(f);
    string[fsize] = 0;

    if (FT_Error error = FT_New_Memory_Face(m_Library, (FT_Byte*)string, fsize, 0, &face)) {
        aout << "ERROR::FREETYPE: Failed to load font: " << getErrorMessage(error) << std::endl;
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (ubyte c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            aout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }

        u32 texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<u32>(face->glyph->advance.x)
        };
        m_Characters.insert(std::pair<char, Character>(c, character));
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    FT_Done_Face(face);
    delete[] string;
}
