#pragma once

// @brief All the vert attributes are already in order (CW)
struct Triangle
{
    static constexpr size_t size = 3;
    Vec3f verts[size];
    float wCoords[size];

    Triangle(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2,
        float w0 = 1, float w1 = 1, float w2 = 1) : verts{v0, v1, v2}, wCoords{w0, w1, w2} {}
};