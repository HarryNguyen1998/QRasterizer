#pragma once
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <utility>

#include "Math/Vector.h"
#include "Utils/Helper.h"

namespace Math
{
    template<size_t R, size_t C>
    static inline size_t SetElement(size_t pos)
    {
        return pos % R == pos / R;
    }

    template<typename T, size_t Dim, size_t... Seq>
    inline std::array<T, Dim * Dim> InitIdentity(std::index_sequence<Seq...>)
    {
        return std::array<T, Dim * Dim>{{(T)SetElement<Dim, Dim>(Seq)...} };
    }

    struct IdentityTag {};
    static IdentityTag g_identityTag;

	// Square Matrix math class
    template<typename T, size_t Dim>
	struct Matrix
	{

        Matrix() = default;
        Matrix(IdentityTag) : e{InitIdentity<T, Dim>(std::make_index_sequence<Dim * Dim>{})} {}
        Matrix(T val) { for (int i = 0; i < Dim * Dim; ++i) { e[i] = val; } }
        template<typename T_ = T, typename... Ts>
        Matrix(T_ a0, Ts... args) : e{std::forward<T_>(a0), std::forward<Ts>(args)...} {}

        // Accessors
        const T& operator[](size_t i) const { return e[i]; }
        T& operator[](size_t i) { return e[i]; }
        const T& operator()(size_t i, size_t j) const { return e[i * Dim + j]; }
        T& operator()(size_t i, size_t j) { return e[i * Dim + j]; }

		std::array<T, Dim * Dim> e;
	};

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Matrix overloaded operators: comparision, matrix multiplication
    // With float specialization: print to os
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template<typename T, size_t Dim>
	inline bool operator==(const Matrix<T, Dim>& m1, const Matrix<T, Dim>& m2)
	{
		bool result = true;
		for (int i = 0; i < Dim * Dim; ++i) { result &= Helper::IsEqual<T>(m1[i], m2[i]); }
		return result;
	}

    template<typename T, size_t Dim>
	inline bool operator!=(const Matrix<T, Dim>& m1, const Matrix<T, Dim>& m2)
	{
		return !(m1 == m2);
	}

    template<typename T, size_t Dim>
    inline std::ostream& operator<<(std::ostream& os, const Matrix<T, Dim>& m)
    {
        os << "(";
        for (int i = 0; i < Dim; ++i)
        {
            for (int j = 0; j < Dim; ++j)
            {
                os << m(i, j) << " ";
            }
            if (j == Dim - 1) { os << ")\n"; }
            else { os << "\n"; }
        }
    }

    template<size_t Dim>
	inline std::ostream& operator<<(std::ostream& os, const Matrix<float, Dim>& m)
	{
		std::ios_base::fmtflags oldFlags = os.flags();
		os.precision(5);
		os.setf(std::ios_base::fixed);
		os << "(";
		for (int i = 0; i < Dim; ++i)
		{
            for (int j = 0; j < Dim; ++j)
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
	Matrix<T, Dim> operator*(const Matrix<T, Dim>& m1, const Matrix<T, Dim>& m2)
	{
        Matrix<T, Dim> result{};
        for (int i = 0; i < Dim; ++i)
        {
            for (int j = 0; j < Dim; ++j)
            {
                for (int k = 0; k < Dim; ++k)
                {
                    result(i, j) += m1(i, k) * m2(k, j);
                }
            }
        }
		return result;
	}

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Matrix operations: Transpose, Inverse 
    // Specialization for 3D vector and 4x4 matrix: multiply between point/vector and a matrix
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template<typename T, size_t Dim>
	inline Matrix<T, Dim> Transpose(const Matrix<T, Dim> m)
	{
        Matrix<T, Dim> result;
        for (int i = 0; i < Dim; ++i)
        {
            for (int j = 0; j < Dim; ++j)
            {
                result(i, j) = m(j, i);
            }
        }
        return result;
	}

    // If matrix can't be inversed (singular matrix), return a default matrix.
    template<typename T, size_t Dim>
	Matrix<T, Dim> Inverse(const Matrix<T, Dim> m)
	{
		// Augmented matrix [src|dest]
        Matrix<T, Dim> dest{g_identityTag};
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

    template<typename T>
	inline Vector<T, 3> MultiplyPtMat(const Vector<T, 3>& src, const Matrix<T, 4>& m)
	{
        Vector<T, 3> result;
		result.x = src.x * m[0] + src.y * m[4] + src.z * m[8] + m[12];
		result.y = src.x * m[1] + src.y * m[5] + src.z * m[9] + m[13];
		result.z = src.x * m[2] + src.y * m[6] + src.z * m[10] + m[14];
		T w = src.x * m[3] + src.y * m[7] + src.z * m[11] + m[15];

		// Any pt is implicitly homogeneoug coords, so we need to convert it back
		// to Cartesian coords.
		result.x /= w;
		result.y /= w;
		result.z /= w;

        return result;
	}

    template<typename T, size_t Dim>
    inline Vector<T, Dim> MultiplyVecMat(const Vector<T, Dim>& src, const Matrix<T, Dim>& m)
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
	inline Vector<T, 3> MultiplyVecMat(const Vector<T, 3>& src, const Matrix<T, 4>& m)
	{
        Vector<T, 3> result;
		result.x = src.x * m[0] + src.y * m[4] + src.z * m[8] + m[12];
		result.y = src.x * m[1] + src.y * m[5] + src.z * m[9] + m[13];
		result.z = src.x * m[2] + src.y * m[6] + src.z * m[10] + m[14];
        return result;
	}

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Construct 4x4 Matrices for graphic purposes: translation/rotation/scale, pespective projection.
    ///////////////////////////////////////////////////////////////////////////////////////////////
    using Mat44f = Matrix<float, 4>;

    inline Mat44f InitTranslation(float x, float y, float z)
    {
        Mat44f result{g_identityTag};
        result(3, 0) = x;
        result(3, 1) = y;
        result(3, 2) = z;
        return result;
    }

    inline Mat44f InitRotation(float roll, float pitch, float yaw)
    {
        Mat44f rX{g_identityTag};
        rX(1, 1) = cos(roll);
        rX(2, 1) = -sin(roll);
        rX(1, 2) = sin(roll);
        rX(2, 2) = cos(roll);

        Mat44f rY{g_identityTag};
        rY(2, 2) = cos(pitch);
        rY(0, 2) = -sin(pitch);
        rY(2, 0) = sin(pitch);
        rY(0, 0) = cos(pitch);

        Mat44f rZ{g_identityTag};
        rZ(0, 0) = cos(yaw);
        rZ(1, 0) = -sin(yaw);
        rZ(0, 1) = sin(yaw);
        rZ(1, 1) = cos(yaw);

        Mat44f result = rX * rY * rZ;
        return result;
    }

    inline Mat44f InitScale(float fX, float fY, float fZ)
    {
        Mat44f result{};
        result(0, 0) = fX;
        result(1, 1) = fY;
        result(2, 2) = fZ;
        return result;
    }

    inline Mat44f InitPersp(float fovY, float aspectRatio, float n, float f)
    {
        Mat44f result{};
        result(0, 0) = 1.0f / (tan(fovY / 2) * aspectRatio);
        result(1, 1) = 1.0f / tan(fovY / 2);

        result(2, 2) = -(f + n) / (f - n);
        result(2, 3) = -1.0f;
        result(3, 2) = -2.0f * f * n / (f - n);
        return result;
    }

}

using Mat44f = Math::Matrix<float, 4>;

