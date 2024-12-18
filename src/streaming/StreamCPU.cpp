#include "StreamCPU.h"
#include "algo.h"

#include <iostream>

#define NUM_MAX_REQUESTS_PER_FRAME 4

struct PointSet {			// RELECTURE(nico) inutilisé ?

	unsigned offset;
	unsigned range;
};

CPU::~CPU() {
	stopThread = true;
	_cond.notify_all();
	_transfertThread.join();
}

void CPU::streamPoints(StreamCPU& stream) {

	std::unique_lock<std::mutex> mlock(_mutex);
	Allocation* pAlloc = new Allocation();

	IScan &scan = _scans.getScan(stream.uNodeId.scanId);
	uint32_t sizeofPoints = scan.getSizeofPoints(stream.uNodeId.nodeId);

	bool result = m_allocator->allocateMemory(pAlloc, sizeofPoints);

	if (result == false) {
		stream.state = TFT_OUT_OF_MEMORY;
		return;
	}

	if (pAlloc->pMappedData != nullptr) {
		// RELECTURE(robin) j’aurai plutot imaginé une fonction indépendante du scan
		scan.readPoints(stream.uNodeId.nodeId, pAlloc->pMappedData, sizeofPoints);
	}
	stream.pAlloc = pAlloc;

	stream.state = TFT_FINISHED;

}

void CPU::processRequests(const NodeRequests& nodeRequests, NodeCache& nc) {

	// thread safety
	std::unique_lock<std::mutex> mlock(_mutex);

	std::map<UniqueNodeId, StreamCPU> ongoingTransferts;
	m_outOfMemory = false;
	
	for (auto transfert : _transfertsScheduled) {
		// Update transferts that have finished
		if (transfert.second.state == TFT_FINISHED) {
			nc._cacheCPU[transfert.first] = transfert.second.pAlloc;
		}
		// Keep trace of the transferts ongoing
		else if (transfert.second.state == TFT_STARTED) {
			ongoingTransferts[transfert.first] = transfert.second;
		}
		// When a transfer fail because of memory, we notify the system
		else if (transfert.second.state == TFT_OUT_OF_MEMORY) {
			m_outOfMemory = true;
		}
	}

	_transfertsScheduled = ongoingTransferts;
	// WARNING(robin) Dont forget to empty queue
	_queue = std::queue<UniqueNodeId>();

	NodeRequests cpuRequests;

	// Select nodes that are not in CPU cache or CPU transfert
	for (NodeRequest req : nodeRequests._requests) {
		// Search in CPU cache
		auto searchCache = nc._cacheCPU.find(req.uNodeId);
		if (searchCache != nc._cacheCPU.end()) continue;
		// Search in transferts still running
		auto searchTransfert = _transfertsScheduled.find(req.uNodeId);
		if (searchTransfert != _transfertsScheduled.end()) continue;

		cpuRequests.push(req);
	}

	cpuRequests.selectBestNodes(NUM_MAX_REQUESTS_PER_FRAME);

	pushTransferts(cpuRequests);

	// thread safety
	mlock.unlock();
	_cond.notify_one();
}

bool CPU::isOutOfMemory()
{
	return m_outOfMemory;
}

void CPU::freeAllocation(Allocation* pAlloc)
{
	m_allocator->freeAllocation(pAlloc);
}

void CPU::pushTransferts(const NodeRequests& nodeRequests) {

	for (NodeRequest req : nodeRequests._requests) {

		_transfertsScheduled[req.uNodeId] = { TFT_NOT_STARTED,
										      req.uNodeId,
											  nullptr
											};
		_queue.push(req.uNodeId);
	}
}

void CPU::startNextTransfert(CPU& cpu) {

	std::cout << std::hex << "Transfert thread : 0x" << std::this_thread::get_id() << std::endl;

	while (true) {
		std::unique_lock<std::mutex> mlock(cpu._mutex);
		while (cpu._queue.empty())
		{
			cpu._cond.wait(mlock);
			if (cpu.stopThread) return;
		}

		UniqueNodeId node = cpu._queue.front();
		cpu._queue.pop();
		cpu._transfertsScheduled[node].state = TFT_STARTED;
		mlock.unlock();		//RELECTURE(nico) pourquoi unlocker ici pour relocker dans `streamPoints` ?

		cpu.streamPoints(cpu._transfertsScheduled[node]);
	}
}

//-------------------------------------//
//------------ Version 2 -------------//
//-----------------------------------//

CPU_v2::~CPU_v2() {
	stopThread = true;
	_cond.notify_all();
	_transfertThread.join();
}

void CPU_v2::processFinishedTransferts(NodeCache& cache) {

	std::unique_lock<std::mutex> result_lock(_resultMutex);

	std::map<UniqueNodeId, StreamCPU> ongoingTransferts;

	for (auto transfert : _transfertsScheduled) {
		// Update transferts that have finished
		if (transfert.second.state == TFT_FINISHED) {
			cache._cacheCPU[transfert.first] = transfert.second.pAlloc;
		}
		// Keep trace of the transferts ongoing
		if (transfert.second.state == TFT_STARTED) {
			ongoingTransferts[transfert.first] = transfert.second;
		}

		// NOTE(robin) a transfert can fail. What do we do in this case ?
	}

	_transfertsScheduled = ongoingTransferts;
}

/*
processRequests_Nico() {

	{
		result_lock;
		resultsToProcess = threadResults;
		threadResults.clear();
	}
	for (auto result : resultsToProcess) {
		...
	}
}*/

void CPU_v2::processNewRequests(const NodeRequests& nodeRequests, const NodeCache& cache) {

	std::unique_lock<std::mutex> queue_lock(_queueMutex);

	// Reset queue
	_queue = std::queue<UniqueNodeId>();

	NodeRequests cpuRequests;

	// Select nodes that are not in CPU cache or CPU transfert
	for (NodeRequest req : nodeRequests._requests) {
		// Search in CPU cache
		auto searchCache = cache._cacheCPU.find(req.uNodeId);
		if (searchCache != cache._cacheCPU.end()) continue;
		// Search in transferts still running
		auto searchTransfert = _transfertsScheduled.find(req.uNodeId);
		if (searchTransfert != _transfertsScheduled.end()) continue;

		cpuRequests.push(req);
	}

	cpuRequests.selectBestNodes(cpuRequests._requests.size());

	pushTransferts(cpuRequests);

	// thread safety
	queue_lock.unlock();
	_cond.notify_one();
}

void CPU_v2::pushTransferts(const NodeRequests& nodeRequests) {

	for (NodeRequest req : nodeRequests._requests) {
		_transfertsScheduled[req.uNodeId] = { TFT_NOT_STARTED,
											  req.uNodeId,
											  nullptr
		};
		_queue.push(req.uNodeId);
	}
}

void CPU_v2::startNextTransfert(CPU_v2& cpu) {

	std::cout << std::hex << "Transfert thread : 0x" << std::this_thread::get_id() << std::endl;

	while (true) {

		std::unique_lock<std::mutex> queue_lock(cpu._queueMutex);
		while (cpu._queue.empty())
		{
			cpu._cond.wait(queue_lock);
			if (cpu.stopThread) return;
		}

		UniqueNodeId node = cpu._queue.front();
		cpu._queue.pop();
		queue_lock.unlock();

		// TODO(robin) allocate the memory for the stream
		//Allocation* pAlloc = new Allocation();
		//bool result = m_allocator->allocateMemory(pAlloc, stream.fileDataAdr.dataSize);
		//
		//if (result == false) {
		//	stream.state = TFT_OUT_OF_MEMORY;
		//	return;
		//}

		std::unique_lock<std::mutex> result_lock(cpu._resultMutex);
		cpu._transfertsScheduled[node].state = TFT_STARTED;

		cpu.streamData(cpu._transfertsScheduled[node]);
		result_lock.unlock();
	}
}
/*
startNextTransfert_Nico() {

	while (1) {

		std::unique_lock<std::mutex> mlock(cpu._mutex);
		while (cpu._queue.empty())
		{
			cpu._cond.wait(mlock);
		}

		command = queue.pop();
		mlock.unlock();

		if (command == END_THREAD) return;
		if (command == TRANSFER) {

			result = transfer();

			result_lock;
			threadResults.push_back(result);
		}
	}
}*/

void CPU_v2::streamData(StreamCPU& stream) {

	ScanId scanId = stream.uNodeId.scanId;
	NodeId nodeId = stream.uNodeId.nodeId;
	IScan &scan = _scans.getScan(scanId);

	uint32_t sizeofPoints = scan.getSizeofPoints(nodeId);
	// TODO(robin) open file

	// TODO(robin) replace by the istream.read() function
	if (stream.pAlloc->pMappedData != nullptr) {
		scan.readPoints(nodeId, stream.pAlloc->pMappedData, sizeofPoints);
	}

	stream.state = TFT_FINISHED;
}
