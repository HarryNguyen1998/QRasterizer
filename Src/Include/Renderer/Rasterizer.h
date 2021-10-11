#pragma once
#include <cstdint>
#include <vector>

#include "Math/Vector.h"

class Rasterizer
{
public:
    void Rasterize(uint32_t *pixels, int w, int h, const std::vector<Vec3i> verts, const std::vector<Vec3f> colors);
private:
    float ComputeEdge(const Vec3f& a, const Vec3f& b, const Vec3f& c);
    int ComputeEdge(const Vec3i& a, const Vec3i& b, const Vec3i &c);

    // @note Remember that we use RGBA32 in memory
    uint32_t ToColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    uint8_t ClampChannel(float channel);
};