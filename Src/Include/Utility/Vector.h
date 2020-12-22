#include <cmath> 
namespace Utility
{

	// Helper function to compare 2 floats, adapted from embeddeduse.com/2019/08/26/qt-compare-two-floats/
	// Return true if they are equal, else false.
	bool CompareFloat(float f1, float f2)
	{
		static constexpr float epsilon = 1.0e-05f;
		if (std::abs(f1 - f2) <= epsilon)
			return true;
		return std::abs(f1 - f2) <= epsilon * std::fmax(std::abs(f1), std::abs(f2));
	}

	template<typename T>
	struct Vec3
	{
		// Ctor
		Vec3();
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

		// Access operator
		const T& operator[](unsigned int i) const;
		T& operator[](unsigned int i);

		T x, y, z;
	};

	template<typename T>
	Vec3<T>::Vec3() : x(0), y(0), z(0) {}

	template<typename T>
	Vec3<T>::Vec3(T x) : x(x), y(x), z(x) {}

	template<typename T>
	Vec3<T>::Vec3(T x, T y, T z) : x(x), y(y), z(z) {}

	template<typename T>
	Vec3<T> Vec3<T>::operator+(const Vec3& v) const
	{
		return Vec3<T>(x + v.x, y + v.y, z + v.z);
	}

	template<typename T>
	Vec3<T> Vec3<T>::operator-(const Vec3& v) const
	{
		return Vec3<T>(x - v.x, y - v.y, z - v.z);
	}

	template<typename T>
	Vec3<T> Vec3<T>::operator*(T k) const
	{
		return Vec3<T>(x * k, y * k, z * k);
	}

	template<typename T>
	Vec3<T>& Vec3<T>::operator+=(const Vec3& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	template<typename T>
	Vec3<T>& Vec3<T>::operator-=(const Vec3& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	template<typename T>
	Vec3<T>& Vec3<T>::operator*=(T k)
	{
		x *= k;
		y *= k;
		z *= k;
		return *this;
	}

	template<typename T>
	T Vec3<T>::Dot(const Vec3& v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	template<typename T>
	Vec3<T> Vec3<T>::Cross(const Vec3& v) const
	{
		return Vec3<T>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}

	template<typename T>
	T Vec3<T>::LengthSqr() const
	{
		return x * x + y * y + z * z;
	}

	template<typename T>
	T Vec3<T>::Length() const
	{
		return sqrt(LengthSqr());
	}

	template<typename T>
	const T& Vec3<T>::operator[](unsigned int i) const
	{
		return (&x)[i];
	}

	template<typename T>
	T& Vec3<T>::operator[](unsigned int i)
	{
		return (&x)[i];
	}
}

using Vec3f = Utility::Vec3<float>;
using Vec3i = Utility::Vec3<int>;
