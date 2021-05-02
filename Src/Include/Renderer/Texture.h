#pragma once
#include "SDL.h"

#include <memory>
#include <string>
#include <unordered_map>

// @brief A wrapper that handles texture data, using SDL_Texture
// @todo  Do i need an IsEmpty() method, or handle that case in Release is better?
class TextureWrapper
{
public:
    // @brief Create empty texture to be drawn on (should only be the bitmap that will be output on screen)
    // @todo Recover from exceptions, e.g., fails to create texture
    void Init(SDL_Renderer* renderer, int w, int h, int bpp);

    void Init(const std::string& filePath, SDL_Renderer* renderer);

    // @brief Render the tex on screen. Whether the whole tex or a portion of it is rendered
    // depends on the clip param.
    // @param clip specifies the portion we'll take from the texture. If nullptr, the whole
    // texture will be rendered.
    // @param flip Should the texture be flipped horizontally/vertically or both
    // @note Should i add angle, center in the future?
    void Render(SDL_Renderer* renderer, int destX, int destY,
         SDL_Rect* clip, SDL_RendererFlip flip);

    // @brief Render a texel from texture to screen.
    // @todo How do i flip the final img??
    void RenderPixel(SDL_Renderer* renderer, int srcX, int srcY, int destX, int destY) const;

    void Shutdown();

    int Width() const;
    int Height() const;

private:
    SDL_Texture *m_texObj;
    int m_w, m_h;

    // @brief Number of bits per pixel
    int m_bpp;
};

// @details A resource manager that manages shareable and reusable textures. Responsibilities:
// if texture is already loaded, it simply returns handle to the already loaded texture. It 
// may cache the result, e.g., if last ref to a resource has been dropped, the manager may
// choose to keep it in mem in case it gets loaded again in the future.

// @todo Reload() to update resources while updating the game at runtime,
// In the future, loading routine should base on stream. This would help in, say retrieving assets
// through a network, some archives, or simply from file. Ex: LoadFromStream() calls LoadFromFile(),
// which loads file from mem and passes to GetFromFile(), which creates a stream and pass to
// GetFromStream()
class TextureManager
{
public:
    // @note Singleton pattern
    TextureManager() = default;
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    
    // @brief Initialization SDL_Img
    bool Init();

    // @brief calls Shutdown() if users don't do that, also shutdown SDL_Img
    void Shutdown();

    std::shared_ptr<TextureWrapper> Load(const std::string& filePath, SDL_Renderer* renderer);
    void Unload(const std::string& filePath);

    // @brief We can manually calls UnloadAll() to reuse again. Else, calls Shutdown()
    void UnloadAll();

private:
    std::unordered_map<std::string, std::shared_ptr<TextureWrapper>> loadedTexs;

    // @brief 
    bool b_hasCalledUnloadAll;
};

