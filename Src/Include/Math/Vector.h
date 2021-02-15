#pragma once
#include "Helper.h"

// For sqrt
#include <cmath>

namespace Math
{
	// Vector 2D math class.
	// Note: Only use CompareFloat(), because after computation, some elements
	// may have values like -0.0f
	template<typename T>
	struct Vec2
	{
		Vec2() = default;
		Vec2(T x);
		Vec2(T x, T y);

		// Vector addition and scaling
		Vec2 operator+(const Vec2& v) const;
		Vec2 operator-(const Vec2& v) const;
		Vec2 operator*(T k) const;
		Vec2& operator+=(const Vec2& v);
		Vec2& operator-=(const Vec2& v);
		Vec2& operator*=(T k);

		T Dot(const Vec2<T>& v) const;
		T LengthSqr() const;
		T Length() const;
        Vec2 Normal() const;
        void Normalize();

		// Access operator
		const T& operator[](unsigned int i) const;
		T& operator[](unsigned int i);

		T x, y;
	};

	// Vector 3D math class.
	// Note: Only use CompareFloat(), because after computation, some elements
	// may have values like -0.0f
	template<typename T>
	struct Vec3
	{
		// Ctor
		Vec3() = default;
		Vec3(T x);
		Vec3(T x, T y, T z);

		// Vector addition and scaling
		Vec3 operator+(const Vec3& v) const;
		Vec3 operator-(const Vec3& v) const;
		Vec3 operator*(T k) const;
		Vec3& operator+=(const Vec3& v);
		Vec3& operator-=(const Vec3& v);
		Vec3& operator*=(T k);

		T Dot(const Vec3<T>& v) const;
		Vec3 Cross(const Vec3<T>& v) const;
		T LengthSqr() const;
		T Length() const;
        Vec3 Normal() const;
        void Normalize();

		// Access operator
		const T& operator[](unsigned int i) const;
		T& operator[](unsigned int i);

		T x, y, z;
	};

	template<typename T>
	inline Vec2<T>::Vec2(T x) : x(x), y(x) {}

	template<typename T>
	inline Vec2<T>::Vec2(T x, T y) : x(x), y(y) {}

	template<typename T>
	inline Vec2<T> Vec2<T>::operator+(const Vec2& v) const
	{
		return Vec2<T>{x + v.x, y + v.y};
	}

	template<typename T>
	inline Vec2<T> Vec2<T>::operator-(const Vec2& v) const
	{
		return Vec2<T>{x - v.x, y - v.y};
	}

	template<typename T>
	inline Vec2<T> Vec2<T>::operator*(T k) const
	{
		return Vec2<T>{x * k, y * k};
	}

	template<typename T>
	inline Vec2<T>& Vec2<T>::operator+=(const Vec2& v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}

	template<typename T>
	inline Vec2<T>& Vec2<T>::operator-=(const Vec2& v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	template<typename T>
	inline Vec2<T>& Vec2<T>::operator*=(T k)
	{
		x *= k;
		y *= k;
		return *this;
	}

	template<typename T>
	inline T Vec2<T>::Dot(const Vec2<T>& v) const
	{
		return (x * v.x + y * v.y);
	}

	template<typename T>
	inline T Vec2<T>::LengthSqr() const
	{
		return (x * x + y * y);
	}

	template<typename T>
	inline T Vec2<T>::Length() const
	{
		return sqrt(LengthSqr());
	}

    template<typename T>
    inline Vec2<T> Vec2<T>::Normal() const
    {
        Vec2<T> result = *this *= (1 / Length());
        return result;
    }

    template<typename T>
    inline void Vec2<T>::Normalize()
    {
        *this *= (1 / Length());
    }

	template<typename T>
	inline const T& Vec2<T>::operator[](unsigned int i) const
	{
		return (&x)[i];
	}
	template<typename T>
	inline T& Vec2<T>::operator[](unsigned int i)
	{
		return (&x)[i];
	}

	template<typename T>
	inline Vec3<T>::Vec3(T x) : x(x), y(x), z(x) {}

	template<typename T>
	inline Vec3<T>::Vec3(T x, T y, T z) : x(x), y(y), z(z) {}

	template<typename T>
	inline Vec3<T> Vec3<T>::operator+(const Vec3& v) const
	{
		return Vec3<T>(x + v.x, y + v.y, z + v.z);
	}

	template<typename T>
	inline Vec3<T> Vec3<T>::operator-(const Vec3& v) const
	{
		return Vec3<T>(x - v.x, y - v.y, z - v.z);
	}

	template<typename T>
	inline Vec3<T> Vec3<T>::operator*(T k) const
	{
		return Vec3<T>(x * k, y * k, z * k);
	}

	template<typename T>
	inline Vec3<T>& Vec3<T>::operator+=(const Vec3& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	template<typename T>
	inline Vec3<T>& Vec3<T>::operator-=(const Vec3& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	template<typename T>
	inline Vec3<T>& Vec3<T>::operator*=(T k)
	{
		x *= k;
		y *= k;
		z *= k;
		return *this;
	}

	template<typename T>
	inline T Vec3<T>::Dot(const Vec3& v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	template<typename T>
	inline Vec3<T> Vec3<T>::Cross(const Vec3& v) const
	{
		return Vec3<T>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}

	template<typename T>
	inline T Vec3<T>::LengthSqr() const
	{
		return x * x + y * y + z * z;
	}

	template<typename T>
	inline T Vec3<T>::Length() const
	{
		return sqrt(LengthSqr());
	}

    template<typename T>
    inline Vec3<T> Vec3<T>::Normal() const
    {
        Vec3<T> result = *this * (1 / Length());
        return result;
    }

    template<typename T>
    inline void Vec3<T>::Normalize()
    {
        *this *= (1 / Length());
    }

	template<typename T>
	inline const T& Vec3<T>::operator[](unsigned int i) const
	{
		return (&x)[i];
	}

	template<typename T>
	inline T& Vec3<T>::operator[](unsigned int i)
	{
		return (&x)[i];
	}
}

using Vec2f = Math::Vec2<float>;
using Vec2i = Math::Vec2<int>;

using Vec3f = Math::Vec3<float>;
using Vec3i = Math::Vec3<int>;
