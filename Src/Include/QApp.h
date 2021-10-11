#pragma once
#include <memory>
#include <vector>

#include "SDL_Deleter.h"

// Forward declarations
struct SDL_KeyboardEvent;
class QRenderer;
class TextureManager;

// @brief a Sort-of controller that handles "when" to init/shutdown its members, main loop
class QApp
{
public:
    // @note Singleton pattern
    QApp() = default;
    QApp(const QApp&) = delete;
    QApp& operator=(const QApp&) = delete;
    static QApp& Instance();
    
    bool Init(const std::string& title, int w, int h);
    void Start();
    void Shutdown();

    void ShowFrameStatistics(int frameCnt, double dt);

private:
    bool m_isPaused;

    std::string m_title;
    int m_w, m_h;
    std::unique_ptr<SDL_Window, SDL_Deleter> m_window;

    // @note Differentiate with SDL_Renderer
    std::unique_ptr<QRenderer> m_qrenderer;
};