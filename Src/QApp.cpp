#include "SDL.h"
#include "SDL_image.h"

#include <cassert>
#include <memory>
#include <string>
#include <sstream>

#include "QApp.h"
#include "Renderer/OBJLoader.h"
#include "Renderer/QRenderer.h"
#include "Renderer/Model.h"
#include "Renderer/Texture.h"

QApp& QApp::Instance()
{
    static QApp app{};
    return app;
}

bool QApp::Init(std::string title, int w, int h)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "Failed to initialize SDL! Error is: " << SDL_GetError() << "\n";
        return false;
    }

    int imgFlags = IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        std::cerr << "SDL_image can't be initialized! SDL_image error: " << IMG_GetError();
        return false;
    }

    m_title = std::move(title);
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

void QApp::LoadModel(Model model)
{
    m_models.push_back(std::move(model));
}

void QApp::LoadTexture(const std::string& textureFilePath)
{
    assert(!m_models.empty() && "Load model first.");
    m_textures.push_back(TextureManager::Instance().Load(textureFilePath, m_qrenderer->GetRenderer()));
    m_modelToTextureIndex.insert(std::make_pair((int)m_models.size() - 1, (int)m_textures.size() - 1));
}

void QApp::SetDrawMode(QRendererMode drawMode)
{
    m_drawMode = drawMode;
}

void QApp::Start()
{
    assert(!m_models.empty() && "models is empty.");

    // deltatime
    const float secsPerCnt = 1.0f / SDL_GetPerformanceFrequency();
    uint64_t startCounts = SDL_GetPerformanceCounter();
    float accumulatedTime = 0.0;
    int frameCnt = 0;

    // Setup
    Vec3f eye{0.0f, 0.0f, 3.0f};

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

        std::vector<Model> changedModels = m_models;
        rotAmount += 0.45f * dt;
        Mat44f rotMonkeyMat = Math::InitRotation(0, 0.0f, rotAmount);
        Mat44f rotCubeMat = Math::InitRotation(-rotAmount, rotAmount, 0.0f);
        Mat44f moveMonkeyMat = Math::InitTranslation(1.5f, 0.0f, 0.0f);
        Mat44f moveCubeMat = Math::InitTranslation(-1.5f, 0.0f, 0.0f);
        for (int modelIndex = 0; modelIndex < changedModels.size(); ++modelIndex)
        {
            for (int vertIndex = 0; vertIndex < changedModels[modelIndex].verts.size(); ++vertIndex)
            {
                // Move monkey to right, cube to left
                if (modelIndex == 0)
                {
                    changedModels[modelIndex].verts[vertIndex] =
                        Math::MultiplyVecMat(changedModels[modelIndex].verts[vertIndex], rotMonkeyMat);
                }
                if (modelIndex == 2)
                {
                    changedModels[modelIndex].verts[vertIndex] =
                        Math::MultiplyVecMat(changedModels[modelIndex].verts[vertIndex], rotCubeMat * moveCubeMat);
                }
                
                changedModels[modelIndex].verts[vertIndex] =
                    Math::MultiplyVecMat(changedModels[modelIndex].verts[vertIndex], viewMat);
            }

        }

        
        // Rendering
        for (int i = 0; i < changedModels.size(); ++i)
        {
            auto it = m_modelToTextureIndex.find(i);
            // If found texture, draw with texture, else draw with color
            if (it != m_modelToTextureIndex.end())
                m_qrenderer->Render(changedModels[i], m_textures[it->second], m_drawMode);
            else
                m_qrenderer->Render(changedModels[i], m_drawMode);

        }

        m_qrenderer->SwapBuffers();
        

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

