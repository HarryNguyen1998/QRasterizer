#pragma once
#include <vector>

#include "Math/Vector.h"

// @brief A way to save space by using an index buffer to reuse verts.
// @param indices An index buffer where 3 successive elements are 3 vertices from verts that make up
// a tri.
struct Model
{
    std::vector<Vec3f> verts;
    std::vector<Vec2f> texCoords;
    // @todo If normals aren't provide, build the face normals, and update them when obj is transformed.
    std::vector<Vec3f> normals;

    std::vector<int> vertIndices;
    std::vector<int> uvIndices;
    std::vector<int> nIndices;

    Model() = default;
    Model(std::vector<Vec3f> inVerts, std::vector<int> inVertIndices,
        std::vector<Vec2f> inTexCoords = {}, std::vector<int> inUVIndices = {},
        std::vector<Vec3f> inNormals = {}, std::vector<int> inNIndices = {});

    void CalculateNormal();
};
