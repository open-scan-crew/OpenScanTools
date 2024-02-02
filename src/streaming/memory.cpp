#include "memory.h"

Allocator::~Allocator()
{
	for (unsigned i = 0; i < m_blocks.size(); i++) {
		delete[] m_blocks[i].m_pageOccupied;
		delete[] m_blocks[i].m_memory;
	}

	auto it = m_allocations.begin();
	for (auto it = m_allocations.begin(); it != m_allocations.end(); ++it) {
		delete *it;
	}
}

bool Allocator::allocateMemory(Allocation* pAlloc, uint32_t size)
{
	for (int i = 0; i < m_blocks.size(); i++) {
		if (m_blocks[i].allocate(pAlloc, size)) {
			// reference the new allocation in the allocator
			m_allocations.insert(pAlloc);
			pAlloc->blockNumber = i;
			return true;
		}
	}

	// No space has been found in the existing block, we try to create a new one
	if (m_blocks.size() < m_maxBlockCount) {
		MemoryBlock_Page block(m_blockSize / m_pageSize, m_pageSize);
		m_blocks.push_back(block);

		if (m_blocks.back().allocate(pAlloc, size)) {
			m_allocations.insert(pAlloc);
			pAlloc->blockNumber = m_blocks.size() - 1;
			return true;
		}
	}

	return false;
}

bool Allocator::freeAllocation(Allocation* pAlloc)
{
	// TODO(robin) free the memory in the right block
	m_blocks[pAlloc->blockNumber].freeAllocation(pAlloc);

	m_allocations.erase(pAlloc);
	delete pAlloc;
	return false;
}


bool MemoryBlock_Page::allocate(Allocation* pAlloc, uint32_t size) {

	const uint32_t pagesNeeded = (size - 1) / m_pageSize + 1;

	for (uint32_t i = m_firstFreePage; i < m_pageCount - pagesNeeded; i++) {
		// Test if there enough free pages at this location
		for (uint32_t j = 0; j < pagesNeeded; j++) {
			if (m_pageOccupied[i + j]) {
				i += j;
				goto endFor;
			}
		}

		occupyPages(i, pagesNeeded);

		pAlloc->pMappedData = m_memory + m_pageSize *i;
		pAlloc->size = size; // or m_pageSize * pagesNeeed ?

		return true;

		endFor:;
	}

	return false;
}

void MemoryBlock_Page::freeAllocation(Allocation* pAlloc)
{
	uint32_t pageCount = (pAlloc->size - 1) / m_pageSize + 1;
	uint32_t firstPage = ((char*)pAlloc->pMappedData - m_memory) / m_pageSize;

	memset(m_pageOccupied + firstPage, 0, pageCount);
	if (firstPage < m_firstFreePage) m_firstFreePage = firstPage;
}

// NOTE(robin) inline function ?
void MemoryBlock_Page::occupyPages(uint32_t firstPage, uint32_t pageCount)
{
	memset(m_pageOccupied + firstPage, 1, pageCount);
	if (firstPage == m_firstFreePage) searchNextFreePage();
}

// NOTE(robin) inline function ?
void MemoryBlock_Page::searchNextFreePage()
{
	for (m_firstFreePage += 1; m_firstFreePage < m_pageCount; m_firstFreePage++)
	{
		if (!m_pageOccupied[m_firstFreePage]) return;
	}
}