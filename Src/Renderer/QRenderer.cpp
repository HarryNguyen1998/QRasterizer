#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>  // To use stringstream

#include "Renderer/QRenderer.h"
#include "Renderer/IndexModel.h"
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

    m_pixels.reserve(w * h * 4);
    m_zBuffer.reserve(w * h);

    return true;
}

void QRenderer::Render(const std::vector<Vec3i>& model, const std::vector<Vec3f>& colors)
{
    std::fill(m_pixels.begin(), m_pixels.end(), 50);
    SDL_RenderClear(m_renderer.get());
    m_rasterizer.Rasterize(m_pixels.data(), m_w, m_h, model, colors);
    SDL_UpdateTexture(m_bitmap.get(), nullptr, reinterpret_cast<const void*>(m_pixels.data()), m_w * 4);
    SDL_RenderCopy(m_renderer.get(), m_bitmap.get(), nullptr, nullptr);
    SDL_RenderPresent(m_renderer.get());
}

void QRenderer::Render(const std::vector<IndexModel>& models, const std::vector<Vec3f>& colors, Mode drawMode)
{
}


