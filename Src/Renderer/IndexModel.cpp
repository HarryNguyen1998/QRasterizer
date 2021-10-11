#include "Renderer/IndexModel.h"

IndexModel::IndexModel(const std::vector<Vec3f>& verts)
    : verts{verts}
{}

IndexModel::IndexModel(const std::vector<Vec3f>& verts, const std::vector<int>& vertIndices)
    : verts{verts}, vertIndices{vertIndices}
{
    ComputeNormals();
}

IndexModel::IndexModel(const std::vector<Vec3f>& verts, const std::vector<Vec2f>& texCoords,
    const std::vector<Vec3f> normals, const std::vector<int>& vertIndices,
    const std::vector<int>& uvIndices, const std::vector<int> nIndices)
    : verts{verts}, texCoords{texCoords}, normals{normals},
    vertIndices{vertIndices}, uvIndices{uvIndices}, nIndices{nIndices}
{
    if (normals.empty()) { ComputeNormals(); }
}

void IndexModel::ComputeNormals()
{
    for (int i = 0; i < nIndices.size(); i += 3)
    {
        // NOTE: .obj file has CCW winding order!
        Vec3f edge01 = verts[nIndices[i + 1]] - verts[nIndices[i]];
        Vec3f edge02 = verts[nIndices[i + 2]] - verts[nIndices[i]];

        normals.push_back(Normal(Cross(edge01, edge02)));
    }
}
