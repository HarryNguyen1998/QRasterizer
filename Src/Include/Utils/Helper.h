#pragma once
#include <algorithm>
#include <cmath>        // std::abs
#include <type_traits>

    // typename std::enable_if<!std::is_arithmetic<T>::value, std::nullptr_t>::type = nullptr
    // typename std::enable_if<!std::is_arithmetic_v<T>, std::nullptr_t>::type = nullptr
    // std::enable_if_t<!std::is_arithmetic_v<T>, std::nullptr_t> = nullptr
    // typename = std::enable_if_t<std::is_arithmetic_v<T>>

// @brief Useful core funcs like comparing floating-point values, get the min/max between 3 values, etc.
namespace Helper
{
    // @note This is for integer types
    template<typename T,
        std::enable_if_t< std::is_integral_v<T> >* = nullptr >
    constexpr bool IsEqual(T a, T b)
    {
        return (a == b);
    }

    // @see https://www.learncpp.com/cpp-tutorial/relational-operators-and-floating-point-comparisons/
    // @note Can't be constexpr because abs function isnt constexpr
    template<typename T,
        std::enable_if_t< std::is_floating_point_v<T> >* = nullptr >
    bool IsEqual(T a, T b)
	{
        // @note Had to be this large since std::cos(M_PI/2) = -4.37113883e-8 which evaluates to
        // false with 0.0 in rotMat test code??
		T absEpsilon = (T)1e-5;
        T relEpsilon = (T)1e-5;
        bool result = true;
        T diff = std::abs(a - b);
        // Comparing numbers near zero.
		if (diff <= absEpsilon)
			return result;
		result = diff <= (relEpsilon * std::max(std::abs(a), std::abs(b)));
        return result;
	}

    template<typename T>
    T Min3(T a, T b, T c) { return std::min(a, std::min(b, c)); }

    template<typename T>
    T Max3(T a, T b, T c) { return std::max(a, std::max(b, c)); }

    // @brief Interpolate from a to b
    template<typename T, typename FloatType,
        std::enable_if_t< std::is_floating_point_v<FloatType> >* = nullptr >
    T Interpolate(T a, T b, FloatType t) { return a + t * (b - a); }
}