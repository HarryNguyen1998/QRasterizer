#pragma once
#include <cmath>        // sqrt
#include <functional>   // std::hash

#include "Utils/Helper.h"

namespace Math
{
	// @brief A vector math class.
    // @note If default or value initialized, variadic ctor with 0 arg will be called instead, with
    // vals 0-initialized.
	template<typename T, size_t Size,
        typename = std::enable_if_t<std::is_arithmetic<T>::value> >
	struct Vector
	{
        explicit Vector(T val) { for (int i = 0; i < Size; ++i) e[i] = val; }
        template<typename ...Args>
        Vector(Args ...args) : e{args...} {}

		// Access operator, also case for passing by const ref
        T& operator[](size_t i) { return e[i]; }
        const T& operator[](size_t i) const { return e[i]; }

		T e[Size];
	};

    template<typename T>
    struct Vector<T, 4>
    {
        Vector() = default;
        explicit Vector(T val) : x(val), y(val), z(val), w(val) {}
        Vector(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

		// Access operator, also case for passing by const ref
        T& operator[](size_t i) { return e[i]; }
        const T& operator[](size_t i) const { return e[i]; }

        union
        {
            T e[4];
            struct { T x, y, z, w; };
        };
    };

    template<typename T>
    struct Vector<T, 3>
    {
        Vector() = default;
        explicit Vector(T val) : x(val), y(val), z(val) {}
        Vector(T x, T y, T z) : x(x), y(y), z(z) {}

		// Access operator, also case for passing by const ref
        T& operator[](size_t i) { return e[i]; }
        const T& operator[](size_t i) const { return e[i]; }

        union
        {
            T e[3];
            struct { T x, y, z ; };
        };
    };

    template<typename T>
    struct Vector<T, 2>
    {
        Vector() = default;
        explicit Vector(T val) : x(val), y(val) {}
        Vector(T x, T y) : x(x), y(y) {}

		// Access operator, also case for passing by const ref
        T& operator[](size_t i) { return e[i]; }
        const T& operator[](size_t i) const { return e[i]; }

        union
        {
            T e[2];
            struct { T x, y; };
        };
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Vector operations: Dot, Cross, LengthSqr, Length, Normal
    ///////////////////////////////////////////////////////////////////////////////////////////////
	template<typename T, size_t Size>
	inline T Dot(const Vector<T, Size>& v1, const Vector<T, Size>& v2)
	{
        T result = 0;
        for (int i = 0; i < Size; ++i) { result += v1[i] * v2[i]; }
        return result;
	}

    template<typename T, size_t Size, typename = std::enable_if_t<(Size > 2)>>
	inline Vector<T, Size> Cross(const Vector<T, Size>& v1, const Vector<T, Size>& v2)
	{
		return Vector<T, Size>(
            v1[1] * v2[2] - v1[2] * v2[1],
            v1[2] * v2[0] - v1[0] * v2[2],
            v1[0] * v2[1] - v1[1] * v2[0]);
	}

	template<typename T, size_t Size>
	inline T LengthSqr(const Vector<T, Size>& v)
	{
        T result = 0;
        for (int i = 0; i < Size; ++i) { result += v[i] * v[i]; }
		return result;
	}

	template<typename T, size_t Size>
	inline T Length(const Vector<T, Size>& v)
	{
		return sqrt(LengthSqr(v));
	}

    template<typename T, size_t Size>
    inline Vector<T, Size> Normal(const Vector<T, Size>& v)
    {
        Vector<T, Size> result = v / Length(v);
        return result;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Vector overloaded operators: comparison, negation, adding and scaling, print to os
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template<typename T, size_t Size>
    inline bool operator==(const Vector<T, Size>& v1, const Vector<T, Size>& v2)
    {
        bool result = true;
        for (int i = 0; i < Size; ++i) { result &= Helper::IsEqual<T>(v1[i], v2[i]); }
        return result;
    }

    template<size_t Size>
    inline bool operator!=(const Vector<float, Size>& v1, const Vector<float, Size>& v2)
    {
        return !(v1 == v2);
    }

    template<typename T, size_t Size>
    inline Vector<T, Size> operator-(const Vector<T, Size>& v)
    {
        Vector<T, Size> result;
        for (int i = 0; i < Size; ++i) { result[i] = -v[i]; }
        return result;
    }

    template<typename T, size_t Size>
    inline Vector<T, Size> operator+(const Vector<T, Size>& v1, const Vector<T, Size>& v2)
    {
        Vector<T, Size> result;
        for (int i = 0; i < Size; ++i) { result[i] = v1[i] + v2[i]; }
        return result;
    }

    template<typename T, size_t Size>
    inline Vector<T, Size> operator-(const Vector<T, Size>& v1, const Vector<T, Size>& v2)
    {
        return (v1 + (-v2));
    }

    template<typename T, size_t Size>
    inline Vector<T, Size> operator*(const Vector<T, Size>& v, T k)
    {
        Vector<T, Size> result;
        for (int i = 0; i < Size; ++i) { result[i] = v[i] * k; }
        return result;
    }

    template<typename T, size_t Size>
    inline Vector<T, Size> operator/(const Vector<T, Size>& v, T k)
    {
        return (v * (1 / k));
    }

    template<typename T, size_t Size>
    inline Vector<T, Size>& operator+=(Vector<T, Size>& v1, const Vector<T, Size>& v2)
    {
        v1 = v1 + v2;
        return v1;
    }

    template<typename T, size_t Size>
    inline Vector<T, Size>& operator-=(Vector<T, Size>& v1, const Vector<T, Size>& v2)
    {
        v1 = v1 - v2;
        return v1;
    }

    template<typename T, size_t Size>
    inline Vector<T, Size>& operator*=(Vector<T, Size>& v, T k)
    {
        v = v * k;
        return v;
    }

    template<typename T, size_t Size>
    inline Vector<T, Size>& operator/=(Vector<T, Size>& v, T k)
    {
        v = v / k;
        return v;
    }

    template<typename T, size_t Size>
    std::ostream& operator<<(std::ostream& os, const Vector<T, Size> v)
    {
        os << "< ";
        for (int i = 0; i < Size; ++i)
        {
            os << v[i] << " ";
        }
        os << ">";
        return os;
    }

    template<size_t Size>
	inline std::ostream& operator<<(std::ostream& os, const Vector<float, Size> v)
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

}

// @see https://en.cppreference.com/w/cpp/utility/hash
namespace std
{
    template<typename T, size_t Size>
    struct hash<Math::Vector<T, Size>>
    {
        using KeyType = Math::Vector<T, Size>;
        std::size_t operator()(const KeyType& key) const
        {
            size_t h1 = std::hash<KeyType>{}(key[0]);
            for (int i = 1; i < Size; ++i)
            {
                size_t h2 = std::hash<KeyType>{}(key[i]);
                h1 ^= (h2 << 1);
            }

            return h1;
        }
    };
}

using Vec2f = Math::Vector<float, 2>;
using Vec2i = Math::Vector<int, 2>;

using Vec3f = Math::Vector<float, 3>;
using Vec3i = Math::Vector<int, 3>;

