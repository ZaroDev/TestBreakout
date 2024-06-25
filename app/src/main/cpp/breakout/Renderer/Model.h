#ifndef ANDROIDGLINVESTIGATIONS_MODEL_H
#define ANDROIDGLINVESTIGATIONS_MODEL_H

#include <vector>
#include "TextureAsset.h"

union Vector3 {
    struct {
        float x, y, z;
    };
    float idx[3];
};

union Vector2 {
    struct {
        float x, y;
    };
    struct {
        float u, v;
    };
    float idx[2];
};

struct Vertex {
    constexpr Vertex(const Vector3 &inPosition, const Vector2 &inUV) : position(inPosition),
                                                                       uv(inUV) {}

    Vector3 position;
    Vector2 uv;
};

typedef uint16_t Index;

class Model {
public:
    inline Model(
            std::vector<Vertex> vertices,
            std::vector<Index> indices)
            : m_Vertices(std::move(vertices)),
              m_Indices(std::move(indices))
              {}

    inline const Vertex *getVertexData() const {
        return m_Vertices.data();
    }

    inline const size_t getIndexCount() const {
        return m_Indices.size();
    }

    inline const Index *getIndexData() const {
        return m_Indices.data();
    }

    inline const size_t getVertexCount() const {
        return m_Vertices.size();
    }

private:
    std::vector<Vertex> m_Vertices;
    std::vector<Index> m_Indices;
};

#endif //ANDROIDGLINVESTIGATIONS_MODEL_H