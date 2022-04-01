#pragma once
#include <vector>

#include "Math/Vector.h"

enum class InputWindingOrder
{
    kCW,
    kCCW,
};


// @param indices An index buffer where 3 successive elements are 3 vertices from verts that make up
// a tri. This is used to save space
// @note We impose a CW winding order, so if input data is CCW, it will change to CW order
struct Model
{
    std::vector<Vec3f> verts;
    std::vector<Vec3f> colors;
    std::vector<Vec2f> texCoords;
    std::vector<Vec3f> normals;

    std::vector<int> vertIndices;
    std::vector<int> uvIndices;
    std::vector<int> nIndices;

    Model() = default;
    Model(InputWindingOrder mode,
        std::vector<Vec3f> inVerts, std::vector<Vec3f> inColors, std::vector<int> inVertIndices,
        std::vector<Vec2f> inTexCoords = {}, std::vector<int> inUVIndices = {},
        std::vector<Vec3f> inNormals = {}, std::vector<int> inNIndices = {});

};
