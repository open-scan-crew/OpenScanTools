#ifndef BASIC_FUNCTIONS_H_
#define BASIC_FUNCTIONS_H_

#include <array>

struct HashVec3 {
	std::hash<float> hasher;
	size_t operator() (const std::array<float, 3>& key) const {
		size_t h = 0;
		for (size_t i = 0; i < 3; ++i)
			h = h * 31 + hasher(key.at(i));

		return h;
	}
};

namespace tls {
    namespace math {
        // get the value rounded up to a power of 2
        // maximum retrun => 2^32
        size_t getCeilPowTwo(size_t minSize);
    }
}

#endif