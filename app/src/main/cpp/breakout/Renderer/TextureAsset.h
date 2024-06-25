#ifndef ANDROIDGLINVESTIGATIONS_TEXTUREASSET_H
#define ANDROIDGLINVESTIGATIONS_TEXTUREASSET_H

#include <memory>
#include <android/asset_manager.h>
#include <GLES3/gl3.h>
#include <string>
#include <vector>

#include "Common.h"

class TextureAsset {
public:
    /*!
     * Loads a texture asset from the assets/ directory
     * @param assetManager Asset manager to use
     * @param assetPath The path to the asset
     * @return a shared pointer to a texture asset, resources will be reclaimed when it's cleaned up
     */
    static std::shared_ptr<TextureAsset>
    loadAsset(AAssetManager *assetManager, const std::string &assetPath);

    ~TextureAsset();

    /*!
     * @return the texture id for use with OpenGL
     */
    constexpr GLuint getTextureID() const { return m_TextureID; }

    constexpr u32 getHeight() const { return m_Height; }

    constexpr u32 getWidth() const { return m_Width; }

private:
    inline TextureAsset(GLuint textureId, u32 width, u32 height)
            : m_TextureID(textureId), m_Width(width), m_Height(height) {}

    GLuint m_TextureID;
    u32 m_Width;
    u32 m_Height;
};

#endif //ANDROIDGLINVESTIGATIONS_TEXTUREASSET_H