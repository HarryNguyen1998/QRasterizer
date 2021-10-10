#include "SDL_image.h"

#include <string>
#include <sstream>

#include "QApp.h"
#include "Renderer/Display.h"
#include "Renderer/IndexModel.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Texture.h"
#include "TestVerts.h"

QApp& QApp::Instance()
{
    static QApp app{};
    return app;
}

bool QApp::Init(int w, int h)
{
    m_display = std::make_unique<Display>();
    if (!m_display->Init(w, h, 4)) { return false; }

    return true;
}

void QApp::Start()
{
    // Setup model
    std::vector<IndexModel> models{TestVerts::Rect(), TestVerts::RectPerspectiveCorrect()};

    // Separate w into 4 parts h into 6 parts
    int w = m_display->Width();
    int h = m_display->Height();
#if 0
    std::vector<Vec3i> tri{{w / 4, h * 5 / 6, 0},
        {w * 3 / 4, h / 3, 0},
        {w / 2, h / 6, 0}};
#endif
    std::vector<Vec3i> tri{{200, 100, 0}, {400, 500, 0}, {600, 400, 0}};

    // Main lopp
    bool isRunning = true;

    // Call SDL_GetPerformanceFrequency() for secsPerCnt;
    // Each loop, query startCounts, then end of loop, query endCounts, then cal deltaTime
    // Check accumulatedTime is 2s, then
    
    const double secsPerCnt = 1.0 / SDL_GetPerformanceFrequency();
    uint64_t startCounts = SDL_GetPerformanceCounter();
    double timeElapsed = 0.0;
    double accumulatedTime = 0.0;
    while (isRunning)
    {
        static int frameCnt;

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

        double dt = (SDL_GetPerformanceCounter() - startCounts) * secsPerCnt;

        DrawDebugOptions dbo = DrawDebugOptions::kDrawLine;
        m_display->Draw(tri);

        // Frame statistics every 2s
        ++frameCnt;
        accumulatedTime += dt;
        if (accumulatedTime > 2.0)
        {
            timeElapsed += accumulatedTime;
            accumulatedTime = 0.0;
            frameCnt = 0;
            ShowFrameStatistics(frameCnt, (float)dt, timeElapsed);
        }

    }

}

void QApp::Shutdown()
{
    m_display->Shutdown();

    SDL_Quit();
}

void QApp::ShowFrameStatistics(int frameCnt, float dt, double timeElapsed)
{
    std::string title = SDL_GetWindowTitle(m_window);
    std::stringstream ss;
    ss.setf(std::ios::fixed);
    ss << title << " - " << frameCnt << " frames over "
        << std::setprecision(1) << timeElapsed << " s: "
        << std::setprecision(2) << 1.0f / dt << " fps, "
        << std::setprecision(3) << (dt * 1000.0f) << " ms/frame";
    const std::string& tmp = ss.str();
    
    SDL_SetWindowTitle(m_window, tmp.c_str());
}

