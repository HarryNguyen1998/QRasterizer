#pragma once
#include <cmath>        // sqrt, tan, sin, cos
#include <initializer_list>
#include <type_traits>

#include "Utils/Helper.h"

namespace Math
{
	// @brief A vector math class.
    // @note Vector has an initializer_list ctor, so be careful with {} and () when initializing!
    // If you use list-initialization and don't provide enough arguments, the leftover elements are
    // 0-initialized.
    template<typename T, size_t Size>
    struct Vector
    {
        T e[Size];
        constexpr Vector() : e{} {}
        constexpr explicit Vector(T val) : e{}
        {
            for (int i = 0; i < Size; ++i)
                e[i] = val;
        }
        constexpr Vector(std::initializer_list<T> li)
            : e{}
        {
            int i = 0;
            for (const auto& element : li)
                e[i++] = element;
        }

		// Accessors, also case for passing by const ref
        constexpr const T& operator[](size_t i) const { return e[i]; }
        constexpr T& operator[](size_t i) { return e[i]; }

    };

    template<typename T>
    struct Vector<T, 2>
    {
        union
        {
            T e[2];
            struct { T x, y; };
        };
        constexpr Vector() : e{} {}
        constexpr explicit Vector(T val) : e{}
        {
            for (int i = 0; i < 2; ++i)
                e[i] = val;
        }
        constexpr Vector(std::initializer_list<T> li)
            : e{}
        {
            int i = 0;
            for (const auto& element : li)
                e[i++] = element;
        }

		// Accessors, also case for passing by const ref
        constexpr const T& operator[](size_t i) const { return e[i]; }
        constexpr T& operator[](size_t i) { return e[i]; }
    };

    template<typename T>
    struct Vector<T, 3>
    {
        union
        {
            T e[3];
            struct { T x, y, z; };
            struct { T r, g, b; };
        };
        constexpr Vector() : e{} {}
        constexpr explicit Vector(T val) : e{}
        {
            for (int i = 0; i < 3; ++i)
                e[i] = val;
        }
        constexpr Vector(std::initializer_list<T> li)
            : e{}
        {
            int i = 0;
            for (const auto& element : li)
                e[i++] = element;
        }

		// Accessors, also case for passing by const ref
        constexpr const T& operator[](size_t i) const { return e[i]; }
        constexpr T& operator[](size_t i) { return e[i]; }
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // List of available operations: 
    // Overloaded operators: ==, !=, <<, +-*/ (Vector addition and scaling), - (negation)
    // Useful functions: Dot, Cross, LengthSqr, Length, Normal

    // @note Comparison operators can only be constexpr if T is integral. This is due to using
    // IsEqual() helper function, which requires std::abs (not constexpr)
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    template<typename T, size_t Size>
    constexpr bool operator==(const Vector<T, Size>& v1, const Vector<T, Size>& v2)
    {
        bool result = true;
        for (int i = 0; i < Size; ++i) { result &= Helper::IsEqual<T>(v1[i], v2[i]); }
        return result;
    }

    template<typename T, size_t Size>
    constexpr bool operator!=(const Vector<T, Size>& v1, const Vector<T, Size>& v2)
    {
        return !(v1 == v2);
    }

    template<typename T, size_t Size>
    constexpr Vector<T, Size> operator-(const Vector<T, Size>& v)
    {
        Vector<T, Size> result;
        for (int i = 0; i < Size; ++i) { result[i] = -v[i]; }
        return result;
    }

    template<typename T, size_t Size>
    constexpr Vector<T, Size> operator+(const Vector<T, Size>& v1, const Vector<T, Size>& v2)
    {
        Vector<T, Size> result;
        for (int i = 0; i < Size; ++i) { result[i] = v1[i] + v2[i]; }
        return result;
    }

    template<typename T, size_t Size>
    constexpr Vector<T, Size> operator-(const Vector<T, Size>& v1, const Vector<T, Size>& v2)
    {
        return (v1 + (-v2));
    }

    template<typename T, size_t Size>
    constexpr Vector<T, Size> operator*(const Vector<T, Size>& v, T k)
    {
        Vector<T, Size> result;
        for (int i = 0; i < Size; ++i) { result[i] = v[i] * k; }
        return result;
    }

    template<typename T, size_t Size>
    constexpr Vector<T, Size> operator*(T k, const Vector<T, Size>& v)
    {
        return (v * k);
    }

    template<typename T, size_t Size>
    constexpr Vector<T, Size> operator/(const Vector<T, Size>& v, T k)
    {
        return (v * (1 / k));
    }

    template<typename T, size_t Size>
    constexpr Vector<T, Size>& operator+=(Vector<T, Size>& v1, const Vector<T, Size>& v2)
    {
        v1 = v1 + v2;
        return v1;
    }

    template<typename T, size_t Size>
    constexpr Vector<T, Size>& operator-=(Vector<T, Size>& v1, const Vector<T, Size>& v2)
    {
        v1 = v1 - v2;
        return v1;
    }

    template<typename T, size_t Size>
    constexpr Vector<T, Size>& operator*=(Vector<T, Size>& v, T k)
    {
        v = v * k;
        return v;
    }

    template<typename T, size_t Size>
    constexpr Vector<T, Size>& operator/=(Vector<T, Size>& v, T k)
    {
        v = v / k;
        return v;
    }

    template<typename T, size_t Size>
	inline std::ostream& operator<<(std::ostream& os, const Vector<T, Size> v)
	{
		std::ios_base::fmtflags oldFlags = os.flags();
		os.precision(5);
		os.setf(std::ios_base::fixed);
        os << "< ";
        for (int i = 0; i < Size; ++i)
        {
            os << v[i] << " ";
        }
        os << ">";
		os.flags(oldFlags);
		return os;
	}

	template<typename T, size_t Size>
	constexpr T Dot(const Vector<T, Size>& v1, const Vector<T, Size>& v2)
	{
        T result = 0;
        for (int i = 0; i < Size; ++i) { result += v1[i] * v2[i]; }
        return result;
	}

    template<typename T, size_t Size, typename = std::enable_if_t<(Size == 3)>>
	constexpr Vector<T, Size> Cross(const Vector<T, Size>& v1, const Vector<T, Size>& v2)
	{
        return Vector<T, Size>{
            v1[1] * v2[2] - v1[2] * v2[1],
            v1[2] * v2[0] - v1[0] * v2[2],
            v1[0] * v2[1] - v1[1] * v2[0]};
	}

	template<typename T, size_t Size>
	constexpr T LengthSqr(const Vector<T, Size>& v)
	{
        T result = 0;
        for (int i = 0; i < Size; ++i) { result += v[i] * v[i]; }
		return result;
	}

	template<typename T, size_t Size>
	constexpr T Length(const Vector<T, Size>& v)
	{
		return (T)std::sqrt(LengthSqr(v));
	}

    template<typename T, size_t Size,
        typename = std::enable_if_t<std::is_floating_point_v<T>> >
    constexpr Vector<T, Size> Normal(const Vector<T, Size>& v)
    {
        Vector<T, Size> result = v / Length(v);
        return result;
    }

}

using Vec2f = Math::Vector<float, 2>;
using Vec2i = Math::Vector<int, 2>;

using Vec3f = Math::Vector<float, 3>;
using Vec3i = Math::Vector<int, 3>;

