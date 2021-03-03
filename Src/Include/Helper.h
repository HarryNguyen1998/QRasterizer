#pragma once
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <type_traits>

namespace Helper
{
    template<typename T>
    typename std::enable_if_t<std::is_integral<T>::value, bool> IsEqual(T a, T b)
	{
        return (a == b);
	}

    // Resource: https://www.learncpp.com/cpp-tutorial/relational-operators-and-floating-point-comparisons/
    template<typename T>
    typename std::enable_if_t <std::is_floating_point<T>::value, bool > IsEqual(T a, T b)
	{
		static constexpr float epsilon = 1.0e-05f;
		if (std::abs(a - b) <= epsilon)
			return true;
		return std::abs(a - b) <= epsilon * std::fmax(std::abs(a), std::abs(b));
	}

    float Min3(float a, float b, float c) { return std::min(a, std::min(b, c)); }
    float Max3(float a, float b, float c) { return std::max(a, std::max(b, c)); }

    using C = std::chrono::steady_clock;
    // Log the time operations took in us and ms to console. Usage: enclose the operations you want
    // to time within a scope and add a Timer ctor at the beginning of the scope. After dtor, it
    // automatically print the duration. One can also manipulate the Timer manually by calling
    // SetManual(true); then use Start() and Stop()
    class Timer
    {
    public:
        Timer() : start{C::now()}, shouldManual{false} {}
        ~Timer() { if (!shouldManual) { Stop(); } }
        void SetManual(bool shouldManual) { shouldManual = shouldManual; }
        void Start() { start = C::now(); }
        void Stop()
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
    private:
        std::chrono::time_point<C> start;
        bool shouldManual;
    };
}