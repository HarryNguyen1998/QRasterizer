#pragma once
#include <cmath>
#include <initializer_list>
#include <iomanip>
#include <iostream>

#include "Math/Vector.h"
#include "Utils/Helper.h"

namespace Math
{
	// @brief A square Matrix math class
    // @note Matrix has an initializer_list ctor, so be careful with {} and () when initializing!
    // Matrix is default-initialized to an identity matrix.
    // If you use list-initialization and don't provide enough arguments, the leftover elements are
    // 0-initialized.
    template<typename T, size_t Dim>
    struct Matrix
    {
        static constexpr size_t Count = Dim * Dim;
        T e[Count];

        constexpr Matrix()
            : e{}
        {
            for (int i = 0; i < Dim; ++i)
            {
                for (int j = 0; j < Dim; ++j)
                {
                    if (i == j)
                        e[i * Dim + j] = 1;
                    else
                        e[i * Dim + j] = 0;
                }
            }
        }

        constexpr Matrix(std::initializer_list<T> li) : e{}
        {
            int i = 0;
            for (const auto& element : li)
                e[i++] = element;
        }

        // Accessors, also case for passing by const ref
        constexpr const T& operator[](size_t i) const { return e[i]; }
        constexpr T& operator[](size_t i) { return e[i]; }
        constexpr const T& operator()(size_t i, size_t j) const { return e[i * Dim + j]; }
        constexpr T& operator()(size_t i, size_t j) { return e[i * Dim + j]; }

    };


    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Lists of available operations:
    // Overloaded operators: ==, !=, <<, * (matrix multiplication, and vector-matrix multiplication
    // for 3D vectors)
    // Useful functions: Transpose, Inverse, translation/scale/rotation/perspective projection matrix

    // @note Comparison operators can only be constexpr if T is integral. This is due to using
    // IsEqual() helper function, which requires std::abs (not constexpr). The same applies to
    // InitRotation(), InitPersp().
    // Convention is right-handed, so vector-matrix multiplication is post-multiplied
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template<typename T, size_t Dim>
    constexpr bool operator==(const Matrix<T, Dim>& m1, const Matrix<T, Dim>& m2)
    {
        bool result = true;
        for (int i = 0; i < Matrix<T, Dim>::Count; ++i)
        {
            result &= Helper::IsEqual(m1[i], m2[i]);
        }

        return result;
    }

    template<typename T, size_t Dim>
    constexpr bool operator !=(const Matrix<T, Dim>& m1, const Matrix<T, Dim>& m2)
    {
        return !(m1 == m2);
    }

    template<typename T, size_t Dim>
	inline std::ostream& operator<<(std::ostream& os, const Matrix<T, Dim>& m)
	{
		std::ios_base::fmtflags oldFlags = os.flags();
		os.precision(5);
		os.setf(std::ios_base::fixed);
		os << "(";
        int j = 0;
		for (int i = 0; i < Dim; ++i)
		{
            for (j = 0; j < Dim; ++j)
            {
                os << std::setw(12) << m(i, j) << " ";
            }
            if (j == Dim - 1) { os << ")\n"; }
            else { os << "\n"; }
		}
		os.flags(oldFlags);
		return os;
	}

    template<typename T, size_t Dim>
	constexpr Matrix<T, Dim> operator*(const Matrix<T, Dim>& m1, const Matrix<T, Dim>& m2)
	{
        Matrix<T, Dim> result{};
        for (int i = 0; i < Dim; ++i)
        {
            for (int j = 0; j < Dim; ++j)
            {
                // @note due to identity matrix so pivots arent 0
                result(i, j) = 0;
                for (int k = 0; k < Dim; ++k)
                {
                    result(i, j) += m1(i, k) * m2(k, j);
                }
            }
        }
		return result;
	}

    template<typename T, size_t Dim>
	constexpr Matrix<T, Dim> Transpose(const Matrix<T, Dim>& m)
	{
        Matrix<T, Dim> result{};
        for (int i = 0; i < Dim; ++i)
        {
            for (int j = 0; j < Dim; ++j)
            {
                result(i, j) = m(j, i);
            }
        }
        return result;
	}

    // @note If matrix can't be inversed (singular matrix), return an identity matrix
    template<typename T, size_t Dim>
	constexpr Matrix<T, Dim> Inverse(const Matrix<T, Dim>& m)
	{
		// Augmented matrix [src|dest]
        Matrix<T, Dim> dest{};
		Matrix<T, Dim> src{m};

		// Forward elimination, p is pivot
        for (int p = 0; p < Dim; ++p)
        {
            T pivotVal = src(p, p);
            int newPivot = p;

            // Mark the coefficient with highest abs value in row pivot
            if (pivotVal < 0) { pivotVal = -pivotVal; }
            for (int k = p + 1; k < Dim; ++k)
            {
                T tmp = src(k, p);
                if (tmp < 0) { tmp = -tmp; }
                if (tmp > pivotVal) 
                {
                    pivotVal = tmp; 
                    newPivot = k;
                }
            }

            if (Helper::IsEqual<T>(pivotVal, 0)) { return Matrix<T, Dim>{}; } // Matrix is singular
            // Exchange row
            if (p != newPivot)
            {
                for (int j = 0; j < Dim; ++j)
                {
                    T tmp = src(p, j);
                    src(p, j) = src(newPivot, j);
                    src(newPivot, j) = tmp;

                    tmp = dest(p, j);
                    dest(p, j) = dest(newPivot, j);
                    dest(newPivot, j) = tmp;
                }
            }

            // Zeroes rows below pivot, so that after the loop, we have a triangular matrix
            for (int k = p + 1; k < Dim; ++k)
            {
                T f = src(k, p) / pivotVal;
                for (int j = 0; j < Dim; ++j)
                {
                    src(k, j) -= f * src(p, j);
                    dest(k, j) -= f * dest(p, j);
                }
            }
        }

        // Backward substitution
        for (int p = Dim - 1; p >= 0; --p)
        {
            T pivotVal = src(p, p);
            if (Helper::IsEqual<T>(pivotVal, 0)) { return Matrix<T, Dim>{}; } // Matrix is singular
            for (int j = 0; j < Dim; ++j)
            {
                src(p, j) /= pivotVal;
                dest(p, j) /= pivotVal;
            }

            for (int k = p - 1; k >= 0; --k)
            {
                T f = src(k, p);
                for (int j = 0; j < Dim; ++j)
                {
                    src(k, j) -= f * src(p, j);
                    dest(k, j) -= f * dest(p, j);
                }
            }
        }
        
		return dest;
	}

    template<typename T, size_t Dim>
    constexpr Vector<T, Dim> MultiplyVecMat(const Vector<T, Dim>& src, const Matrix<T, Dim>& m)
    {
        Vector<T, Dim> result{};
        for (int j = 0; j < Dim; ++j)
        {
            for (int i = 0; i < Dim; ++i)
            {
                result[j] += src[i] * m(i, j);
            }
        }

        return result;
    }

    template<typename T>
	constexpr Vector<T, 3> MultiplyVecMat(const Vector<T, 3>& src, const Matrix<T, 4>& m)
	{
        Vector<T, 3> result;
		result.x = src.x * m[0] + src.y * m[4] + src.z * m[8] + m[12];
		result.y = src.x * m[1] + src.y * m[5] + src.z * m[9] + m[13];
		result.z = src.x * m[2] + src.y * m[6] + src.z * m[10] + m[14];
        return result;
	}

    // @todo Do I need this?
#if 0
    template<typename T>
	inline Vector<T, 3> MultiplyPtMat(const Vector<T, 3>& src, const Matrix<T, 4>& m)
	{
        Vector<T, 3> result;
		result.x = src.x * m[0] + src.y * m[4] + src.z * m[8] + m[12];
		result.y = src.x * m[1] + src.y * m[5] + src.z * m[9] + m[13];
		result.z = src.x * m[2] + src.y * m[6] + src.z * m[10] + m[14];
		T w = src.x * m[3] + src.y * m[7] + src.z * m[11] + m[15];

		// Any pt is implicitly homogeneoug coords, so we need to convert it back to Cartesian
		// coords.
		result.x /= w;
		result.y /= w;
		result.z /= w;

        return result;
	}
#endif


    constexpr Matrix<float, 4> InitTranslation(float x, float y, float z)
    {
        Matrix<float, 4> result{};
        result(3, 0) = x;
        result(3, 1) = y;
        result(3, 2) = z;
        return result;
    }

    // @brief roll, pitch, yaw are in rad
    inline Matrix<float, 4> InitRotation(float roll, float pitch, float yaw)
    {
        Matrix<float, 4> result{};
        if (!Helper::IsEqual(pitch, 0.0f))
        {
            Matrix<float, 4> rX{};
            rX(1, 1) = cos(pitch);
            rX(2, 1) = -sin(pitch);
            rX(1, 2) = sin(pitch);
            rX(2, 2) = cos(pitch);
            result = result * rX;
        }

        if (!Helper::IsEqual(yaw, 0.0f))
        {
            Matrix<float, 4> rY{};
            rY(2, 2) = cos(yaw);
            rY(0, 2) = -sin(yaw);
            rY(2, 0) = sin(yaw);
            rY(0, 0) = cos(yaw);
            result = result * rY;
        }

        if (!Helper::IsEqual(roll, 0.0f))
        {
            Matrix<float, 4> rZ{};
            rZ(0, 0) = cos(roll);
            rZ(1, 0) = -sin(roll);
            rZ(0, 1) = sin(roll);
            rZ(1, 1) = cos(roll);
            result = result * rZ;
        }

        return result;
    }

    constexpr Matrix<float, 4> InitScale(float fX, float fY, float fZ)
    {
        Matrix<float, 4> result{};
        result(0, 0) = fX;
        result(1, 1) = fY;
        result(2, 2) = fZ;
        return result;
    }

    // @brief TRT^-1, where T transforms center back to origin.
    // @param center The point where the obj is centered around, not the origin.
    inline Vec3f AffineRotation(const Vec3f& input, const Vec3f& center, const Matrix<float, 4>& rotMat)
    {
        Vec3f result = MultiplyVecMat(input, rotMat);
        result[0] += -center.x * rotMat(0, 0) - center.y * rotMat(1, 0) - center.z * rotMat(2, 0) + center.x;
        result[1] += -center.x * rotMat(0, 1) - center.y * rotMat(1, 1) - center.z * rotMat(2, 1) + center.y;
        result[2] += -center.x * rotMat(0, 2) - center.y * rotMat(1, 2) - center.z * rotMat(2, 2) + center.z;

        return result;
    }

    // @note z is in range [0,1]
    inline Matrix<float, 4> InitPersp(float fovY, float aspectRatio, float n, float f)
    {
        Matrix<float, 4> result{};
        result(0, 0) = 1.0f / (tan(fovY / 2) * aspectRatio);
        result(1, 1) = 1.0f / tan(fovY / 2);

        result(2, 2) = -f / (f - n);
        result(2, 3) = -1.0f;
        result(3, 2) = -f * n / (f - n);
        return result;
    }


}

using Mat33i = Math::Matrix<int, 3>;
using Mat33f = Math::Matrix<float, 3>;

using Mat44i = Math::Matrix<int, 4>;
using Mat44f = Math::Matrix<float, 4>;

