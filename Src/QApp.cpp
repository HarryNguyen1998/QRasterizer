#include "SDL.h"
#include "SDL_image.h"

#include <cassert>
#include <string>
#include <sstream>

#include "QApp.h"
#include "Renderer/QRenderer.h"
#include "Renderer/IndexModel.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Texture.h"
#include "TestVerts.h"

QApp& QApp::Instance()
{
    static QApp app{};
    return app;
}

bool QApp::Init(const std::string& title, int w, int h)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "Failed to initialize SDL! Error is: " << SDL_GetError() << "\n";
        return false;
    }

    int imgFlags = IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        std::cout << "SDL_image can't be initialized! SDL_image error: " << IMG_GetError();
        return false;
    }

    m_title = title;
    m_w = w;
    m_h = h;
    m_window.reset(SDL_CreateWindow(
        m_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_w, m_h, 0));
    if (!m_window)
    {
        std::cerr << "Failed to create window! Error is: " << SDL_GetError() << "\n";
        return false;
    }

    m_qrenderer = std::make_unique<QRenderer>();
    if (!m_qrenderer->Init(m_window.get(), m_w, m_h)) { return false; }

    return true;
}

void QApp::Start()
{
    // Separate w into 4 parts h into 6 parts
    std::vector<Vec3i> tri{
        {m_w / 4, m_h / 6, 0},
        {m_w * 3 / 4, m_h * 2 / 3, 0},
        {m_w / 2, m_h * 5 / 6, 0},
    };
    std::vector<Vec3f> color{
        {0.0f, 0.1f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.7f, 0.0f},
    };
    //IndexModel model{std::move(tri)};
#if 0
    // @todo Ideally:
    std::vector<Vec2f> texCoords{
        {1.0f, 0.0f},
        {0.0f, 1.0f},
    };
    std::vector<Vec3f> normals{
        {...},
        {...},
        {...},
    };
    // Simply don't use if texCoords or normals are empty, else error since verts are empty.
    IndexModel model{std::move(tri), std::move(texCoords), std::move(normals)};
    m_renderer.Render(model, colors, Renderer::Mode::kDrawLine);
#endif

    // For deltatime
    const double secsPerCnt = 1.0 / SDL_GetPerformanceFrequency();
    uint64_t startCounts = SDL_GetPerformanceCounter();
    double accumulatedTime = 0.0;
    int frameCnt = 0;

    // Main loop
    bool isRunning = true;
    while (isRunning)
    {
        // Input handling
        for (SDL_Event e; SDL_PollEvent(&e);)
        {
            switch (e.type)
            {
            case SDL_QUIT:
            {
                isRunning = false;
                break;
            }
            case SDL_KEYDOWN:
            {
                if (e.key.keysym.sym == SDLK_p)
                {
                    m_isPaused ^= true;
                }
                break;
            }
            }
        }

        if (m_isPaused) { continue; }

        uint64_t endCounts = SDL_GetPerformanceCounter();
        double dt = (endCounts - startCounts) * secsPerCnt;
        startCounts = endCounts;

        
        // Rendering
        m_qrenderer->Render(tri, color);

        // Frame statistics every 2s
        ++frameCnt;
        accumulatedTime += dt;
        if (accumulatedTime > 2.0)
        {
            ShowFrameStatistics(frameCnt, dt);
            accumulatedTime = 0.0;
            frameCnt = 0;
        }

    }

}

void QApp::Shutdown()
{
    IMG_Quit();
    SDL_Quit();
}

// @todo Bug, ms/frame and fps keep increasing
void QApp::ShowFrameStatistics(int frameCnt, double dt)
{
    std::stringstream ss;
    ss.setf(std::ios::fixed);
    ss << m_title << " - " << frameCnt << " frames in "
        << std::setprecision(1) << dt << " s: "
        << std::setprecision(2) << frameCnt / dt << " fps, "
        << std::setprecision(3) << (dt * 1000.0) / frameCnt << " ms/frame";
    const std::string& tmp = ss.str();
    
    SDL_SetWindowTitle(m_window.get(), tmp.c_str());
}

