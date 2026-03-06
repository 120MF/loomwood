#include <engine/InputSystem.hpp>
#include <SDL3/SDL.h>

namespace lw
{

void InputSystem::process(bool& isRunning)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            isRunning = false;
        }
        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.key == SDLK_ESCAPE)
            {
                isRunning = false;
            }
        }
    }
}

} // namespace lw
