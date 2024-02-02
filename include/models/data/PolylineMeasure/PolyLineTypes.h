#ifndef POLYLINES_TYPES_H
#define POLYLINES_TYPES_H

enum class PolyLineLock { LockZ, LockX, LockY };

struct PolyLineOptions 
{
	bool activeOption = false;
	PolyLineLock currentLock = PolyLineLock::LockZ;

	double decalOrientation = 0.0;
};

#endif