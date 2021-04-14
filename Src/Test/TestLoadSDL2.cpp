#include "SDL.h"
#include "SDL_image.h"

#include <cassert>
#include <cstdlib>

void TestInitSDL2()
{
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    
    SDL_Window* window = SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, 0);
    assert(window);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    assert(renderer);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void TestLoadJpegImage()
{
    SDL_Surface* surface = IMG_Load("bricks.jpg");
    assert(surface);

    SDL_FreeSurface(surface);
}

int main(int argc, char** argv)
{
    TestInitSDL2();
    TestLoadJpegImage();
    atexit(SDL_Quit);

    return 0;
}
