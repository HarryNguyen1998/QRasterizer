#pragma once
#include <memory>
#include <vector>

// Forward declarations
struct SDL_KeyboardEvent;
class Display;
class TextureManager;
class Timer;

// @brief a Sort-of controller that handles "when" to init/shutdown its members, main loop
class QApp
{
public:
    // @note Singleton pattern
    QApp() = default;
    QApp(const QApp&) = delete;
    QApp& operator=(const QApp&) = delete;
    static QApp& Instance();
    
    bool Init();
    void Start();
    void Shutdown();

private:
    bool m_isPaused;

    std::unique_ptr<Display> m_display;
    std::unique_ptr<Timer> m_globalTimer;
};