#pragma once
#include <memory>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Renderer/Rasterizer.h"
#include "SDL_Deleter.h"

// Forward declarations
struct Model;
class QTexture;

enum class QRendererMode
{
    kNone,
    kWireframe,
    kZBuffer,
};

// @brief Controls the window
class QRenderer
{
public:
    bool Init(SDL_Window *window, int w, int h);

    void Render(const Model& model, QRendererMode drawMode);
    void Render(const Model& model, std::shared_ptr<QTexture> texture, QRendererMode drawMode);
    void SwapBuffers();

    // @brief Move the projection matrix from caller 
    void SetProjectionMatrix(Mat44f m);

    // @brief Construct a view matrix.
    Mat44f LookAt(const Vec3f& eye, const Vec3f& at, const Vec3f& up = Vec3f{0.0f, 1.0f, 0.0f});

    SDL_Renderer *GetRenderer();

private:
    // @brief Information about rendering that is only used for the window
    std::unique_ptr<SDL_Renderer, SDL_Deleter> m_renderer;

    // @brief Bitmap used for the renderer
    std::unique_ptr<SDL_Texture, SDL_Deleter> m_bitmap;

    Rasterizer m_rasterizer;
    int m_w, m_h;

    Mat44f m_projMat;

    // @brief pixel data of the bitmap
    std::vector<uint32_t> m_pixels;

    // @brief Z-buffer of the bitmap
    std::vector<float> m_zBuffer;
};

