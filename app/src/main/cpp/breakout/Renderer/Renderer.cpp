
#include "Renderer.h"

#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <GLES3/gl3.h>
#include <memory>
#include <vector>
#include <android/imagedecoder.h>

#include "Core/AndroidOut.h"
#include "Renderer/Shader.h"
#include "Utils/Utility.h"
#include "Renderer/TextureAsset.h"
#include "ECS/Components.h"

//! executes glGetString and outputs the result to logcat
#define PRINT_GL_STRING(s) {aout << #s": "<< glGetString(s) << std::endl;}

/*!
 * @brief if glGetString returns a space separated list of elements, prints each one on a new line
 *
 * This works by creating an istringstream of the input c-style string. Then that is used to create
 * a vector -- each element of the vector is a new element in the input string. Finally a foreach
 * loop consumes this and outputs it to logcat using @a aout
 */
#define PRINT_GL_STRING_AS_LIST(s) { \
std::istringstream extensionStream((const char *) glGetString(s));\
std::vector<std::string> extensionList(\
        std::istream_iterator<std::string>{extensionStream},\
        std::istream_iterator<std::string>());\
aout << #s":\n";\
for (auto& extension: extensionList) {\
    aout << extension << "\n";\
}\
aout << std::endl;\
}

//! Color for cornflower blue. Can be sent directly to glClearColor
#define CORNFLOWER_BLUE 100 / 255.f, 149 / 255.f, 237 / 255.f, 1

// Vertex shader, you'd typically load this from assets
static const char *vertex = R"vertex(#version 300 es
in vec3 inPosition;
in vec2 inUV;

out vec2 fragUV;

uniform mat4 uProjection;
uniform mat4 uModel;

void main() {
    fragUV = inUV;
    gl_Position = uProjection * uModel * vec4(inPosition, 1.0);
}
)vertex";

// Fragment shader, you'd typically load this from assets
static const char *fragment = R"fragment(#version 300 es
precision mediump float;

in vec2 fragUV;

uniform sampler2D uTexture;
uniform vec3 uColor;

out vec4 outColor;

void main() {
    outColor = vec4(uColor, 1.0) * texture(uTexture, fragUV);
}
)fragment";


/*!
 * Half the height of the projection matrix. This gives you a renderable area of height 4 ranging
 * from -2 to 2
 */
static constexpr float kProjectionHalfHeight = 2.f;

/*!
 * The near plane distance for the projection matrix. Since this is an orthographic projection
 * matrix, it's convenient to have negative values for sorting (and avoiding z-fighting at 0).
 */
static constexpr float kProjectionNearPlane = -1.f;

/*!
 * The far plane distance for the projection matrix. Since this is an orthographic porjection
 * matrix, it's convenient to have the far plane equidistant from 0 as the near plane.
 */
static constexpr float kProjectionFarPlane = 1.f;

void Renderer::initialize(android_app *app) {
    if(app == nullptr){
        aout << "Provided application is null!" << std::endl;
        return;
    }
    m_App = app;
    // Choose your update attributes
    constexpr EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };

    // The default display is probably what you want on Android
    auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, nullptr, nullptr);

    // figure out how many configs there are
    EGLint numConfigs;
    eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);

    // get the list of configurations
    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);

    // Find a config we like.
    // Could likely just grab the first if we don't care about anything else in the config.
    // Otherwise hook in your own heuristic
    auto config = *std::find_if(
            supportedConfigs.get(),
            supportedConfigs.get() + numConfigs,
            [&display](const EGLConfig &config) {
                EGLint red, green, blue, depth;
                if (eglGetConfigAttrib(display, config, EGL_RED_SIZE, &red)
                    && eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &green)
                    && eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blue)
                    && eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &depth)) {

                    aout << "Found config with " << red << ", " << green << ", " << blue << ", "
                         << depth << std::endl;
                    return red == 8 && green == 8 && blue == 8 && depth == 24;
                }
                return false;
            });

    aout << "Found " << numConfigs << " configs" << std::endl;
    aout << "Chose " << config << std::endl;

    // create the proper window surface
    EGLint format;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    EGLSurface surface = eglCreateWindowSurface(display, config, m_App->window, nullptr);

    // Create a GLES 3 context
    EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    EGLContext context = eglCreateContext(display, config, nullptr, contextAttribs);

    // get some window metrics
    auto madeCurrent = eglMakeCurrent(display, surface, surface, context);
    assert(madeCurrent);

    m_Display = display;
    m_Surface = surface;
    m_Context = context;

    // make width and height invalid so it gets updated the first frame in @a updateRenderArea()
    m_Width = -1;
    m_Height = -1;

    PRINT_GL_STRING(GL_VENDOR);
    PRINT_GL_STRING(GL_RENDERER);
    PRINT_GL_STRING(GL_VERSION);
    PRINT_GL_STRING_AS_LIST(GL_EXTENSIONS);

    m_Shaders = std::unique_ptr<Shader>(
            Shader::loadShader(vertex, fragment, "inPosition", "inUV", "uProjection", "uModel", "uColor"));
    assert(m_Shaders);



    // setup any other gl related global states
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // enable alpha globally for now, you probably don't want to do this in a game
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // get some demo models into memory
    createModels();
    m_Fonts.initialize();
    m_Fonts.loadFont("Fonts/Arial.ttf");
}

Renderer::~Renderer() {
    shutdown();
}

void Renderer::render(const Scene &scene, bool clear) {
    // Check to see if the surface has changed size. This is _necessary_ to do every frame when
    // using immersive mode as you'll get no other notification that your renderable area has
    // changed.
    updateRenderArea();

    // When the renderable area changes, the projection matrix has to also be updated. This is true
    // even if you change from the sample orthographic projection matrix as your aspect ratio has
    // likely changed.
    if (m_ShaderNeedsNewProjectionMatrix) {
        // build an orthographic projection matrix for 2d rendering
        Mat4 projection = glm::ortho(0.0f, static_cast<float>(m_Width), static_cast<float>(m_Height), 0.0f, kProjectionNearPlane, kProjectionFarPlane);

        // send the matrix to the shader
        // Note: the shader must be active for this to work. Since we only have one shader for this
        // demo, we can assume that it's active.
        m_Shaders->activate();
        m_Shaders->setProjectionMatrix(projection);

        m_Fonts.m_Shader->activate();
        m_Fonts.m_Shader->setProjectionMatrix(projection);

        // make sure the matrix isn't generated every frame
        m_ShaderNeedsNewProjectionMatrix = false;
    }
    if(clear){
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.25f, 0.5f, 1.0f);
    }

    {
        m_Shaders->activate();
        const auto &view = scene.getAllEntitiesWith<TransformComponent, SpriteComponent>();
        for (const auto &entity: view) {
            const auto &transform = view.get<TransformComponent>(entity);
            const auto &sprite = view.get<SpriteComponent>(entity);

            if(!transform.Enabled){
                continue;
            }

            if (m_Textures.size() <= sprite.Texture) {
                aout << "Error: Texture is not valid!" << std::endl;
                continue;
            }
            const auto &texture = m_Textures[sprite.Texture];
            const auto model = transform.getSpriteTransform();

            m_Shaders->drawModel(model, *m_SpriteModel, *texture, sprite.Color);
        }
        m_Shaders->deactivate();
    }


    {
        m_Fonts.m_Shader->activate();
        const auto& view = scene.getAllEntitiesWith<TransformComponent, TextComponent>();
        for(const auto& entity : view){
            const auto &transform = view.get<TransformComponent>(entity);
            const auto& text = view.get<TextComponent>(entity);

            if(!transform.Enabled){
                continue;
            }

            f32 x = transform.Translation.x;
            for(const auto c : text.Text){
                Character ch = m_Fonts.m_Characters[c];

                const f32 xPos = x + ch.Bearing.x * transform.Scale.x;
                const f32 yPos = transform.Translation.y + (m_Fonts.m_Characters['H'].Bearing.y - ch.Bearing.y) * transform.Scale.y;

                const f32 w = ch.Size.x * transform.Scale.x;
                const f32 h = ch.Size.y * transform.Scale.y;

                std::vector<Vertex> vertices = {
                        Vertex(Vector3{xPos, yPos + h, 0}, Vector2{0, 1}), // 0
                        Vertex(Vector3{xPos, yPos, 0}, Vector2{0, 0}), // 1
                        Vertex(Vector3{xPos + w, yPos, 0}, Vector2{1, 0}), // 2

                        Vertex(Vector3{xPos, yPos + h, 0}, Vector2{0, 1}), // 3
                        Vertex(Vector3{xPos + w, yPos, 0}, Vector2{1, 0}), // 4
                        Vertex(Vector3{xPos + w, yPos + h, 0}, Vector2{1, 1}) // 5
                };

                Model model = Model(vertices,  {});


                m_Fonts.m_Shader->drawModel(model, ch.TextureID, text.Color);

                x += (ch.Advance >> 6) * transform.Scale.x;
            }
        }

        m_Fonts.m_Shader->deactivate();
    }


}
void Renderer::flush() {
    // Present the rendered image. This is an implicit glFlush.
    auto swapResult = eglSwapBuffers(m_Display, m_Surface);
    assert(swapResult == EGL_TRUE);
}

void Renderer::updateRenderArea() {
    EGLint width;
    eglQuerySurface(m_Display, m_Surface, EGL_WIDTH, &width);

    EGLint height;
    eglQuerySurface(m_Display, m_Surface, EGL_HEIGHT, &height);

    if (width != m_Width || height != m_Height) {
        m_Width = width;
        m_Height = height;
        glViewport(0, 0, width, height);

        // make sure that we lazily recreate the projection matrix before we update
        m_ShaderNeedsNewProjectionMatrix = true;
    }
}

void Renderer::createModels() {
    /*
     * This is a square:
     * 0 --- 1
     * | \   |
     * |  \  |
     * |   \ |
     * 3 --- 2
     */
    std::vector<Vertex> vertices = {
            Vertex(Vector3{1, 1, 0}, Vector2{0, 0}), // 0
            Vertex(Vector3{-1, 1, 0}, Vector2{1, 0}), // 1
            Vertex(Vector3{-1, -1, 0}, Vector2{1, 1}), // 2
            Vertex(Vector3{1, -1, 0}, Vector2{0, 1}) // 3
    };
    std::vector<Index> indices = {
            0, 1, 2, 0, 2, 3
    };


    // Create a model and put it in the back of the update list.
    m_SpriteModel = std::make_unique<Model>(vertices, indices);
}
u32 Renderer::loadTexture(const std::string &path) {
    // loads an image and assigns it to the square.
    //
    // Note: there is no texture management in this sample, so if you reuse an image be careful not
    // to load it repeatedly. Since you get a shared_ptr you can safely reuse it in many models.
    m_Textures.emplace_back(TextureAsset::loadAsset(m_App->activity->assetManager, path));
    return m_Textures.size() - 1;
}

void Renderer::shutdown() {
    if (m_Display != EGL_NO_DISPLAY) {
        eglMakeCurrent(m_Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (m_Context != EGL_NO_CONTEXT) {
            eglDestroyContext(m_Display, m_Context);
            m_Context = EGL_NO_CONTEXT;
        }
        if (m_Surface != EGL_NO_SURFACE) {
            eglDestroySurface(m_Display, m_Surface);
            m_Surface = EGL_NO_SURFACE;
        }
        eglTerminate(m_Display);
        m_Display = EGL_NO_DISPLAY;
    }
}
