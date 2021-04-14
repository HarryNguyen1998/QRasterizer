#include "SDL.h"

#include <iostream>

#include "Utils/Timer.h"

Timer::Timer() :
    prevTime(0), startTime(0), deltaTime(0.0)
{
    secPerCnt = 1.0 / SDL_GetPerformanceFrequency();
}

void Timer::Reset()
{
    long long now = SDL_GetPerformanceCounter();
    isPaused = false;
    prevTime = now;
    startTime = now;
    deltaTime = 0;
}

void Timer::Unpause()
{
    if (isPaused)
    {
        isPaused = false;
        prevTime = SDL_GetPerformanceCounter();
    }
}

void Timer::Pause()
{
    if (!isPaused)
    {
        isPaused = true;
    }
}

void Timer::Tick()
{
    long long now = SDL_GetPerformanceCounter();
    deltaTime = (now - prevTime) * secPerCnt;
    prevTime = now;
}

double Timer::GetDeltaTime() const { return deltaTime; }
double Timer::GetTotalTime() const { return (SDL_GetPerformanceCounter() - startTime) * secPerCnt; }

