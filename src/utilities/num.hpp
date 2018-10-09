#pragma once
#include <algorithm>

namespace num {
	template <class T>
	T clamp(T const &val, T const &lo, T const &hi) {
		return std::min(std::max(val, lo),hi);
	}
}
