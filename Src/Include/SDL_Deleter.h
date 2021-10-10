// Forward declarations
struct SDL_Window;

void SDL_DeleteWindow(SDL_Window*);

struct SDL_Deleter
{
    void operator()(SDL_Window *window) { SDL_DeleteWindow(window); }
};