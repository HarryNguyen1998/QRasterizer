#pragma once
#include <vector>

#include "Math/Vector.h"

// @brief A way to save space by using an index buffer to reuse verts.
// @param indices An index buffer where 3 successive elements are 3 vertices from verts that make up
// a tri.
struct IndexModel
{
    std::vector<Vec3f> verts;
    std::vector<Vec2f> texCoords;

    // @remark If normals aren't provided, face normal will be built
    std::vector<Vec3f> normals;

    std::vector<int> vertIndices;
    std::vector<int> uvIndices;
    std::vector<int> nIndices;

    IndexModel() = default;
    IndexModel(const std::vector<Vec3f>& verts, const std::vector<int>& vertIndices);
    IndexModel(const std::vector<Vec3f>& verts, const std::vector<Vec2f>& texCoords,
        const std::vector<Vec3f> normals, const std::vector<int>& vertIndices,
        const std::vector<int>& uvIndices, const std::vector<int> nIndices);

    // @brief Helper func to calculate normals if the .obj file didn't provide them
    void ComputeNormals();
};
