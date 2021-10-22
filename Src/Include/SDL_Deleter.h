#pragma once
// Forward declarations
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Surface;

struct SDL_Deleter
{
    void operator()(SDL_Window *window);
    void operator()(SDL_Renderer *renderer);
    void operator()(SDL_Texture *texture);
    void operator()(SDL_Surface *surface);
};