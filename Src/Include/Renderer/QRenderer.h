#pragma once
#include <memory>
#include <cstdint>
#include <vector>

#include "Math/Vector.h"
#include "Renderer/Rasterizer.h"
#include "SDL_Deleter.h"

// Forward declarations
struct IndexModel;
struct SDL_PixelFormat;

// @brief Controls the window
class QRenderer
{
public:
    enum class Mode
    {
        kNone,
        kDrawLine,
        kWireframe,
        kZBuffer,
    };
public:
    bool Init(SDL_Window *window, int w, int h);

    // @param verts is list of vertices and expected to be in raster space
    void Draw(const std::vector<Vec3i>& verts);

    // @note Only render 1 single triangle.
    void Render(const std::vector<Vec3i>& model, const std::vector<Vec3f>& colors);
    void Render(const std::vector<IndexModel>& models, const std::vector<Vec3f>& colors, QRenderer::Mode drawMode);

private:
    // @brief Information about rendering that is only used for the window
    std::unique_ptr<SDL_Renderer, SDL_Deleter> m_renderer;

    // @brief Bitmap used for the renderer
    std::unique_ptr<SDL_Texture, SDL_Deleter> m_bitmap;

    Rasterizer m_rasterizer;
    int m_w, m_h;

    // @brief pixel data of the bitmap
    std::vector<uint32_t> m_pixels;

    // @brief Z-buffer of the bitmap
    std::vector<float> m_zBuffer;
};

#if 0
// @brief Test if depth calculation is correct by drawing the depth buffer on screen
void DrawDepth(SDL_Surface* surface, const std::vector<float>& zBuffer, int x, int y, int pxStride)
{
    float intensity = (1.0f - zBuffer[x + y * pxStride]) * 0.5f;
    auto decodedColor = DecodeGamma(intensity);
    unsigned color = SDL_MapRGB(surface->format, decodedColor, decodedColor, decodedColor);
    PutPixel((unsigned char*)surface->pixels, color, 4, pxStride, x, y);
}

// @brief Draw wire frame of the obj on screen
void DrawWireframe(SDL_Surface* surface, const IndexModel& model)
{
    int w = surface->w;
    int h = surface->h;
    for (int i = 0; i < model.vertIndices.size(); i += 3)
    {
        Vec3f v0Raster = model.verts[model.vertIndices[i]];
        Vec3f v1Raster = model.verts[model.vertIndices[i + 1]];
        Vec3f v2Raster = model.verts[model.vertIndices[i + 2]];

        int x0 = (int)((v0Raster.x + 1) / 2.0f * w);
        int y0 = (int)((1 - v0Raster.y) / 2.0f * h);

        int x1 = (int)((v1Raster.x + 1) / 2.0f * w);
        int y1 = (int)((1 - v1Raster.y) / 2.0f * h);

        int x2 = (int)((v2Raster.x + 1) / 2.0f * w);
        int y2 = (int)((1 - v2Raster.y) / 2.0f * h);

        // No clipping yet!
        int xMin = Helper::Min3(x0, x1, x2);
        int yMin = Helper::Min3(y0, y1, y2);
        int xMax = Helper::Max3(x0, x1, x2);
        int yMax = Helper::Max3(y0, y1, y2);
        if (xMax > w - 1 || xMin < 0 || yMax > h - 1 || yMin < 0) { continue; }

        DrawLine((unsigned*)surface->pixels, SDL_MapRGB(surface->format, 255, 255, 255),
            w, x0, y0, x1, y1);
        DrawLine((unsigned*)surface->pixels, SDL_MapRGB(surface->format, 255, 255, 255),
            w, x1, y1, x2, y2);
        DrawLine((unsigned*)surface->pixels, SDL_MapRGB(surface->format, 255, 255, 255),
            w, x2, y2, x0, y0);
    }
}
#endif

