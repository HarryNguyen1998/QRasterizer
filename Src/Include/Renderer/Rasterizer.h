#pragma once
#include <cstdint>
#include <vector>

#include "Math/Matrix.h"

struct Model;

class Rasterizer
{
public:
    void Rasterize(uint32_t *pixels, int w, int h, const Model& model, const Mat44f& projMat, const std::vector<Vec3f>& colors);
private:
    float ComputeEdge(const Vec3f& a, const Vec3f& b, const Vec3f& c);

    // @note Remember that we use RGBA32 in memory
    uint32_t ToColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    uint8_t ClampChannel(float channel);
};