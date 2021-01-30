#include "Helper.h"

#include <cmath>

namespace Helper
{
	bool CompareFloat(float f1, float f2)
	{
		static constexpr float epsilon = 1.0e-05f;
		if (std::abs(f1 - f2) <= epsilon)
			return true;
		return std::abs(f1 - f2) <= epsilon * std::fmax(std::abs(f1), std::abs(f2));
	}
}