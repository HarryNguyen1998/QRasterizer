#include "SDL.h"
#include "SDL_image.h"

#include <cassert>
#include <iostream>
#include <memory>

#include "Renderer/Texture.h"

void TextureWrapper::Init(SDL_Renderer* renderer, int w, int h, int bpp)
{
    m_texObj = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, w, h);
    m_w = w;
    m_h = h;
    m_bpp = bpp;
}

void TextureWrapper::Init(const std::string& filePath, SDL_Renderer* renderer)
{
    SDL_Surface* tempSurf = IMG_Load(filePath.c_str());
    if (!tempSurf) { assert(1 == 0 && "Can't load img file!"); }
    m_texObj = SDL_CreateTextureFromSurface(renderer, tempSurf);
    m_w = tempSurf->w;
    m_h = tempSurf->h;
    m_bpp = tempSurf->format->BitsPerPixel;

    SDL_FreeSurface(tempSurf);
    if (!m_texObj) { assert(1 == 0 && "Texture can't be created from surface!"); }
}

void TextureWrapper::Render(SDL_Renderer* renderer, int destX, int destY,
    SDL_Rect* clip, SDL_RendererFlip flip)
{
    // destRect is where on the screen (and size) we will render to.
    SDL_Rect destRect{destX, destY, m_w, m_h};
    if (clip)
    {
        destRect.w = clip->w;
        destRect.h = clip->h;
    }

    SDL_RenderCopyEx(renderer, m_texObj,
                     clip, &destRect, 0, 0, flip);
}

void TextureWrapper::RenderPixel(SDL_Renderer* renderer, 
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

bool TextureManager::Init()
{
    int imgFlags = IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        std::cout << "SDL_image can't be initialized! SDL_image error: " << IMG_GetError();
        return false;
    }

    return true;
}

void TextureManager::Shutdown()
{
    if (!b_hasCalledUnloadAll) { UnloadAll(); }

    IMG_Quit();
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

    b_hasCalledUnloadAll = true;
}

