#pragma once
#include <map>
#include <memory>
#include <vector>

#include "SDL_Deleter.h"

// Forward declarations
enum class QRendererMode;
class QRenderer;
class QTexture;
struct Model;

class QApp
{
public:
    // @note Singleton pattern
    QApp(const QApp&) = delete;
    QApp& operator=(const QApp&) = delete;
    static QApp& Instance();
    
    bool Init(std::string title, int w, int h);
    void Start();
    void Shutdown();

    void LoadModel(Model model);
    void LoadTexture(const std::string& textureFilePath);
    void SetDrawMode(QRendererMode drawMode);

private:
    QApp() = default;
    void ShowFrameStatistics(int frameCnt, float dt);
private:
    bool m_isPaused;

    std::string m_title;
    int m_w, m_h;
    std::unique_ptr<SDL_Window, SDL_Deleter> m_window;

    // Renderer stuffs
    QRendererMode m_drawMode;
    std::vector<Model> m_models;
    std::vector<std::shared_ptr<QTexture>> m_textures;
    std::map<int, int> m_modelToTextureIndex;

    // @note Differentiate with SDL_Renderer
    std::unique_ptr<QRenderer> m_qrenderer;
};