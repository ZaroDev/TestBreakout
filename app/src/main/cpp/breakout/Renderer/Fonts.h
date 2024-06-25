#ifndef _FONTS_H
#define _FONTS_H

#include "Common.h"
#include "TextureAsset.h"
#include "Math/MathTypes.h"
#include <map>
#include "Shader.h"

#include <ft2build.h>
#include FT_FREETYPE_H

struct Character{
    u32 TextureID;
    Iv2 Size;
    Iv2 Bearing;
    u32 Advance;
};

class Fonts {
public:
    Fonts() = default;
    ~Fonts();

    void initialize();
    void loadFont(const std::string& path);

private:
    std::map<char, Character> m_Characters;
    std::unique_ptr<Shader> m_Shader;
    FT_Library m_Library;
    friend class Renderer;
};


#endif //_FONTS_H
