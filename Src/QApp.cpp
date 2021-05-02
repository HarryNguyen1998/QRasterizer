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

void QApp::DoKeyDown(SDL_KeyboardEvent* e)
{
    if (e->repeat == 0)
    {
        if (e->keysym.sym == SDLK_b)
        {
            isPaused = !isPaused;
        }
    }
}

bool QApp::Init()
{
    m_display = std::make_unique<Display>();
    if (!m_display->Init(800, 600, std::vector<float>(800 * 600 * 4, FLT_MAX))) { return false; }

    m_texManager = std::make_unique<TextureManager>();
    if (!m_texManager->Init()) { return false; }

    m_timer = std::make_unique<Timer>();

    return true;
}

void QApp::Start()
{
    // Setup example model, texture
    std::vector<IndexModel> models{TestVerts::Rect(), TestVerts::RectPerspectiveCorrect()};
    std::shared_ptr<TextureWrapper> tex = m_texManager->Load("Assets/checkerboard.jpg", m_display->GetRenderer());

    bool isRunning = true;
    float accumulatedTime = 0.0f;
    m_timer->Reset();
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
                DoKeyDown(&e.key);
                break;
            }
            }
        }

        // Even if the game is paused, we should still accept inputs, e.g., menu screen.
        if (isPaused) { continue; }

        m_timer->Tick();
        float dt = (float)m_timer->GetDeltaTime();
#if 0
        accumulatedTime += dt;
        // Spiral of Doom protection
        if (accumulatedTime > dt * 8)
        {
            accumulatedTime = dt;
        }
#endif

        // Frame statistics
        static int totalFrameCnt = 0;
        static double timeElapsed = 0.0;
        if (m_timer->GetTotalTime() - timeElapsed > 1.0)
        {
            m_display->ShowFrameStatistics(dt, (float)m_timer->GetTotalTime() / totalFrameCnt, totalFrameCnt);
            ++timeElapsed;
        }

#if 0
        std::vector<Vec3f> verts{{-1.0f, -1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {1.0f, -1.0f, 0.0f},};
        std::vector<int> vertIndex{0, 1, 2, 2, 1, 0};
        IndexModel myModel{verts, vertIndex};
#endif

#if 0
        while (accumulatedTime >= dt)
        {
            Update(dt);
            accumulatedTime -= dt;
        }
#endif
#if 0
        // Physics loop
        float rotatedPerSec = (float)M_PI * dt / 4;
        Mat44f rotMat = Math::InitRotation(0.0f, rotatedPerSec, 0.0f);
        for (int i = 0, n = (int)model.verts.size(); i < n; ++i)
        {
            model.verts[i] = MultiplyVecMat(model.verts[i], rotMat);
        }
#endif

        // Render loop
        m_display->Update(dt, models, *tex);
#if 0
        // Peek into the future and generate the output
        Render(accumulatedTime / dt);
#endif

        ++totalFrameCnt;
    }

}

void QApp::Shutdown()
{
    m_texManager->Shutdown();
    m_display->Shutdown();

    SDL_Quit();
}

