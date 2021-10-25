#pragma once
#include <cstdint>
#include <vector>

#include "Math/Matrix.h"

struct Model;
class QTexture;
struct Triangle;

class Rasterizer
{
public:
    void Rasterize(uint32_t *pixels, float *zBuffer, int w, int h, const Model& model, const Mat44f& projMat, const std::vector<Vec3f>& colors);
    void Rasterize(uint32_t *pixels, float *zBuffer, QTexture *texture, int w, int h, const Model& model, const Mat44f& projMat);
private:
    float ComputeEdge(const Vec3f& a, const Vec3f& b, const Vec3f& c);

    // @note Remember that we use RGBA32 in memory
    uint32_t ToColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    uint8_t ClampChannel(float channel);

    void DrawLine(uint32_t* pixels, uint32_t color, int w, int x0, int x1, int y0, int y1);

    // @brief Draw 12 lines starting from center of screen, with each new line
    // as the previous rotated by 30deg
    // @param w, h is width * height = size of the pixel buffer
    void TestDrawLine(uint32_t *pixels, int scrW, int scrH);

    // @brief The size of the return vector is:
    // 0 triangle is clipped
    // 3 triangle is inside, or 1 new triangle is created (1 pt is inside 2 pts are clipped)
    // 6 2 triangles are created (2 pts are inside 1 pt is clipped)
    std::vector<Triangle> ClipTriangleAgainstPlane(Vec3f planeN, Vec3f planePt, const Triangle& inTri);

    Vec3f IntersectRayPlane(const Vec3f& p0, const Vec3f& p1, float planeD, const Vec3f& planeNormal, float *outT = nullptr);

private:
    // @note Maybe useful in the future but right now only necessary to clean up Clipping algorithm
    struct Point
    {
        Vec3f pos;
        Vec2f texCoord;
        float wCoord;
        Vec3f color;
        Point(Vec3f inPos, Vec2f inTexCoord, float inWCoord = 1.0f, Vec3f inColor = Vec3f(0.0f)) :
            pos{std::move(inPos)}, texCoord{std::move(inTexCoord)}, wCoord{inWCoord}, color{std::move(inColor)} {}
    };
};
