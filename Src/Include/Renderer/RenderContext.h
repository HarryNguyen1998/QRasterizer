#pragma once
#include "Math/Matrix.h"
#include "Math/Vector.h"

struct Model;
class TextureWrapper;
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_PixelFormat;

// @brief Provides methods to draw on bitmap
class RenderContext
{
public:
    // @brief color the specified pixel on the pixel array
    void DrawPt(unsigned char* pixels, unsigned color, int bpp, int pixelStride,
        int x, int y);

    // @brief Draw a line from (x0, y0) to (x1, y1) on the pixel array using Bresenham algorithm
    void DrawLine(unsigned char* pixels, unsigned color, int bpp, int pixelStride,
        int x0, int x1, int y0, int y1);

    // @brief Draw 12 lines starting from center of screen, with each new line
    // as the previous rotated by 30deg
    // @param w, h is width * height = size of the pixel buffer
    void TestDrawLine(unsigned char *pixels, int w, int h, const SDL_PixelFormat *format);

    // @note remember not to pass a vector with z-value of 0.
    float ComputeDepth(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2, float w0, float w1, float w2);

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

    // @brief Gamma correct the color
    unsigned char DecodeGamma(float value);
    
    // @brief Draw Triangle method using a bitmap
    // @remark v0 v1 v2 are assumed in CW order
    void DrawTriangle(SDL_Renderer* renderer, const TextureWrapper& bitmap, float* zBuffer, Vec3f v0, Vec3f v1, Vec3f v2,
        Vec2f uv0, Vec2f uv1, Vec2f uv2, const TextureWrapper& tex);

    void DrawTriangles(SDL_Renderer* renderer, const TextureWrapper& bitmap, float* zBuffer,
        const Model& model, const TextureWrapper& tex);

};
