#include "SDL.h"
#include "SDL_image.h"

#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>

#include "Renderer/Texture.h"

void QTexture::Init(const std::string& filePath, SDL_Renderer *renderer)
{
    std::unique_ptr<SDL_Surface, SDL_Deleter> tempSurf{IMG_Load(filePath.c_str())};
    if (!tempSurf) { assert(1 == 0 && "Uh oh, cannot load img file."); }
    std::unique_ptr<SDL_Surface, SDL_Deleter> formattedSurf{SDL_ConvertSurfaceFormat(tempSurf.get(), SDL_PIXELFORMAT_RGBA32, 0)};
    if (!formattedSurf) { assert(1 == 0 && "Uh oh, cannot format surface."); }

    m_w = formattedSurf->w;
    m_h = formattedSurf->h;
    m_texture.reset(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, m_w, m_h));
    if (!m_texture) { assert(1 == 0 && "Uh oh, cannot create texture."); }
    
    // Write data from loaded surface to our texture
    SDL_LockTexture(m_texture.get(), nullptr, &(void *)m_texels, &m_pitch);
    if (!m_texels) { assert(1 == 0 && "Uh oh, cannot get pixels!"); }
    memcpy(m_texels, formattedSurf->pixels, m_pitch * m_h);
    // @note Do I need to copy color key?
    SDL_UnlockTexture(m_texture.get());
    m_texels = nullptr;
}

void QTexture::LockTexture()
{
    if (m_texels != nullptr)
        std::cerr << "Texture has already been locked!\n";
    else
    {
        if (SDL_LockTexture(m_texture.get(), nullptr, &(void*)m_texels, &m_pitch) != 0)
            std::cerr << "Failed to lock texture!\n";
    }
}

void QTexture::UnlockTexture()
{
    if (m_texels == nullptr)
        std::cerr << "Texture has already been unlocked!\n";
    else
    {
        SDL_UnlockTexture(m_texture.get());
        m_texels = nullptr;
    }
}

int QTexture::GetW() const { return m_w; }
int QTexture::GetH() const { return m_h; }
int QTexture::GetPitch() const { return m_pitch; }

const uint32_t *QTexture::GetTexels() const { return m_texels; }

TextureManager& TextureManager::Instance()
{
    static TextureManager textureManager{};
    return textureManager;
}

// @todo Recover from exceptions, e.g., img fails to load
std::shared_ptr<QTexture> TextureManager::Load(const std::string& filePath, SDL_Renderer* renderer)
{
    if (filePath.empty()) { assert(1 == 0 && "Filename can't be empty!"); }

    // If already loaded, simply return the texture to be shared and re-used
    auto texIt = loadedTexs.find(filePath);
    if (texIt != loadedTexs.end()) { return texIt->second;  }

    // Else, load the texture
    QTexture* newTexture = new QTexture{};
    newTexture->Init(filePath, renderer);
    std::shared_ptr<QTexture> newTextureHandle{newTexture};

    // Now, cache it so it can be re-used in the future
    loadedTexs.insert(std::make_pair(filePath, newTextureHandle));

    return newTextureHandle;
}

void TextureManager::Unload(const std::string& filePath)
{
    if (filePath.empty()) { assert(1 == 0 && "Filename can't be empty!"); }

    auto unloadedTexIt = loadedTexs.find(filePath);
    if (unloadedTexIt == loadedTexs.end()) { assert(1 == 0 && "Can't find texture!"); }

    loadedTexs.erase(unloadedTexIt);
}

void TextureManager::UnloadAll()
{
    if (loadedTexs.empty()) { return; }

    loadedTexs.clear();
}

