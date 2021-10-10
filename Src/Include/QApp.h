#pragma once
#include <memory>
#include <vector>

// Forward declarations
struct SDL_Window;
struct SDL_KeyboardEvent;
class Display;
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
    
    bool Init(int w, int h);
    void Start();
    void Shutdown();

    void ShowFrameStatistics(int frameCnt, float dt, double timeElapsed);

private:
    bool m_isPaused;

    SDL_Window *m_window;
    std::unique_ptr<Display> m_display;
};