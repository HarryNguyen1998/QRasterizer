#include <cassert>

#include "../Include/Utility/Vector.h"

// Helper function to test 2 Vec3f. Return true if they are equal, else false.
static bool CompareVec3f(const Vec3f& v1, const Vec3f& v2)
{
	bool result = true;
	result &= Utility::CompareFloat(v1.x, v2.x);
	result &= Utility::CompareFloat(v1.y, v2.y);
	result &= Utility::CompareFloat(v1.z, v2.z);

	return result;
}

void TestVec3f()
{
	Vec3f v1;
	Vec3f v3{ 0.0f };
	Vec3f v2{ 1.0f };

	assert(CompareVec3f(v1, Vec3f{ 0.0f }));
	assert(CompareVec3f(v2, Vec3f{ 1.0f, 1.0f, 1.0f }));

	v2 *= 2.0f;
	v1 += Vec3f{ 5.0f };
	v1 -= v2;

	assert(CompareVec3f(v2, Vec3f{ 2.0f }));
	assert(CompareVec3f(v1, Vec3f{ 3.0f }));
}

int main()
{
	TestVec3f();
}