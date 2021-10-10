#pragma once
#include <memory>
#include <vector>

#include "Math/Vector.h"

// Forward declarations
struct IndexModel;
class RenderContext;
struct SDL_Window;
struct SDL_Renderer;
struct SDL_PixelFormat;
class TextureWrapper;

enum class DrawDebugOptions
{
    kNone,
    kDrawLine,
    kWireframe,
    kZBuffer,
};

// @brief Controls the window
class Display
{
public:
    bool Init(int w, int h, int bpp);

    // @param verts is list of vertices and expected to be in raster space
    void Draw(const std::vector<Vec3i>& verts);

    void Render(const std::vector<IndexModel>& models, DrawDebugOptions dbo);
    void Shutdown();

    SDL_Renderer *GetRenderer() const;
    int Width() const;
    int Height() const;

private:
    // @brief Title of the window
    std::string m_title;

    // @brief Information about the window being used for display
    SDL_Window *m_window;

    // @brief Information about rendering that is only used for the window
    SDL_Renderer *m_renderer;

    // @brief Bitmap used for the renderer
    std::unique_ptr<TextureWrapper> m_bitmap;

    std::unique_ptr<RenderContext> m_renderCtx;

    // @brief pixel data (R, G, B, A channel) of the bitmap
    std::vector<unsigned char> m_pixels;

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

