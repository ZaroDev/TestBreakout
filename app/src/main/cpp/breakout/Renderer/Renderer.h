

#ifndef TESTBREAKOUT_RENDERER_H
#define TESTBREAKOUT_RENDERER_H

struct android_app;

#include <EGL/egl.h>
#include <memory>

#include "Renderer/Model.h"
#include "Renderer/Shader.h"
#include "ECS/Scene.h"
#include "Fonts.h"


class Renderer {
public:
    Renderer() :
            m_Display(EGL_NO_DISPLAY),
            m_Surface(EGL_NO_SURFACE),
            m_Context(EGL_NO_CONTEXT),
            m_Width(0),
            m_Height(0),
            m_ShaderNeedsNewProjectionMatrix(true) {}

    ~Renderer();

    void initialize(android_app *app);

    void render(const Scene &scene, bool clear = true);

    void flush();

    void shutdown();

    u32 loadTexture(const std::string& path);
    std::shared_ptr<TextureAsset> getTexture(u32 id) const { return m_Textures[id]; }

    u32 width() const { return m_Width;}
    u32 height() const { return m_Height; }
private:
    /*!
     * @brief we have to check every frame to see if the framebuffer has changed in size. If it has,
     * update the viewport accordingly
     */
    void updateRenderArea();

    /*!
     * Creates the models for this sample. You'd likely load a scene configuration from a file or
     * use some other setup logic in your full game.
     */
    void createModels();


    EGLDisplay m_Display;
    EGLSurface m_Surface;
    EGLContext m_Context;
    EGLint m_Width;
    EGLint m_Height;

    bool m_ShaderNeedsNewProjectionMatrix = true;

    std::unique_ptr <Shader> m_Shaders;

    std::vector<std::shared_ptr<TextureAsset>> m_Textures;
    std::unique_ptr<Model> m_SpriteModel;

    Fonts m_Fonts;

    android_app* m_App;
};


#endif //TESTBREAKOUT_RENDERER_H
