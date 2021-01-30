#pragma once
#include "Helper.h"
#include "Math/Vector.h"

#include <cmath>
#include <iomanip>
#include <iostream>

namespace Math
{
	// Matrix math class for floats. This is an identity matrix when default ctor is used.
	// Note: Only use CompareFloat(), because after computation, some elements
	// may have values like -0.0f
	struct Mat44f
	{
		Mat44f() = default;
		Mat44f(float a, float b, float c, float d,
			float e, float f, float g, float h,
			float i, float j, float k, float l,
			float m, float n, float o, float p);

		const float& operator[](unsigned int i) const;
		float& operator[](unsigned int i);

		bool operator==(const Mat44f& other) const;
		bool operator!=(const Mat44f& other) const;
		friend std::ostream& operator<<(std::ostream& os, const Mat44f& m);

		// Return a transposed copy or transpose the matrix itself.
		Mat44f Transposed() const;
		Mat44f& Transpose();

		Mat44f operator*(const Mat44f& b) const;
		// Helper function to multiply 2 matrices axb=c.
		static void MultiplyMatMat(const Mat44f& a, const Mat44f& b, Mat44f* c);

		// Return an inverse copy. If fails, return an Identity Matrix.
		// TODO: Revamp the Inverse function!!
		Mat44f Inverse();

		// Invert the matrix. If fails, return an Identity Matrix.
		Mat44f& Invert();

		// Difference between a Point and a Vector: a Point can be translated,
		// and it is implicitly homogeneous, so it needs to be converted back
		// to Cartesian coordinates.
		void MultiplyPtMat(const Vec3f& src, Vec3f* dest) const;

		void MultiplyVecMat(const Vec3f& src, Vec3f* dest) const;

		float x[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f };
	};

	inline Mat44f::Mat44f(float a, float b, float c, float d,
		float e, float f, float g, float h,
		float i, float j, float k, float l,
		float m, float n, float o, float p)
		: x{ a,b,c,d, e,f,g,h, i,j,k,l, m,n,o,p } {}

	inline const float& Mat44f::operator[](unsigned int i) const
	{
		return x[i];
	}

	inline float& Mat44f::operator[](unsigned int i)
	{
		return x[i];
	}

	bool Mat44f::operator==(const Mat44f& other) const
	{
		bool result = true;
		for (int i = 0; i < 16; ++i)
		{
			// Put in temp var to debug easier
			float val1 = x[i];
			float val2 = other[i];
			result &= Helper::CompareFloat(val1, val2);
		}

		return result;
	}

	bool Mat44f::operator!=(const Mat44f& other) const
	{
		return !(*this == other);
	}

	std::ostream& operator<<(std::ostream& os, const Mat44f& m)
	{
		std::ios_base::fmtflags oldFlags = os.flags();
		os.precision(5);
		os.setf(std::ios_base::fixed);
		os << "(";
		for (int i = 0; i < 3; ++i)
		{
			int j = 0;
			os << std::setw(12) << m[j++] << " "
				<< std::setw(12) << m[j++] << " "
				<< std::setw(12) << m[j++] << " "
				<< std::setw(12) << m[j] << "\n ";
		}
		os << std::setw(12) << m[12] << " "
			<< std::setw(12) << m[13] << " "
			<< std::setw(12) << m[14] << " "
			<< std::setw(12) << m[15] << ")\n";

		os.flags(oldFlags);
		return os;
	}


	inline Mat44f Mat44f::Transposed() const
	{
		return Mat44f{ x[0], x[4], x[8], x[12],
			x[1], x[5], x[9], x[13],
			x[2], x[6], x[10], x[14],
			x[3], x[7], x[11], x[15] };
	}

	inline Mat44f& Mat44f::Transpose()
	{
		Mat44f tmp{ x[0], x[4], x[8], x[12],
			x[1], x[5], x[9], x[13],
			x[2], x[6], x[10], x[14],
			x[3], x[7], x[11], x[15] };

		*this = tmp;
		return *this;
	}

	Mat44f Mat44f::operator*(const Mat44f& b) const
	{
		Mat44f c;
		MultiplyMatMat(*this, b, &c);
		return c;
	}

	void Mat44f::MultiplyMatMat(const Mat44f& a, const Mat44f& b, Mat44f* c)
	{
		float* __restrict cAt = c->x;
		cAt[0] = a[0] * b[0] + a[1] * b[4] + a[2] * b[8] + a[3] * b[12];
		cAt[1] = a[0] * b[1] + a[1] * b[5] + a[2] * b[9] + a[3] * b[13];
		cAt[2] = a[0] * b[2] + a[1] * b[6] + a[2] * b[10] + a[3] * b[14];
		cAt[3] = a[0] * b[3] + a[1] * b[7] + a[2] * b[11] + a[3] * b[15];

		cAt[4] = a[4] * b[0] + a[5] * b[4] + a[6] * b[8] + a[7] * b[12];
		cAt[5] = a[4] * b[1] + a[5] * b[5] + a[6] * b[9] + a[7] * b[13];
		cAt[6] = a[4] * b[2] + a[5] * b[6] + a[6] * b[10] + a[7] * b[14];
		cAt[7] = a[4] * b[3] + a[5] * b[7] + a[6] * b[11] + a[7] * b[15];

		cAt[8] = a[8] * b[0] + a[9] * b[4] + a[10] * b[8] + a[11] * b[12];
		cAt[9] = a[8] * b[1] + a[9] * b[5] + a[10] * b[9] + a[11] * b[13];
		cAt[10] = a[8] * b[2] + a[9] * b[6] + a[10] * b[10] + a[11] * b[14];
		cAt[11] = a[8] * b[3] + a[9] * b[7] + a[10] * b[11] + a[11] * b[15];

		cAt[12] = a[12] * b[0] + a[13] * b[4] + a[14] * b[8] + a[15] * b[12];
		cAt[13] = a[12] * b[1] + a[13] * b[5] + a[14] * b[9] + a[15] * b[13];
		cAt[14] = a[12] * b[2] + a[13] * b[6] + a[14] * b[10] + a[15] * b[14];
		cAt[15] = a[12] * b[3] + a[13] * b[7] + a[14] * b[11] + a[15] * b[15];
	}

	Mat44f Mat44f::Inverse()
	{
		// Augmented matrix [src|dest]
		Mat44f dest;
		Mat44f src{ *this };

		// Forward elimination
		for (int pivot = 0; pivot < 16; pivot += 5)
		{
			int newPivot = pivot;
			float pivotVal = src[pivot];
			if (pivotVal < 0.0f) { pivotVal = -pivotVal; }

			// For each row below current pivot, if it's abs value is higher than current pivot then
			// mark it
			for (int l = pivot + 4; l < 16; l += 4)
			{
				float tmp = src[l];
				if (tmp < 0.0f) { tmp = -tmp; }
				if (tmp > pivotVal)
				{
					pivotVal = tmp;
					newPivot = l;
				}
			}
			if (Helper::CompareFloat(pivotVal, 0.0f)) { return Mat44f{}; } // Singular matrix

			// Exchange row so that pivot has highest abs value
			if (newPivot != pivot)
			{
				// i is the 1st column of the row which has the pivot, e.g. pivot = C0/C5/C10/C15
				// then i = C0/C4/C8/C12. k is the same as i: 1st col of the row of newPivot.
				int i = (pivot / 4) * 4;
				int k = (newPivot / 4) * 4;
				for (int j = 0; j < 4; ++j)
				{
					float tmp = src[i + j];
					src[i + j] = src[k + j];
					src[k + j] = tmp;

					tmp = dest[i + j];
					dest[i + j] = dest[k + j];
					dest[k + j] = tmp;
				}
			}

			// For each coefficient of same col as pivot Pii (e.g. P11 or C1), denoted Cli
			// (e.g. C21 or C5), for each coefficient in row k Clj (e.g. C20/C21/C22/C32 or
			// C8/C9/C10/C11), we have Clj -= Cli / Pii * Cij (where Cij = C10/C11/C12/C13 or
			// C4/C5/C6/C7)
			for (int l = pivot + 4; l < 16; l += 4)
			{
				float f = src[l] / src[pivot];
				int i = (pivot / 4) * 4;
				int k = (l / 4) * 4;
				for (int j = 0; j < 4; ++j)
				{
					src[k + j] -= f * src[i + j];
					dest[k + j] -= f * dest[i + j];
				}
			}
		}

		// Backward substitution
		for (int pivot = 15; pivot >= 0; pivot -= 5)
		{
			float pivotVal = src[pivot];
			if (Helper::CompareFloat(pivotVal, 0.0f)) { return Mat44f{}; }
			// Scale the pivot to 1
			int i = (pivot / 4) * 4;
			for (int j = 0; j < 4; ++j)
			{
				src[i + j] /= pivotVal;
				dest[i + j] /= pivotVal;
			}

			// coefficients Ckj, Ckj -= Cki * Cij. So, if i=3, k=1, then we have
			// C10/C11/C12/C13 -= C13 * C30/C31/C32/C33 (outward col * each
			// corresponding coefficient same col but row of pivot)
			for (int l = pivot - 4; l >= 0; l -= 4)
			{
				float f = src[l];
				int k = (l / 4) * 4;
				for (int j = 0; j < 4; ++j)
				{
					src[k + j] -= f * src[i + j];
					dest[k + j] -= f * dest[i + j];
				}
			}
		}

		return dest;
	}

	Mat44f& Mat44f::Invert()
	{
		Mat44f tmp = Inverse();
		*this = tmp;
		return *this;
	}


	inline void Mat44f::MultiplyPtMat(const Vec3f& src, Vec3f* dest) const
	{
		// A pt can be translated, while a vector can't.
		dest->x = src.x * x[0] + src.y * x[4] + src.z * x[8] + x[12];
		dest->y = src.x * x[1] + src.y * x[5] + src.z * x[9] + x[13];
		dest->z = src.x * x[2] + src.y * x[6] + src.z * x[10] + x[14];
		float w = src.x * x[3] + src.y * x[7] + src.z * x[11] + x[15];

		// Any pt is implicitly homogeneoug coords, so we need to convert it back
		// to Cartesian coords.
		dest->x *= 1.0f/w;
		dest->y *= 1.0f/w;
		dest->z *= 1.0f/w;
	}

	inline void Mat44f::MultiplyVecMat(const Vec3f& src, Vec3f* dest) const
	{
		dest->x = src.x * x[0] + src.y * x[4] + src.z * x[8] + x[12];
		dest->y = src.x * x[1] + src.y * x[5] + src.z * x[9] + x[13];
		dest->z = src.x * x[2] + src.y * x[6] + src.z * x[10] + x[14];
	}
}

using Mat44f = Math::Mat44f;

