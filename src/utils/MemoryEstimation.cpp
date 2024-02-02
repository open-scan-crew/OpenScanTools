#include "utils/MemoryEstimation.h"
#include "utils/System.h"
#include "models/pointCloud/PointXYZIRGB.h"
#include <iostream>

#include <iostream>

uint64_t Utils::MemoryEstimation::calculateMemoryUsageForReading(const uint64_t& nextScanNbPoints, const FileType& filetype)
{
	double estimatedSize((double)nextScanNbPoints * sizeof(PointXYZIRGB)* GlobalOctreeMemoryFactor);
	switch (filetype)
	{
	case FileType::E57:
		estimatedSize *= E57OctreeMemoryFactor;
		break;
	case FileType::TLS:
		estimatedSize *= TLSOctreeMemoryFactor;
		break;
	}
#ifdef _DEBUG
	std::cout << "calculateMemoryUsageForReading: " << (uint64_t)estimatedSize << "\n";
#endif
	return (uint64_t)estimatedSize;
}

bool  Utils::MemoryEstimation::checkMemoryUsage(const uint64_t& memoryCost, const bool& withSwap)
{
	if (withSwap)
		return memoryCost < (Utils::System::getPhysicalMemoryAvailable() + Utils::System::getPagingMemoryAvailable());
	return memoryCost < Utils::System::getPhysicalMemoryAvailable();
}

bool  Utils::MemoryEstimation::checkMemoryUsageForReading(const uint64_t& nextScanNbPoints, const FileType& filetype, const bool& withSwap)
{
	return checkMemoryUsage(calculateMemoryUsageForReading(nextScanNbPoints, filetype), withSwap);
}