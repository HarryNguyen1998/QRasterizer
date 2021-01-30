#include "Helper.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"

#include <cassert>

// Note: I didn't implemente equality operator because i didn't want to deal with template
// specialization. Not yet anyway.
// Helper function to test 2 Vec2f. Return true if they are equal, else false.
static bool CompareVec2f(const Vec2f& v1, const Vec2f& v2)
{
	bool result = true;
	result &= Helper::CompareFloat(v1.x, v2.x);
	result &= Helper::CompareFloat(v1.y, v2.y);

	return result;
}

// Helper function to test 2 Vec3f. Return true if they are equal, else false.
static bool CompareVec3f(const Vec3f& v1, const Vec3f& v2)
{
	bool result = true;
	result &= Helper::CompareFloat(v1.x, v2.x);
	result &= Helper::CompareFloat(v1.y, v2.y);
	result &= Helper::CompareFloat(v1.z, v2.z);

	return result;
}

void TestVec2f()
{
	Vec2f v1;
	Vec2f v2{ 1.0f };
	assert(CompareVec2f(v1, Vec2f{ 0.0f }));
	assert(CompareVec2f(v2, Vec2f{ 1.0f }));

	// Access operator
	Vec2f v3{ -6.9f, 3.1f };
	assert(Helper::CompareFloat(v3.x, v3[0]));
	assert(Helper::CompareFloat(v3.y, v3[1]));

	// Vector adding and scaling
	v3 += v2;
	assert(CompareVec2f(v3, Vec2f{ -5.9f, 4.1f }));
	v2 *= 2.0f;
	assert(CompareVec2f(v2, Vec2f{ 2.0f, 2.0f }));
	v3 -= v2;
	assert(CompareVec2f(v3, Vec2f{ -7.9f, 2.1f }));

	// Dot, Length
	assert(Helper::CompareFloat(v3.Dot(v2), -11.6f));
	assert(Helper::CompareFloat(v3.Length(), 8.174350127f));
}

void TestVec3f()
{
	Vec3f v1;
	Vec3f v2{ 1.0f };
	assert(CompareVec3f(v1, Vec3f{ 0.0f }));
	assert(CompareVec3f(v2, Vec3f{ 1.0f, 1.0f, 1.0f }));

	// Access operator
	Vec3f v3{ -6.9f, 3.1f, 42.71828f };
	assert(Helper::CompareFloat(v3.x, v3[0]));
	assert(Helper::CompareFloat(v3.y, v3[1]));
	assert(Helper::CompareFloat(v3.z, v3[2]));

	// Vector adding and scaling
	v3 += v2;
	assert(CompareVec3f(v3, Vec3f{ -5.9f, 4.1f, 43.71828f }));
	v2 *= 2.0f;
	assert(CompareVec3f(v2, Vec3f{ 2.0f }));
	v3 -= v2;
	assert(CompareVec3f(v3, Vec3f{ -7.9f, 2.1f, 41.71828f }));

	// Dot, Length, Cross
	assert(Helper::CompareFloat(v3.Dot(v2), 71.83656f));
	assert(Helper::CompareFloat(v3.Length(), 42.51158532f));
	assert(CompareVec3f(v3.Cross(v2), Vec3f{ -79.23656f, 99.23656f, -20.0f }));
}

void TestMat44f()
{
	Mat44f m;
	Mat44f m2{ 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f };
	
	// Compare, access operator
	assert(m == m2);
	m2[1] = 1.0f;
	assert(m != m2);

	// Matrix transpose
	m = Mat44f{ 1.0f, 2.0f, 3.0f, 4.0f,
		5.0f, 6.0f, 7.0f, 8.0f,
		9.0f, 10.0f, 11.0f, 12.0f,
		13.0f, 14.0f, 15.0f, 16.0f, };
	Mat44f mTransposed{ 1.0f, 5.0f, 9.0f, 13.0f,
		2.0f, 6.0f, 10.0f, 14.0f,
		3.0f, 7.0f, 11.0f, 15.0f,
		4.0f, 8.0f, 12.0f, 16.0f, };
	assert(mTransposed == m.Transposed());

	// Matrix-Matrix multiplication
	Mat44f mProduct{ 30.0f, 70.0f, 110.0f, 150.0f,
		70.0f, 174.0f, 278.0f, 382.0f,
		110.0f, 278.0f, 446.0f, 614.0f,
		150.0f, 382.0f, 614.0f, 846.0f, };
	assert(mProduct == (m * mTransposed));

	// Matrix inverse
	m = Mat44f{ 5.0f, 3.0f, 1.0f, 0.0f,
		1.0f, 0.0f, -2.0f, 0.0f,
		1.0f, 2.0f, 5.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f, };
	Mat44f mInverse{ 4.00001f, - 13.00003f, - 6.00001f, 0.00000f,
		-7.00002f, 24.00005f, 11.00002f, 0.00000f,
		2.00000f, -7.00002f, -3.00001f, -0.00000f,
		0.00000f, 0.00000f, 0.00000f, 1.00000f, };
	assert(mInverse == m.Invert());
	m = Mat44f{ 9.0f, 5.0f, 2.0f, 5.0f,
		9.0f, 5.0f, 3.0f, 7.0f,
		6.0f, 5.0f, 4.0f, 8.0f,
		1.0f, 5.0f, 3.0f, 7.0f, };

	Mat44f mInverse2{ 0.00000f, 0.12500f, -0.00000f, -0.12500f,
		0.80000f, -0.95000f, 0.20000f, 0.15000f,
		1.00000f, -2.25000f, 2.00000f, -0.75000f,
		-1.00000f, 1.62500f, -1.00000f, 0.37500f, };
	assert(mInverse2 == m.Inverse());
}

int main()
{
	TestVec2f();
	TestVec3f();
	TestMat44f();
}