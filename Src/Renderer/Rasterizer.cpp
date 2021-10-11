#include <algorithm>

#include "Renderer/Rasterizer.h"

void Rasterizer::Rasterize(uint32_t *pixels, int w, int h, const std::vector<Vec3i> verts, const std::vector<Vec3f> colors)
{
    for (int i = 0; i < verts.size(); i += 3)
    {
        // Winding order is CW
        // @todo Add a isFrontCCW to be able to swap between CCW and CW easily
        Vec3i v0 = verts[0];
        Vec3i v1 = verts[2];
        Vec3i v2 = verts[1];

        int areaOfParallelogram = ComputeEdge(v0, v1, v2);
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                // y points down
                Vec3i pt{x, h - y, 0};

                // Inside-outside test
                int e12 = ComputeEdge(v1, v2, pt);
                int e20 = ComputeEdge(v2, v0, pt);
                int e01 = ComputeEdge(v0, v1, pt);
                if ((e01 | e12 | e20) < 0) { continue; }

                float t0 = (float)e12 / areaOfParallelogram;
                float t1 = (float)e20 / areaOfParallelogram;
                float t2 = (float)e01 / areaOfParallelogram;

                uint8_t r = ClampChannel(colors[0].r * t0 + colors[1].r * t1 + colors[2].r * t2);
                uint8_t g = ClampChannel(colors[0].g * t0 + colors[1].g * t1 + colors[2].g * t2);
                uint8_t b = ClampChannel(colors[0].b * t0 + colors[1].b * t1 + colors[2].b * t2);

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

int Rasterizer::ComputeEdge(const Vec3i& a, const Vec3i& b, const Vec3i &c)
{
    int result = (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
    return result;
}

