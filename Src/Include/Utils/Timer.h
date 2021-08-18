#pragma once

// @brief High-precision timer that keeps track of time progression. The class uses SDL2 API for timing.
// @remark To use the timer, calls Reset().
// @note Timer doesn't have a Shutdown() since its members are arithmetic types.
class Timer
{
public:
    Timer();

    // @brief Initialize the timer. This func should be called before using, or re-using a timer.
    void Reset();

    // @brief If the timer is paused, then unpause it. If it's already running, do nothing.
    void Unpause();

    // @brief If the timer is currently running, pause it. if it's already paused, do nothing.
    void Pause();

    // @brief Called every frame to update the time. If it's paused, dt = 0.
    void Tick();

    // @brief Returns the time elapsed between 2 frames; dt is updated during game loop with Tick()
    double GetDeltaTime() const;

    // @brief Returns the total time has passed since this timer started.
    double GetTotalTime() const;

private:
    long long m_prevTime;
    long long m_startTime;
    double m_deltaTime;
    double m_secPerCnt;
    bool m_isPaused;
};
