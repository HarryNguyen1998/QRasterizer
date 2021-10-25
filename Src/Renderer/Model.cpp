#include <algorithm>

#include "Renderer/Model.h"

Model::Model(std::vector<Vec3f> inVerts, std::vector<int> inVertIndices,
    std::vector<Vec2f> inTexCoords, std::vector<int> inUVIndices,
    std::vector<Vec3f> inNormals, std::vector<int> inNIndices)
    : verts{std::move(inVerts)}, vertIndices{std::move(inVertIndices)},
    texCoords{std::move(inTexCoords)}, uvIndices{std::move(inUVIndices)},
    normals{std::move(inNormals)}, nIndices{std::move(inNIndices)}
{
    // Change to CW order
    for (int i = 0; i < vertIndices.size(); i += 3)
    {
        std::swap(vertIndices[i + 1], vertIndices[i + 2]);

        if (!uvIndices.empty())
            std::swap(uvIndices[i + 1], uvIndices[i + 2]);

        if (!nIndices.empty())
            std::swap(nIndices[i + 1], nIndices[i + 2]);
        // No normal data, so calc surf normal
        else
        {
            normals.push_back(Math::Cross(
                verts[vertIndices[i + 2]] - verts[vertIndices[i]],
                verts[vertIndices[i + 1]] - verts[vertIndices[i]]));
        }
    }
}

void Model::CalculateNormal()
{
    // If no normal index is specified, each normal is a surf normal. Else, it's a vert normal
    if (nIndices.empty())
    {
            // @todo    
    }

}

