#include <algorithm>
#include <cassert>
#include <iostream>

#include "Renderer/IndexModel.h"
#include "Renderer/Rasterizer.h"

void Rasterizer::Rasterize(uint32_t *pixels, int w, int h, const Model& model, const Mat44f& projMat, const std::vector<Vec3f>& colors)
{
    assert(!model.verts.empty());
    assert(!model.vertIndices.empty());

    for (int i = 0; i < model.vertIndices.size(); i += 3)
    {
        Vec3f v0 = model.verts[model.vertIndices[i]];
        Vec3f v1 = model.verts[model.vertIndices[i + 1]];
        Vec3f v2 = model.verts[model.vertIndices[i + 2]];

        // Back face culling in cam space
        Vec3f surfNormal = Math::Normal(Math::Cross(v2 - v0, v1 - v0));
        if (Math::Dot(v0, surfNormal) >= 0.0f)
            continue;

        // Flat shading
        Vec3f lightDir = Math::Normal(Vec3f{0.0f, 0.0f, 1.0f});
        // @note Since it survives back face culling, surfNormal should be (+)
        float dp = std::max(0.05f, Math::Dot(lightDir, surfNormal));

        // Projection
        float homogeneousCoords[3] = {-v0.z, -v1.z, -v2.z};
        v0 = MultiplyVecMat(v0, projMat);
        v1 = MultiplyVecMat(v1, projMat);
        v2 = MultiplyVecMat(v2, projMat);
        

        // Perspective divide
        v0 /= homogeneousCoords[0];
        v1 /= homogeneousCoords[1];
        v2 /= homogeneousCoords[2];

        // To raster space
        v0.x = (v0.x + 1.0f) * w / 2;
        v0.y = (v0.y + 1.0f) * h / 2;
        v1.x = (v1.x + 1.0f) * w / 2;
        v1.y = (v1.y + 1.0f) * h / 2;
        v2.x = (v2.x + 1.0f) * w / 2;
        v2.y = (v2.y + 1.0f) * h / 2;

        // @note Reminder when dragging camera in x-axis when array is 1D and y is in bbMaxY range
        assert(v0.x < w && v1.x < w && v2.x < w);
        assert(v0.x >= 0 && v1.x >= 0 && v2.x >= 0);

        float areaOfParallelogram = ComputeEdge(v0, v1, v2);
        int bbMinX = (int)Helper::Min3(v0.x, v1.x, v2.x);
        int bbMaxX = (int)Helper::Max3(v0.x, v1.x, v2.x);
        int bbMinY = (int)Helper::Min3(v0.y, v1.y, v2.y);
        int bbMaxY = (int)Helper::Max3(v0.y, v1.y, v2.y);
        for (int y = bbMinY; y <= bbMaxY; ++y)
        {
            for (int x = bbMinX; x <= bbMaxX; ++x)
            {
                Vec3f pt{(float)x, (float)y, 0.0f};

                // Inside-outside test
                float e12 = ComputeEdge(v1, v2, pt);
                float e20 = ComputeEdge(v2, v0, pt);
                float e01 = ComputeEdge(v0, v1, pt);
                if (e01 < 0.0f || e12 < 0.0f || e20 < 0.0f) { continue; }

                float t0 = e12 / areaOfParallelogram;
                float t1 = e20 / areaOfParallelogram;
                float t2 = e01 / areaOfParallelogram;

#if 0
                uint8_t r = ClampChannel(colors[i].r * t0 + colors[i+1].r * t1 + colors[i+2].r * t2);
                uint8_t g = ClampChannel(colors[i].g * t0 + colors[i+1].g * t1 + colors[i+2].g * t2);
                uint8_t b = ClampChannel(colors[i].b * t0 + colors[i+1].b * t1 + colors[i+2].b * t2);
#endif
                // @todo This is sooooo error-prone due to i/3 expect <= 12 colors
                uint8_t r = ClampChannel(colors[2].r*dp);
                uint8_t g = ClampChannel(colors[2].g*dp);
                uint8_t b = ClampChannel(colors[2].b*dp);

                pixels[x + y * w] = ToColor(r, g, b, 255);
            }
        }
    }
}


uint32_t Rasterizer::ToColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    uint32_t color = (a << 24) | (b << 16) | (g << 8) | r;
    return color;
}

uint8_t Rasterizer::ClampChannel(float channel)
{
    return (uint8_t)std::max(0.0f, std::min(1.0f, channel) * 255.0f + 0.5f);
}

float Rasterizer::ComputeEdge(const Vec3f& a, const Vec3f& b, const Vec3f& c)
{
    float result = (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
    return result;
}

