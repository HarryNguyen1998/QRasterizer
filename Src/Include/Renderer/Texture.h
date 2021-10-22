#pragma once
#include "SDL.h"

#include <memory>
#include <string>
#include <unordered_map>

#include "SDL_Deleter.h"

// @brief A wrapper that handles texture data, using SDL_Texture
// @note Do i need an IsEmpty() method, or handle that case in Release is better?
class QTexture
{
public:
    void Init(const std::string& filePath, SDL_Renderer *renderer);
    void LockTexture();
    void UnlockTexture();

    // @brief Blit the texture to the rendering target
    // @param clip is the width and height we take from texture to blit to the rendering target. If
    // nullptr, take the width and height of the texture
    // @param flip Should the texture be flipped horizontally/vertically or both?
    // @remark Blit from (startX, startY) to (w, h) of clip
    // @note Should i add angle, center in the future?
    void Draw(SDL_Renderer *renderer, int startX, int startY,
         SDL_Rect *clip, SDL_RendererFlip flip);

    void RenderPixel(SDL_Renderer *renderer, int srcX, int srcY, int destX, int destY) const;

    int GetW() const;
    int GetH() const;
    int GetPitch() const;
    SDL_Texture *GetTexture() const;
    uint32_t *GetPixels();

private:
    std::unique_ptr<SDL_Texture, SDL_Deleter> m_texture;
    int m_w, m_h, m_pitch;
    uint32_t *m_pixels;
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
    
    // @brief calls Shutdown() if users don't do that
    void Shutdown();

    std::shared_ptr<QTexture> Load(const std::string& filePath, SDL_Renderer *renderer);
    void Unload(const std::string& filePath);

    // @brief We can manually calls UnloadAll() to reuse again. Else, calls Shutdown()
    void UnloadAll();

private:
    std::unordered_map<std::string, std::shared_ptr<QTexture>> loadedTexs;

    // @brief 
    bool m_hasCalledUnloadAll;
};

