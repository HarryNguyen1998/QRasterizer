#include "Renderer/IndexModel.h"

Model::Model(const std::vector<Vec3f>& inVerts, const std::vector<int>& inVertIndices)
    : verts{inVerts}, vertIndices{inVertIndices}
{
    // Change to CW order
    for (int i = 0; i < vertIndices.size(); i += 3)
    {
        int tmp = vertIndices[i + 1]; 
        vertIndices[i + 1] = vertIndices[i + 2];
        vertIndices[i + 2] = tmp;

        if (!uvIndices.empty())
        {
            tmp = uvIndices[i + 1];
            uvIndices[i + 1] = uvIndices[i + 2];
            uvIndices[i + 2] = tmp;
        }

        if (!nIndices.empty())
        {
            tmp = nIndices[i + 1];
            nIndices[i + 1] = nIndices[i + 2];
            nIndices[i + 2] = tmp;
        }
        // No normal data, so calc surf normal
        else
        {
            normals.push_back(Math::Cross(
                verts[vertIndices[i + 2]] - verts[vertIndices[i]],
                verts[vertIndices[i + 1]] - verts[vertIndices[i]]));
        }
    }
}

Model::Model(const std::vector<Vec3f>& inVerts, const std::vector<Vec2f>& inTexCoords,
    const std::vector<Vec3f>& inNormals, const std::vector<int>& inVertIndices,
    const std::vector<int>& inUVIndices, const std::vector<int>& inNIndices)
    : verts{inVerts}, texCoords{inTexCoords}, normals{inNormals},
    vertIndices{inVertIndices}, uvIndices{inUVIndices}, nIndices{inNIndices}
{
    // Change to CW order
    for (int i = 0; i < vertIndices.size(); i += 3)
    {
        int tmp = vertIndices[i + 1]; 
        vertIndices[i + 1] = vertIndices[i + 2];
        vertIndices[i + 2] = tmp;

        if (!uvIndices.empty())
        {
            tmp = uvIndices[i + 1];
            uvIndices[i + 1] = uvIndices[i + 2];
            uvIndices[i + 2] = tmp;
        }

        if (!nIndices.empty())
        {
            tmp = nIndices[i + 1];
            nIndices[i + 1] = nIndices[i + 2];
            nIndices[i + 2] = tmp;
        }
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

