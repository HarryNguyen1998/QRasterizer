#include "SDL.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>  // To use stringstream
#include <utility>

#include "Renderer/QRenderer.h"
#include "Renderer/Model.h"
#include "Renderer/Texture.h"

bool QRenderer::Init(SDL_Window *window, int w, int h)
{
    m_renderer.reset(SDL_CreateRenderer(window, -1, 0));
    if (!m_renderer)
    {
        std::cerr << "Failed to create renderer! Error is: " << SDL_GetError() << "\n";
        return false;
    }

    m_w = w;
    m_h = h;

    m_bitmap.reset(SDL_CreateTexture(
        m_renderer.get(), SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, w, h));
    if (!m_bitmap)
    {
        std::cerr << "Failed to create main texture! Error is: " << SDL_GetError() << "\n";
        return false;
    }

    m_pixels = std::vector<uint32_t>(m_w * m_h, 0);
    m_zBuffer = std::vector<float>(m_w * m_h, 0.0f);

    return true;
}

void QRenderer::Render(const Model& model, const std::vector<Vec3f>& colors, QRenderer::Mode drawMode)
{
    m_rasterizer.Rasterize(m_pixels.data(), m_zBuffer.data(), m_w, m_h, model, m_projMat, colors);
}

void QRenderer::Render(QTexture *texture, const Model& model, Mode drawMode)
{
    m_rasterizer.Rasterize(m_pixels.data(), m_zBuffer.data(), texture, m_w, m_h, model, m_projMat);
}

void QRenderer::SwapBuffers()
{
    SDL_UpdateTexture(m_bitmap.get(), nullptr, reinterpret_cast<const void*>(m_pixels.data()), m_w * 4);
    SDL_RenderCopyEx(m_renderer.get(), m_bitmap.get(), nullptr, nullptr, 0, nullptr, SDL_FLIP_VERTICAL);
    SDL_RenderPresent(m_renderer.get());
    std::fill(m_pixels.begin(), m_pixels.end(), 0);
    std::fill(m_zBuffer.begin(), m_zBuffer.end(), 0.0f);
}

void QRenderer::SetProjectionMatrix(Mat44f m) { m_projMat = std::move(m); }

Mat44f QRenderer::LookAt(const Vec3f& eye, const Vec3f& at, const Vec3f& up)
{
    Vec3f camForward = Math::Normal(eye - at);
    Vec3f camRight = Math::Normal(Math::Cross(up, camForward));
    Vec3f camUp = Cross(camForward, camRight);

    Mat44f viewMat{};
    for (int i = 0; i < 3; ++i)
    {
        // Inverse of orthonormal is transpose, of translation is same values but negated.
        viewMat(i, 0) = camRight[i];
        viewMat(i, 1) = camUp[i];
        viewMat(i, 2) = camForward[i];
        viewMat(3, i) = -eye[i];
    }
#if 0
    viewMat(3, 0) = -Math::Dot(camRight, eye);
    viewMat(3, 1) = -Math::Dot(camUp, eye);
    viewMat(3, 2) = -Math::Dot(camForward, eye);
#endif

    return viewMat;
}

SDL_Renderer *QRenderer::GetRenderer()
{
    return m_renderer.get();
}

