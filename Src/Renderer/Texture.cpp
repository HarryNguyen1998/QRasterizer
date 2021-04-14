#include "SDL.h"
#include "SDL_image.h"

#include <cassert>
#include <memory>

#include "Renderer/Texture.h"

// @todo Recover from exceptions, e.g., img fails to load
void TextureWrapper::Acquire(const std::string& filePath, SDL_Renderer* renderer)
{
    SDL_Surface* tempSurf = IMG_Load(filePath.c_str());
    if (!tempSurf) { assert(1 == 0 && "Can't load img file!"); }
    texObj = SDL_CreateTextureFromSurface(renderer, tempSurf);
    w = tempSurf->w;
    h = tempSurf->h;

    SDL_FreeSurface(tempSurf);
    if (!texObj) { assert(1 == 0 && "Texture can't be created from surface!"); }
}

void TextureWrapper::Render(SDL_Renderer* renderer, int x, int y,
    SDL_Rect* clip, SDL_RendererFlip flip)
{
    // destRect is where on the screen (and size) we will render to.
    SDL_Rect destRect{x, y, w, h};
    if (clip)
    {
        destRect.w = clip->w;
        destRect.h = clip->h;
    }

    SDL_RenderCopyEx(renderer, texObj,
                     clip, &destRect, 0, 0, flip);
}

void TextureWrapper::RenderPixel(SDL_Renderer* pRenderer, 
    int x, int y, int width, int height, int currentRow, int currentFrame,SDL_RendererFlip flip)
{
    // @todo Implement pixel
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

void TextureWrapper::Release()
{
    SDL_DestroyTexture(texObj);
}

int TextureWrapper::Width() const { return w; }
int TextureWrapper::Height() const { return h; }

TextureManager& TextureManager::Instance()
{
    static TextureManager instance;
    return instance;
}

// @todo Recover from exceptions, e.g., img fails to load
std::shared_ptr<TextureWrapper> TextureManager::Load(const std::string& filePath, SDL_Renderer* renderer)
{
    // If already loaded, simply return the texture to be shared and re-used
    auto texIt = loadedTexs.find(filePath);
    if (texIt != loadedTexs.end()) { return texIt->second;  }

    // Else, load the texture
    TextureWrapper* newTexture = new TextureWrapper{};
    newTexture->Acquire(filePath, renderer);
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

    unloadedTexIt->second->Release();
    loadedTexs.erase(unloadedTexIt);
}