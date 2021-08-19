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
    if (!m_display->Init(800, 600, 4)) { return false; }

    m_globalTimer = std::make_unique<Timer>();

    return true;
}

void QApp::Start()
{
    // Setup model
    std::vector<IndexModel> models{TestVerts::Rect(), TestVerts::RectPerspectiveCorrect()};

    // Main lopp
    bool isRunning = true;
    m_globalTimer->Reset();
    m_globalTimer->BeginStopwatch();
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

        m_globalTimer->Tick();
        float dt = (float)m_globalTimer->GetDeltaTime();

        DrawDebugOptions dbo = DrawDebugOptions::kDrawLine;
        m_display->Draw(models, dbo);

        // Frame statistics every 2s
        ++frameCnt;
        static double sec = 0.0; sec =  m_globalTimer->EndStopwatch();
        if (sec > 2.0)
        {
            m_display->ShowFrameStatistics(dt, sec, frameCnt);
            frameCnt = 0;
            m_globalTimer->BeginStopwatch();
        }

    }

}

void QApp::Shutdown()
{
    m_display->Shutdown();

    SDL_Quit();
}

