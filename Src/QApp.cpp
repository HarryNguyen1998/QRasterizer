#include "SDL_image.h"

#include <iostream>
#include <string>

#include "QApp.h"
#include "Renderer/Display.h"
#include "Renderer/IndexModel.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Texture.h"
#include "Utils/Timer.h"
#include "TestVerts.h"

QApp& QApp::Instance()
{
    static QApp app{};
    return app;
}

bool QApp::Init()
{
    m_display = std::make_unique<Display>();
    if (!m_display->Init(800, 600, std::vector<float>(800 * 600 * 4, FLT_MAX))) { return false; }

    m_globalTimer = std::make_unique<Timer>();

    return true;
}

void QApp::Start()
{
    // Setup model
    std::vector<IndexModel> models{TestVerts::Rect(), TestVerts::RectPerspectiveCorrect()};

    // Main lopp
    bool isRunning = true;
    float accumulatedTime = 0.0f;
    m_globalTimer->Reset();
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

        m_globalTimer->Tick();
        float dt = (float)m_globalTimer->GetDeltaTime();

        // Frame statistics
        static int totalFrameCnt = 0;
        static double timeElapsed = 0.0;
        if (m_globalTimer->GetTotalTime() - timeElapsed > 1.0)
        {
            m_display->ShowFrameStatistics(dt, (float)m_globalTimer->GetTotalTime() / totalFrameCnt, totalFrameCnt);
            ++timeElapsed;
        }

        DrawDebugOptions dbo = DrawDebugOptions::kDrawLine;
        m_display->Draw(models, dbo);

        ++totalFrameCnt;
    }

}

void QApp::Shutdown()
{
    m_display->Shutdown();

    SDL_Quit();
}

