#include "SDL_image.h"

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

bool QApp::Init(int w, int h)
{
    m_display = std::make_unique<Display>();
    if (!m_display->Init(w, h, 4)) { return false; }

    m_globalTimer = std::make_unique<Timer>();

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
    m_globalTimer->Init();
    m_globalTimer->StartStopwatch();
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
        m_display->Render(tri);

        // Frame statistics every 2s
        ++frameCnt;
        static double sec = 0.0; sec =  m_globalTimer->StopStopwatch();
        if (sec > 2.0)
        {
            m_display->ShowFrameStatistics(dt, sec, frameCnt);
            frameCnt = 0;
            m_globalTimer->StartStopwatch();
        }

    }

}

void QApp::Shutdown()
{
    m_display->Shutdown();

    SDL_Quit();
}

