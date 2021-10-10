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
        std::enable_if_t< std::is_integral<T>::value >* = nullptr >
    constexpr bool IsEqual(T a, T b)
    {
        return (a == b);
    }

    // @see https://www.learncpp.com/cpp-tutorial/relational-operators-and-floating-point-comparisons/
    // @note Can't be constexpr because abs function isnt constexpr
    template<typename T,
        std::enable_if_t< std::is_floating_point<T>::value >* = nullptr >
    bool IsEqual(T a, T b)
	{
		double absEpsilon = 1e-12;
        double relEpsilon = 1e-5;
		if (std::fabs(a - b) <= absEpsilon)
			return true;
		return std::fabs(a - b) <= relEpsilon * std::max(std::fabs(a), std::fabs(b));
	}

    template<typename T>
    T Min3(T a, T b, T c) { return std::min(a, std::min(b, c)); }

    template<typename T>
    T Max3(T a, T b, T c) { return std::max(a, std::max(b, c)); }
}