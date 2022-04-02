#pragma once
#include <cstdint>
#include <vector>

#include "Math/Matrix.h"

struct Model;
class QTexture;
struct Triangle;
enum class QRendererMode;

class Rasterizer
{
public:
    void Rasterize(uint32_t *pixels, float *zBuffer, int w, int h, const Model& model, const Mat44f& projMat, QRendererMode mode);
    void Rasterize(uint32_t *pixels, float *zBuffer, QTexture *texture, int w, int h, const Model& model, const Mat44f& projMat, QRendererMode mode);
    // @brief Gamma correct the color
    unsigned char DecodeGamma(int value);

private:
    float ComputeEdge(const Vec3f& a, const Vec3f& b, const Vec3f& c);

    // @note Remember that we use RGBA32 in memory
    uint32_t ToColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    void ToComponent(uint32_t inColor, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a);
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

    // @brief Gamma decoded LUT with gamma = 2.2, 8-bit.
    // @see https://scantips.com/lights/gamma3.html
    const unsigned char g_gammaDecodedTable[256] = 
    {    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,
         1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   2,   2,
         3,   3,   3,   3,   3,   4,   4,   4,   4,   5,   5,   5,   5,   6,   6,   6,
         6,   7,   7,   7,   8,   8,   8,   9,   9,   9,  10,  10,  11,  11,  11,  12,
        12,  13,  13,  13,  14,  14,  15,  15,  16,  16,  17,  17,  18,  18,  19,  19,
        20,  20,  21,  22,  22,  23,  23,  24,  25,  25,  26,  26,  27,  28,  28,  29,
        30,  30,  31,  32,  33,  33,  34,  35,  35,  36,  37,  38,  39,  39,  40,  41,
        42,  43,  43,  44,  45,  46,  47,  48,  49,  49,  50,  51,  52,  53,  54,  55,
        56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,
        73,  74,  75,  76,  77,  78,  79,  81,  82,  83,  84,  85,  87,  88,  89,  90,
        91,  93,  94,  95,  97,  98,  99, 100, 102, 103, 105, 106, 107, 109, 110, 111,
       113, 114, 116, 117, 119, 120, 121, 123, 124, 126, 127, 129, 130, 132, 133, 135,
       137, 138, 140, 141, 143, 145, 146, 148, 149, 151, 153, 154, 156, 158, 159, 161,
       163, 165, 166, 168, 170, 172, 173, 175, 177, 179, 181, 182, 184, 186, 188, 190,
       192, 194, 196, 197, 199, 201, 203, 205, 207, 209, 211, 213, 215, 217, 219, 221,
       223, 225, 227, 229, 231, 234, 236, 238, 240, 242, 244, 246, 248, 251, 253, 255
    };
    
};
