#include <iostream>
#include <string>

#include "Renderer/Display.h"
#include "Renderer/IndexModel.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Texture.h"

bool Display::Init(int w, int h, const std::vector<float>& zBuffer)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cout << "Failed to initialize SDL! Error is: " << SDL_GetError() << "\n";
        return false;
    }

    m_title = "Rasterizer";
    m_window = SDL_CreateWindow(m_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, 0);
    if (!m_window)
    {
        std::cout << "Failed to create window! Error is: " << SDL_GetError() << "\n";
        return false;
    }

    m_renderer = SDL_CreateRenderer(m_window, -1, 0);
    if (!m_renderer)
    {
        std::cout << "Failed to create renderer! Error is: " << SDL_GetError() << "\n";
        return false;
    }

    
    m_bitmap->Init(m_renderer, w, h, 4);
    m_renderCtx = std::make_unique<RenderContext>();
    m_zBuffer = zBuffer;

    return true;
}

void Display::Update(float dt, const std::vector<IndexModel>& models, const TextureWrapper& tex)
{
    // Clear screen
    SDL_SetRenderDrawColor(m_renderer, 50, 50, 50, 50);
    SDL_RenderClear(m_renderer);

    for (const IndexModel& model : models)
    {
        m_renderCtx->DrawTriangles(m_renderer, *m_bitmap, m_zBuffer.data(), model, tex);
    }

    SDL_RenderPresent(m_renderer);
}

void Display::ShowFrameStatistics(float dt, float avgFrameTime, int totalFrameCnt)
{
    float fps = 1.0f / dt;
    std::string toPrint = m_title + " - Current frame time: " + std::to_string(dt) + "s - " + std::to_string(fps) +
        "fps - Avg frame time: " + std::to_string(avgFrameTime) + "s over " + std::to_string(totalFrameCnt) + "frames";
    SDL_SetWindowTitle(m_window, toPrint.c_str());
}

void Display::Shutdown()
{
    
    m_bitmap->Shutdown();
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
}

SDL_Renderer *Display::GetRenderer() const { return m_renderer; }
