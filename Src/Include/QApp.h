#pragma once
#include <memory>
#include <vector>

#include "SDL_Deleter.h"

// Forward declarations
class QRenderer;

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

    void ShowFrameStatistics(int frameCnt, float dt);

private:
    QApp() = default;
private:
    bool m_isPaused;

    std::string m_title;
    int m_w, m_h;
    std::unique_ptr<SDL_Window, SDL_Deleter> m_window;

    // @note Differentiate with SDL_Renderer
    std::unique_ptr<QRenderer> m_qrenderer;
};