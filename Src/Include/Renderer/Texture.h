#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "SDL_Deleter.h"

// @brief A wrapper that handles texture data, using SDL_Texture
class QTexture
{
public:
    void Init(const std::string& filePath, SDL_Renderer *renderer);
    void LockTexture();
    void UnlockTexture();

    int GetW() const;
    int GetH() const;
    int GetPitch() const;
    const uint32_t *GetTexels() const;

private:
    std::unique_ptr<SDL_Texture, SDL_Deleter> m_texture;
    int m_w, m_h, m_pitch;
    uint32_t *m_texels;
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
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    static TextureManager& Instance();
    
    std::shared_ptr<QTexture> Load(const std::string& filePath, SDL_Renderer *renderer);
    void Unload(const std::string& filePath);

    // @brief We can manually calls UnloadAll() to reuse again.
    void UnloadAll();

private:
    TextureManager() = default;

private:
    std::unordered_map<std::string, std::shared_ptr<QTexture>> loadedTexs;
};

