#include "Math.h"

#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtx/matrix_decompose.hpp>

namespace Math {
    Mat4 createTransform(const V3 &translation, const Quaternion &rotation, const V3 &scale) {
        const auto rot = glm::toMat4(rotation);
        return glm::translate(Mat4(1.0f), translation) * rot * glm::scale(Mat4(1.0f), scale);
    }

    Mat4 createSpriteTransform(const V3 &translation, const Quaternion &rotation, const V3 &scale) {
        auto model = Mat4(1.0);
        model = glm::translate(model, translation);
        model = glm::translate(model, V3{0.5f * scale.x, 0.5f * scale.y, 0.0f});

        const auto rot = glm::toMat4(rotation);
        model = model * rot;

        model = glm::translate(model, V3{-0.5f * scale.x, -0.5f * scale.y, 0.0});
        model = glm::scale(model, scale);

        return model;
    }

    void decomposeTransform(const Mat4 &m, V3 &translation, Quaternion &rotation, V3 &scale) {
        V3 view;
        V4 pers;

        glm::decompose(m, scale, rotation, translation, view, pers);
    }

    float lerp(float a, float b, float f) {
        return a + f * (b - a);
    }
}
