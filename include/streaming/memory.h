#pragma once

#include <cstdint>
#include <vector>
#include <set>


struct Allocation {

	void* pMappedData; // or char* ?
	uint32_t size;
	// When we free an Allocation we need the block number
	uint32_t blockNumber;
};

// The memory block represent an allocation of memory in the system.
// Then the memory block 
//
// We choose to implement 2 different way to keep trace of memory used: page or continuous
// The 2 methods may behave differently in term of performance or fragmentation

// Total memory allocated in the system: allocCount * allocSize
struct MemoryBlock_Page {
	MemoryBlock_Page(uint32_t pageCount, uint32_t pageSize) : m_pageCount(pageCount), m_pageSize(pageSize), m_pageOccupied(new bool[pageCount]), m_memory(new char[pageCount * pageSize])
	{
		memset(m_pageOccupied, 0, m_pageCount);
	};

	const uint32_t m_pageCount;
	const uint32_t m_pageSize;
	bool* m_pageOccupied;
	char* m_memory;

	bool allocate(Allocation* pAlloc, uint32_t size);
	void freeAllocation(Allocation* pAlloc);

private:
	uint32_t m_firstFreePage = 0;
	void occupyPages(uint32_t firstPage, uint32_t pageCount);
	void searchNextFreePage();

};

// TODO(robin) implement
/*
struct MemoryBlock_Continuous {
	MemoryBlock_Continuous(uint32_t capacity) : _capacity(capacity), _memory(new char[capacity]) { };

	const uint32_t _capacity;
	char* _memory;

	bool allocate(uint32_t size, AllocationInfo& info);

private:
	void searchNextFreeSpace();

};
*/

// For the moment:
// 1 type of memory: system memory
// No defragmentation
// Use a page system to allocate memory
// The Allocator act as one Memory Pool with multitple blocks of the same capacity
// the blocks are created when needed
// the blocks are not sorted by free space remaining
class Allocator {

public:
	Allocator(uint32_t blockSize, uint32_t pageSize, uint32_t maxBlockCount) : m_blockSize(blockSize), m_pageSize(pageSize), m_maxBlockCount(maxBlockCount) {};
	~Allocator();

	bool allocateMemory(Allocation* pAlloc, uint32_t size);
	bool freeAllocation(Allocation* alloc);


private:
	uint32_t m_blockSize;
	uint32_t m_pageSize;
	uint32_t m_maxBlockCount;

	std::vector<MemoryBlock_Page> m_blocks;
	//std::vector<MemoryBlock_Continuous> m_memBlocksContinuous;

	std::set<Allocation*> m_allocations;
};

