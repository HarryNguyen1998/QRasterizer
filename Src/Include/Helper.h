#pragma once
#include <chrono>
#include <iostream>

namespace Helper
{
	// Compare 2 floats, adapted from embeddeduse.com/2019/08/26/qt-compare-two-floats/
	// Return true if they are equal, else false.
	bool CompareFloat(float f1, float f2);

    float Min3(float a, float b, float c);
    float Max3(float a, float b, float c);

    using C = std::chrono::steady_clock;
    // Log the time operations took in us and ms to console. Usage: enclose the operations you want
    // to time within a scope and add a Timer ctor at the beginning of the scope. After dtor, it
    // automatically print the duration. One can also manipulate the Timer manually by calling
    // SetManual(true); then use Start() and Stop()
    class Timer
    {
    public:
        Timer();
        ~Timer();
        void SetManual(bool shouldManual);
        void Start();
        void Stop();
    private:
        std::chrono::time_point<C> start;
        bool shouldManual;
    };
}