#ifndef _MEMORYESTIMATION_H_
#define _MEMORYESTIMATION_H_

#include "io/FileUtils.h"

#define GlobalOctreeMemoryFactor (2)
#define E57OctreeMemoryFactor (2.50)
#define TLSOctreeMemoryFactor (1.20)

namespace Utils
{
	namespace MemoryEstimation 
	{
		uint64_t calculateMemoryUsageForReading(const uint64_t& nextScanNbPoints, const FileType& filetype);
		bool checkMemoryUsage(const uint64_t& memoryCost, const bool& withSwap = false);
		bool checkMemoryUsageForReading(const uint64_t& nextScanNbPoints, const FileType& filetype, const bool& withSwap = false);
	};
};
#endif