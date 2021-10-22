#include "SDL.h"
#include "SDL_image.h"

#include <cassert>
#include <string>
#include <sstream>

#include "QApp.h"
#include "Renderer/OBJLoader.h"
#include "Renderer/QRenderer.h"
#include "Renderer/IndexModel.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Texture.h"

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
#if 1
    std::vector<Vec3f> colors{
        {1.0f, 1.0f, 0.0f}, // yellow
        {1.0f, 0.0f, 1.0f}, // purple
        {0.0f, 1.0f, 1.0f}, // cyan
    };
    Model model{OBJ::LoadFileData("Assets/cube.obj")};
    QTexture texture = m_qrenderer->CreateTexture("Assets/texture-test.jpg");
#else
#if 1
    std::vector<Vec3f> tri{
        {-0.9f, 0.9f, 1.0f},
        {0.9f, 0.9f, -1.0f},
        {0.0f, 0.0f, 0.0f},
    };
#endif
#if 0
    std::vector<Vec3f> tri{
        {-0.5f, -0.5f, 0.0f},
        {0.0f, 0.5f, 0.0f},
        {0.5f, -0.5f, 0.0f},
    };
#endif

    // CCW
    std::vector<int> indices{
        0, 2, 1
    };
    std::vector<Vec3f> colors{
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
    };
    Model model{tri, indices};
#endif

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
    const float secsPerCnt = 1.0f / SDL_GetPerformanceFrequency();
    uint64_t startCounts = SDL_GetPerformanceCounter();
    float accumulatedTime = 0.0;
    int frameCnt = 0;


    Vec3f eye{0.0f, 0.0f, 4.0f};

    // Main loop
    const uint8_t *isKeyHeldDown = SDL_GetKeyboardState(nullptr);
    bool isRunning = true;
    float rotAmount = 0;
    float yaw = 0.0f;   // Amount of rotation in lookDir
    constexpr float pi = 3.141592653589f;
    m_qrenderer->SetProjectionMatrix(Math::InitPersp(pi / 2.0f, (float)m_w / m_h, 0.5f, 100.0f));
    while (isRunning)
    {
        uint64_t endCounts = SDL_GetPerformanceCounter();
        float dt = (float)(endCounts - startCounts) * secsPerCnt;
        startCounts = endCounts;

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
                    m_isPaused = !m_isPaused;
            }
            }
        }

        // Camera mvmt
        if (isKeyHeldDown[SDL_SCANCODE_Q])
        {
            eye.y -= 2.0f * dt;
        }
        if (isKeyHeldDown[SDL_SCANCODE_E])
        {
            eye.y += 2.0f * dt;
        }
        if (isKeyHeldDown[SDL_SCANCODE_W])
        {
            eye.z -= 2.0f * dt;
        }
        if (isKeyHeldDown[SDL_SCANCODE_S])
        {
            eye.z += 2.0f * dt;
        }
        if (isKeyHeldDown[SDL_SCANCODE_D])
        {
            eye.x += 2.0f * dt;
        }
        if (isKeyHeldDown[SDL_SCANCODE_A])
        {
            eye.x -= 2.0f * dt;
        }
        if (isKeyHeldDown[SDL_SCANCODE_RIGHT])
        {
            yaw -= 2.0f * dt;
        }
        if (isKeyHeldDown[SDL_SCANCODE_LEFT])
        {
            yaw += 2.0f * dt;
        }

        if (m_isPaused) { continue; }

        // Rotate model
        Vec3f lookDir{0.0f, 0.0f, -1.0f};
        Mat44f leftOrRightMat = Math::InitRotation(0.0f, 0.0f, yaw);
        Vec3f at = Math::MultiplyVecMat(lookDir, leftOrRightMat);
        at += eye;
        Mat44f viewMat = m_qrenderer->LookAt(eye, at);
        Model changedModel = model;
        //Mat44f rotMat = Math::InitRotation(0.0f, 0.0f, 0.0f);
        for (int i = 0; i < changedModel.verts.size(); ++i)
        {
            //changedModel.verts[i] = Math::MultiplyVecMat(changedModel.verts[i], rotMat);
            changedModel.verts[i] = Math::MultiplyVecMat(changedModel.verts[i], viewMat);
        }

        
        // Rendering
        QRenderer::Mode drawMode = QRenderer::Mode::kNone;
        m_qrenderer->Render(changedModel, texture, colors, drawMode);

        // Frame statistics every 2s
        ++frameCnt;
        accumulatedTime += dt;
        if (accumulatedTime > 1.0f)
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

void QApp::ShowFrameStatistics(int frameCnt, float dt)
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

