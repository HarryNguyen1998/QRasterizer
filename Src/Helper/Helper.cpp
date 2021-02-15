#include "Helper.h"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace Helper
{
	bool CompareFloat(float f1, float f2)
	{
		static constexpr float epsilon = 1.0e-05f;
		if (std::abs(f1 - f2) <= epsilon)
			return true;
		return std::abs(f1 - f2) <= epsilon * std::fmax(std::abs(f1), std::abs(f2));
	}

    float Min3(float a, float b, float c) { return std::min(a, std::min(b, c)); }
    float Max3(float a, float b, float c) { return std::max(a, std::max(b, c)); }

    Timer::Timer() : start{C::now()}, shouldManual{false} {}
    Timer::~Timer() { if (!shouldManual) { Stop(); } }
    void Timer::SetManual(bool shouldManual) { shouldManual = shouldManual; }
    void Timer::Start() { start = C::now(); }
    void Timer::Stop()
    {
        // Cast the duration of now - start to microsec type.
        // Microsec type is alias of Duration type <typename Rep, typename Period>. Number of ticks
        // is type Rep, so if it's a float it can represent fraction of ticks. count() returns num of
        // ticks. Period is how many sec per tick. In this case microsec: std::ratio<1, 1000>
        // eli5: num of ticks is duration in microsec.
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(C::now() - start).count();
        double ms = us * 0.001;
        std::cout << us << "us (" << ms << "ms)";
        if (!shouldManual) { std::cout << "\n"; }
    }
}