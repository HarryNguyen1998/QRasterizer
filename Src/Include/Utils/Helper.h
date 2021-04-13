#pragma once
#include <algorithm>
#include <cmath>
#include <type_traits>
#include <vector>
#include <string>   // std::getline

namespace Helper
{
    template<typename T>
    inline typename std::enable_if_t<std::is_integral<T>::value, bool> IsEqual(T a, T b)
	{
        return (a == b);
	}

    // Resource: https://www.learncpp.com/cpp-tutorial/relational-operators-and-floating-point-comparisons/
    template<typename T>
    inline typename std::enable_if_t <std::is_floating_point<T>::value, bool > IsEqual(T a, T b)
	{
		float epsilon = 1.0e-05f;
		if (std::abs(a - b) <= epsilon)
			return true;
		return std::abs(a - b) <= epsilon * std::fmax(std::abs(a), std::abs(b));
	}

    template<typename T>
    T Min3(T a, T b, T c) { return std::min(a, std::min(b, c)); }

    template<typename T>
    T Max3(T a, T b, T c) { return std::max(a, std::max(b, c)); }

    // Simple func to split a string using a delimiter
    std::vector<std::string> Split(const std::string& s, char delim);
    
}