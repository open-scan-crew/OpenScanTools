#pragma once

#include "algo_helpers.h"
#include "StreamScan.h"
#include "memory.h"

#include <vector>
#include <map>
#include <queue>

#include <mutex>
#include <condition_variable>
#include <thread>

struct NodeRequest;
struct NodeRequests;
struct NodeCache;

struct StreamCPU {

	TransfertState state;
	UniqueNodeId uNodeId;
	Allocation* pAlloc;
};

struct CPU {

public:
	explicit CPU(AllScans const &scans) : _scans(scans) {

#ifdef _USE_FAKE_OCTREE_
		// Allocator with max 8 blocks of 4096 bytes with a granularity of 16 bytes
		m_allocator = new Allocator(4096u, 16u, 8u);
#else
		// Allocator with max xx blocks of yyMi bytes with a granularity of 4096 bytes
		// NOTE(robin) A node in tlb scan has a size of max 64kb
		unsigned blockSize = 2 * 1024u * 1024u;
		unsigned maxNumBlocks = 4096;
		m_allocator = new Allocator(blockSize, 4096u, maxNumBlocks);
#endif
		// TODO(robin) move thread creation in initTransfer()
		_transfertThread = std::thread(startNextTransfert, std::ref(*this));
	};
	~CPU();

	//void initTransfer();

	void processRequests(const NodeRequests& nodeRequests, NodeCache& cache);
	bool isOutOfMemory();
	void freeAllocation(Allocation* pAlloc);

private:
	void pushTransferts(const NodeRequests& nodeRequests);
	static void startNextTransfert(CPU& cpu);
	void streamPoints(StreamCPU& stream);

	Allocator* m_allocator;
	bool m_outOfMemory = false;

	AllScans const &_scans;
	std::map<UniqueNodeId, StreamCPU> _transfertsScheduled;
	std::queue<UniqueNodeId> _queue;

	std::thread _transfertThread;
	bool stopThread = false;
	std::mutex _mutex;
	std::condition_variable _cond;
};


// TODO(robin) return a CPUData with streamPoints()
// TODO(robin) try to split _transfertsScheduled in one struct by state


// CPU_Streamer
struct CPU_v2 {				// RELECTURE(nico) utilisé ?

public:
	// RELECTURE(nico) de mon expérience, mieux vaut ne pas avoir d'opérations lourdes/système dans les constructeurs/destructeurs
	//   ça finit toujours par poser un problème ; je préfère toujours avoir des méthodes explicites pour lancer ces opérations
	explicit CPU_v2(AllScans const &scans) : _scans(scans) {
		_transfertThread = std::thread(startNextTransfert, std::ref(*this));
	};
	~CPU_v2();

	void processFinishedTransferts(NodeCache& cache);
	void processNewRequests(const NodeRequests& nodeRequests, const NodeCache& cache);

private:
	void pushTransferts(const NodeRequests& nodeRequests);
	static void startNextTransfert(CPU_v2& cpu);
	void streamData(StreamCPU& stream);

	AllScans const &_scans;
	std::map<UniqueNodeId, StreamCPU> _transfertsScheduled;
	std::queue<UniqueNodeId> _queue;

	std::thread _transfertThread;
	bool stopThread = false;
	std::mutex _resultMutex;
	std::mutex _queueMutex;
	std::condition_variable _cond;
};



//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
