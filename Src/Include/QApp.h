#pragma once
#include <memory>
#include <vector>

// Forward declarations
struct SDL_KeyboardEvent;
class Display;
class TextureManager;
class Timer;

class QApp
{
public:
    // @note Singleton pattern
    QApp() = default;
    QApp(const QApp&) = delete;
    QApp& operator=(const QApp&) = delete;
    static QApp& Instance();
    
    void DoKeyDown(SDL_KeyboardEvent* e);

    // @brief Initialization and shutdown of the app
    bool Init();
    void Shutdown();

    void Start();
    void Update();

private:
    bool isPaused;

    std::unique_ptr<Display> m_display;
    std::unique_ptr<TextureManager> m_texManager;
    std::unique_ptr<Timer> m_timer;

};