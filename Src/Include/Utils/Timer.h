#pragma once
#include <chrono>

// PURPOSE: High-precision timer that keeps track of time progression. The class uses SDL2 API for timing.
// NOTES: To use the timer, calls Reset(). Maybe add a pausedTime in the future, if needed?
class Timer
{
public:
    Timer();

    // Initialize the timer. This func should be called before using, or re-using a timer.
    void Reset();

    // If the timer is paused, then unpause it. If it's already running, do nothing.
    void Unpause();

    // If the timer is currently running, pause it. if it's already paused, do nothing.
    void Pause();

    // Called every frame to update the time. If it's paused, dt = 0.
    void Tick();

    // Returns the time elapsed between 2 frames; dt is updated during game loop with Tick()
    double GetDeltaTime() const;

    // Returns the total time has passed since this timer started.
    double GetTotalTime() const;

private:
    long long prevTime;
    long long startTime;
    double deltaTime;
    double secPerCnt;
    bool isPaused;
};
