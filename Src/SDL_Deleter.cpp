#include "SDL.h"

#include "SDL_Deleter.h"

void SDL_Deleter::operator()(SDL_Window *window)
{
    SDL_DestroyWindow(window);
}

void SDL_Deleter::operator()(SDL_Renderer *renderer)
{
    SDL_DestroyRenderer(renderer);
}

void SDL_Deleter::operator()(SDL_Texture *texture)
{
    SDL_DestroyTexture(texture);
}
