#pragma once
#include <memory>
#include <vector>

// Forward declarations
struct IndexModel;
class RenderContext;
struct SDL_Window;
struct SDL_Renderer;
class TextureWrapper;

// @brief Controls the window. Default convention is bpp=4
class Display
{
public:
    bool Init(int w, int h, const std::vector<float>& zBuffer);
    void Update(float dt, const std::vector<IndexModel>& models, const TextureWrapper& tex);
    void Shutdown();

    void ShowFrameStatistics(float dt, float avgFrameTime, int totalFrameCnt);

    SDL_Renderer *GetRenderer() const;

private:
    // @brief Title of the window
    std::string m_title;

    // @brief Information about the window being used for display
    SDL_Window *m_window;

    // @brief Information about rendering that is only used for the window
    SDL_Renderer *m_renderer;

    // @brief Bitmap used for the renderer
    std::unique_ptr<TextureWrapper> m_bitmap;

    std::unique_ptr<RenderContext> m_renderCtx;

    // @brief Z-buffer of the bitmap
    std::vector<float> m_zBuffer;
};