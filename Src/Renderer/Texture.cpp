#include "SDL.h"
#include "SDL_image.h"

#include <cassert>
#include <iostream>
#include <memory>

#include "Renderer/Texture.h"

void TextureWrapper::Init(const std::string& filePath, SDL_Renderer *renderer)
{
    SDL_Surface* tempSurf = IMG_Load(filePath.c_str());
    if (!tempSurf) { assert(1 == 0 && "Can't load img file!"); }
    m_texObj = SDL_CreateTextureFromSurface(renderer, tempSurf);
    m_w = tempSurf->w;
    m_h = tempSurf->h;
    m_format.reset(SDL_AllocFormat(tempSurf->format->format));

    SDL_FreeSurface(tempSurf);
    if (!m_texObj) { assert(1 == 0 && "Texture can't be created from surface!"); }
}

void TextureWrapper::UpdateTexture(unsigned char *pixels)
{
    SDL_UpdateTexture(m_texObj, nullptr, reinterpret_cast<const void*>(pixels), m_w *
        m_format->BytesPerPixel);
}

void TextureWrapper::Draw(SDL_Renderer *renderer, int startX, int startY,
    SDL_Rect *clip, SDL_RendererFlip flip)
{
    // destRect is where on the screen (and size) we will render to.
    SDL_Rect destRect{startX, startY, m_w, m_h};
    if (clip)
    {
        destRect.w = clip->w;
        destRect.h = clip->h;
    }

    SDL_RenderCopyEx(renderer, m_texObj, clip, &destRect, 0, 0, flip);
}

void TextureWrapper::RenderPixel(SDL_Renderer *renderer, 
    int srcX, int srcY, int destX, int destY) const
{
    SDL_Rect srcRect{srcX, srcY, 1, 1};
    SDL_Rect destRect{destX, destY, 1, 1};

    SDL_RenderCopy(renderer, m_texObj, &srcRect, &destRect);
#if 0
    SDL_Rect srcRect;
    SDL_Rect destRect;

    srcRect.x = width * currentFrame;
    srcRect.y = height * (currentRow - 1);
    srcRect.w = width;
    destRect.w = width;
    srcRect.h = height;
    destRect.h = height;
    destRect.x = x;
    destRect.y = y;

    SDL_RenderCopyEx(pRenderer, texObj,
                     &srcRect, &destRect, 0, 0, flip);
#endif
}

void TextureWrapper::Shutdown()
{
    SDL_DestroyTexture(m_texObj);
}

int TextureWrapper::Width() const { return m_w; }
int TextureWrapper::Height() const { return m_h; }

SDL_Texture *TextureWrapper::GetTextureObj() { return m_texObj; }
const SDL_PixelFormat *TextureWrapper::GetPixelFormat() const { return m_format.get(); }

void TextureManager::Shutdown()
{
    if (!m_hasCalledUnloadAll) { UnloadAll(); }
}

// @todo Recover from exceptions, e.g., img fails to load
std::shared_ptr<TextureWrapper> TextureManager::Load(const std::string& filePath, SDL_Renderer* renderer)
{
    // If already loaded, simply return the texture to be shared and re-used
    auto texIt = loadedTexs.find(filePath);
    if (texIt != loadedTexs.end()) { return texIt->second;  }

    // Else, load the texture
    TextureWrapper* newTexture = new TextureWrapper{};
    newTexture->Init(filePath, renderer);
    std::shared_ptr<TextureWrapper> newTextureHandle{newTexture};

    // Now, cache it so it can be re-used in the future
    loadedTexs.insert(std::make_pair(filePath, newTextureHandle));

    return newTextureHandle;
}

void TextureManager::Unload(const std::string& filePath)
{
    if (filePath.empty()) { assert(1 == 0 && "Filename can't be empty!"); }
    auto unloadedTexIt = loadedTexs.find(filePath);
    if (unloadedTexIt == loadedTexs.end()) { assert(1 == 0 && "Can't find texture!"); }

    unloadedTexIt->second->Shutdown();
    loadedTexs.erase(unloadedTexIt);
}

void TextureManager::UnloadAll()
{
    if (loadedTexs.empty()) { return; }

    for (auto it = loadedTexs.begin(); it != loadedTexs.end(); ++it)
    {
        it->second->Shutdown();
    }
    loadedTexs.clear();

    m_hasCalledUnloadAll = true;
}

