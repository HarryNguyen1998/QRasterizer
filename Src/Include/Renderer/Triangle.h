#pragma once

// @brief All the vert attributes are already in order (CW)
struct Triangle
{
    Vec3f verts[3];
    Vec2f texCoords[3];
    float wCoords[3];
    Vec3f colors[3];

    Triangle() = default;
    Triangle(Vec3f v0, Vec3f v1, Vec3f v2,
        Vec2f uv0, Vec2f uv1, Vec2f uv2,
        float w0 = 1, float w1 = 1, float w2 = 1,
        Vec3f c0 = Vec3f(0.0f), Vec3f c1 = Vec3f(0.0f), Vec3f c2 = Vec3f(0.0f))
        : verts{std::move(v0), std::move(v1), std::move(v2)},
        texCoords{std::move(uv0), std::move(uv1), std::move(uv2)},
        wCoords{w0, w1, w2},
        colors{std::move(c0), std::move(c1), std::move(c2)} {}
};