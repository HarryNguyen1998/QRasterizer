#include <string>
#include <sstream>  // To use stringstream

#include "Renderer/Display.h"
#include "Renderer/IndexModel.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Texture.h"

bool Display::Init(int w, int h, int bpp)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "Failed to initialize SDL! Error is: " << SDL_GetError() << "\n";
        return false;
    }

    m_title = "Rasterizer";
    m_window = SDL_CreateWindow(m_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, 0);
    if (!m_window)
    {
        std::cerr << "Failed to create window! Error is: " << SDL_GetError() << "\n";
        return false;
    }

    m_renderer = SDL_CreateRenderer(m_window, -1, 0);
    if (!m_renderer)
    {
        std::cerr << "Failed to create renderer! Error is: " << SDL_GetError() << "\n";
        return false;
    }
    SDL_SetRenderDrawColor(m_renderer, 50, 50, 50, 250);
    SDL_RenderClear(m_renderer);

    m_bitmap = std::make_unique<TextureWrapper>();
    m_bitmap->Init(m_renderer, w, h);

    m_renderCtx = std::make_unique<RenderContext>();
    m_pixels.resize(w * h * bpp, 50);
    m_zBuffer.resize(w * h, FLT_MAX);

    return true;
}

void Display::Render(const std::vector<Vec3i>& verts)
{
    SDL_RenderClear(m_renderer);

    Vec3i v0 = verts[0];
    Vec3i v1 = verts[1];
    Vec3i v2 = verts[2];
    unsigned white = SDL_MapRGBA(m_bitmap->GetPixelFormat(), 255, 255, 255, 255);
#if 0
    m_renderCtx->DrawLine(m_pixels.data(), white, m_bitmap->GetPixelFormat()->BytesPerPixel,
        Width(), v0.x, v1.x, v0.y, v1.y);
    m_renderCtx->DrawLine(m_pixels.data(), white, m_bitmap->GetPixelFormat()->BytesPerPixel,
        Width(), v1.x, v2.x, v1.y, v2.y);
    m_renderCtx->DrawLine(m_pixels.data(), white, m_bitmap->GetPixelFormat()->BytesPerPixel,
        Width(), v2.x, v0.x, v2.y, v0.y);
    m_renderCtx->DrawPt(m_pixels.data(), white, 4, Width(), 400, 200);
#endif
    m_renderCtx->DrawFilledTriangle(m_pixels.data(), white, Width(), Height(), v0, v1, v2);

    m_bitmap->UpdateTexture(m_pixels.data());
    m_bitmap->Draw(m_renderer, 0, 0, nullptr, SDL_FLIP_VERTICAL);
    SDL_RenderPresent(m_renderer);
}

void Display::Render(const std::vector<IndexModel>& models, DrawDebugOptions dbo)
{
    SDL_RenderClear(m_renderer);

    switch (dbo)
    {   // Begin switch
    case DrawDebugOptions::kNone:
    {
#if 0
        for (const IndexModel& model : models)
        {
            m_renderCtx->DrawTriangles(m_renderer, *m_bitmap, m_zBuffer.data(), model, tex);
        }
#endif
        break;
    }
    case DrawDebugOptions::kDrawLine:
    {
        m_renderCtx->TestDrawLine(m_pixels.data(), m_bitmap->Width(), m_bitmap->Height(),
            m_bitmap->GetPixelFormat());
        break;
    }
    case DrawDebugOptions::kWireframe:
    {
        break;
    }
    case DrawDebugOptions::kZBuffer:
    {
        break;
    }
    default:
        break;

    }   // End switch

    m_bitmap->UpdateTexture(m_pixels.data());
    m_bitmap->Draw(m_renderer, 0, 0, nullptr, SDL_FLIP_NONE);
    SDL_RenderPresent(m_renderer);
}

void Display::ShowFrameStatistics(float dt, double sec, int frameCnt)
{
    std::stringstream ss;
    ss.setf(std::ios::fixed);
    ss << m_title << " - " << frameCnt << " frames over "
        << std::setprecision(1) << sec << " s: "
        << std::setprecision(2) << 1.0f / dt << " fps, "
        << std::setprecision(3) << (dt * 1000.0f) << " ms/frame";
    const std::string& tmp = ss.str();
    
    SDL_SetWindowTitle(m_window, tmp.c_str());
}

void Display::Shutdown()
{
    m_bitmap->Shutdown();
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
}

SDL_Renderer *Display::GetRenderer() const { return m_renderer; }
int Display::Width() const { return m_bitmap->Width(); }
int Display::Height() const { return m_bitmap->Height(); }

