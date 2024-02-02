#ifndef _STREAM_GPU_GL_H_
#define _STREAM_GPU_GL_H_

#include <vector>
#include <map>
#include "algo_helpers.h"
#include "StreamCPU.h"
#include "memory.h"

class  AllScans;
struct CPUData;
struct SortedDrawCommands;

struct NodeRequests;
struct NodeCache;
struct UniqueNodeId;
struct CPUData;

struct GPUData {

	std::vector<char> pointsData;
};

struct StreamGPU {

	TransfertState state;
	Allocation* cpuAlloc;
	GPUData gpuData;
};

struct GPU { // RELECTURE(nico) distinguer l'API publique des méthodes privées/internes

	struct DrawOptions {

		bool debugNodeCubes;
		bool debugDisablePoints;
	};

	void commitDrawCommands(AllScans const &scans, const SortedDrawCommands &sortedDrawCommands, DrawOptions const& options);
	GPUData storePoints(Allocation* cpuAlloc);

	void processRequests(const NodeRequests& nodeRequests, NodeCache& nc);
	bool isOutOfMemory();
	bool m_outOfMemory = false;

	void pushTransferts(const NodeRequests& nodeRequests, const NodeCache& nc);
	void startNextTransfert();

	std::map<UniqueNodeId, StreamGPU> _transfertsScheduled;
	std::queue<UniqueNodeId> _queue;
};




#endif // _STREAM_GPU_GL_H_
