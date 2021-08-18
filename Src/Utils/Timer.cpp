#include "SDL.h"

#include <iostream>

#include "Utils/Timer.h"

Timer::Timer() :
    m_prevTime(0), m_startTime(0), m_deltaTime(0.0)
{
    m_secPerCnt = 1.0 / SDL_GetPerformanceFrequency();
}

void Timer::Reset()
{
    long long now = SDL_GetPerformanceCounter();
    m_isPaused = false;
    m_prevTime = now;
    m_startTime = now;
    m_deltaTime = 0.0;
}

void Timer::Unpause()
{
    if (m_isPaused)
    {
        m_isPaused = false;
        m_prevTime = SDL_GetPerformanceCounter();
    }
}

void Timer::Pause()
{
    if (!m_isPaused)
    {
        m_isPaused = true;
    }
}

void Timer::Tick()
{
    long long now = SDL_GetPerformanceCounter();
    m_deltaTime = (now - m_prevTime) * m_secPerCnt;
    m_prevTime = now;
}

double Timer::GetDeltaTime() const { return m_deltaTime; }
double Timer::GetTotalTime() const { return (SDL_GetPerformanceCounter() - m_startTime) * m_secPerCnt; }

