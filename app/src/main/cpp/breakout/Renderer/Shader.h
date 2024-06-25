#ifndef ANDROIDGLINVESTIGATIONS_SHADER_H
#define ANDROIDGLINVESTIGATIONS_SHADER_H

#include <string>
#include <GLES3/gl3.h>
#include "TextureAsset.h"
#include "Math/MathTypes.h"

class Model;

/*!
 * A class representing a simple shader program. It consists of vertex and fragment components. The
 * input attributes are a position (as a Vector3) and a uv (as a Vector2). It also takes a uniform
 * to be used as the entire model/view/projection matrix. The shader expects a single texture for
 * fragment shading, and does no other lighting calculations (thus no uniforms for lights or normal
 * attributes).
 */
class Shader {
public:
    /*!
     * Loads a shader given the full sourcecode and names for necessary attributes and uniforms to
     * link to. Returns a valid shader on success or null on failure. Shader resources are
     * automatically cleaned up on destruction.
     *
     * @param vertexSource The full source code for your vertex program
     * @param fragmentSource The full source code of your fragment program
     * @param positionAttributeName The name of the position attribute in your vertex program
     * @param uvAttributeName The name of the uv coordinate attribute in your vertex program
     * @param projectionMatrixUniformName The name of your model/view/projection matrix uniform
     * @return a valid Shader on success, otherwise null.
     */
    static Shader *loadShader(
            const std::string &vertexSource,
            const std::string &fragmentSource,
            const std::string &positionAttributeName,
            const std::string &uvAttributeName,
            const std::string &projectionMatrixUniformName,
            const std::string &modelMatrixUniformName,
            const std::string &colorUniformName);

    inline ~Shader() {
        if (m_ShaderID) {
            glDeleteProgram(m_ShaderID);
            m_ShaderID = 0;
        }
    }

    /*!
     * Prepares the shader for use, call this before executing any draw commands
     */
    void activate() const;

    /*!
     * Cleans up the shader after use, call this after executing any draw commands
     */
    void deactivate() const;

    /*!
     * Renders a single model
     * @param transform model transform
     * @param model a model to draw
     * @param texture a texture to draw
     * @param color color of the model
     */
    void drawModel(const Mat4& transform, const Model &model, const TextureAsset& texture, const V3& color) const;

    /*!
     * Renders a single model
     * @param model a model to draw
     * @param texture a texture to draw
     * @param color color of the model
     */
    void drawModel(const Model& model, u32 texture, const V3& color) const;
    /*!
     * Sets the model/view/projection matrix in the shader.
     * @param projectionMatrix sixteen floats, column major, defining an OpenGL projection matrix.
     */
    void setProjectionMatrix(const Mat4& projectionMatrix) const;

private:
    /*!
     * Helper function to load a shader of a given type
     * @param shaderType The OpenGL shader type. Should either be GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
     * @param shaderSource The full source of the shader
     * @return the id of the shader, as returned by glCreateShader, or 0 in the case of an error
     */
    static GLuint loadShader(GLenum shaderType, const std::string &shaderSource);

    /*!
     * Constructs a new instance of a shader. Use @a loadShader
     * @param program the GL program id of the shader
     * @param position the attribute location of the position
     * @param uv the attribute location of the uv coordinates
     * @param projectionMatrix the uniform location of the projection matrix
     */
    constexpr Shader(
            GLuint program,
            GLint position,
            GLint uv,
            GLint projectionMatrix,
            GLint modelMatrix,
            GLint color)
            : m_ShaderID(program),
              m_Position(position),
              m_TexCoords(uv),
              m_ProjectionMatrix(projectionMatrix),
              m_ModelMatrix(modelMatrix),
              m_Color(color){}

    GLuint m_ShaderID;
    GLint m_Position;
    GLint m_TexCoords;
    GLint m_ProjectionMatrix;
    GLint m_ModelMatrix;
    GLint m_Color;
};

#endif //ANDROIDGLINVESTIGATIONS_SHADER_H