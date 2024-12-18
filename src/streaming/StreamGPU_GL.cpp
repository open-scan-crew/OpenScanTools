#include <GLFW/glfw3.h>
#include "StreamGPU_GL.h"
#include "algo.h"


#define NUM_MAX_REQUESTS_PER_FRAME 16


static void drawCube(const nllVec3 center, float halfSize);

void GPU::commitDrawCommands(AllScans const &scans, const SortedDrawCommands &sortedDrawCommands, DrawOptions const& options) {

	for (auto command : sortedDrawCommands._commands) {

		const ScanId scanId = command.uNodeId.scanId;
		const IScan &scan = scans.getScan(scanId);
		Position octreeCenter = scan.getOctreeCenter();

		// NOTE(robin) we dont use the general color of the scan anymore
		//             but we can use it for color composition

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(octreeCenter.coord[0], octreeCenter.coord[1], octreeCenter.coord[2]);

		if (!options.debugDisablePoints) {

			glPushMatrix();

#ifdef _USE_FAKE_OCTREE_
			unsigned pointStep = 1;
			float const offset[3] = { 0,0,0 };
#else
			unsigned pointStep = 1;
			// NOTE(nico) les points du NestedOctree sont stockés en worldSpace, on les remet en octreeSpace
			glTranslatef(-octreeCenter.coord[0], -octreeCenter.coord[1], -octreeCenter.coord[2]);
#endif

			glEnable(GL_POINT_SIZE);
			glPointSize(2.f);

			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);

			PointColor const* data = (PointColor*)command.gpuData.pointsData.data();
			unsigned const numPoints = command.gpuData.pointsData.size() / sizeof(PointColor);

			glColorPointer(3, GL_UNSIGNED_BYTE, sizeof(*data), (char*)data + offsetof(PointColor, r));
			glVertexPointer(3, GL_FLOAT, sizeof(*data), data);
			glDrawArrays(GL_POINTS, 0, numPoints);

			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_COLOR_ARRAY);

			glPointSize(1.f);
			glDisable(GL_POINT_SIZE);

			glPopMatrix();
		}

		if (options.debugNodeCubes) {

			Color color = scan.getColor();
			glEnable(GL_BLEND);
			glColor4f(color.rgb[0], color.rgb[1], color.rgb[2], 0.1f);

			drawCube(command.center.coord, command.size);

			glDisable(GL_BLEND);
		}

		glPopMatrix();
	}
}

GPUData GPU::storePoints(Allocation* cpuAlloc) {		// FIXME(nico) passer explicitement le nombre de points à afficher, on en a besoin ensuite

	GPUData result;

	unsigned ptCount = cpuAlloc->size / sizeof(PointColor);
	result.pointsData.resize(ptCount * sizeof(PointColor));

	memcpy(result.pointsData.data(), cpuAlloc->pMappedData, cpuAlloc->size);
	return result;
}

void GPU::processRequests(const NodeRequests& nodeRequests, NodeCache& nc) {

	std::map<UniqueNodeId, StreamGPU> ongoingTransferts;

	for (auto transfert : _transfertsScheduled) {
		// Update transfert that have finished
		if (transfert.second.state == TFT_FINISHED) {
			nc._cacheGPU[transfert.first] = transfert.second.gpuData;
		}
		// Keep trace of the transferts ongoing
		if (transfert.second.state == TFT_STARTED) {
			ongoingTransferts[transfert.first] = transfert.second;
		}
		// When a transfer fail because of memory, we notify the system
		else if (transfert.second.state == TFT_OUT_OF_MEMORY) {
			m_outOfMemory = true;
		}
	}

	_transfertsScheduled = ongoingTransferts;
	_queue = std::queue<UniqueNodeId>();

	NodeRequests gpuRequests;

	// Select nodes that are in CPU cache and not in GPU cache/transfert
	for (NodeRequest req : nodeRequests._requests) {
		// Search in CPU cache
		auto searchCPUCache = nc._cacheCPU.find(req.uNodeId);
		if (searchCPUCache == nc._cacheCPU.end()) continue;
		else {
			// Search in GPU cache
			auto searchCache = nc._cacheGPU.find(req.uNodeId);
			if (searchCache != nc._cacheGPU.end()) continue;
			// Search in the transferts still running
			auto searchTransfert = _transfertsScheduled.find(req.uNodeId);
			if (searchTransfert != _transfertsScheduled.end()) continue;

			gpuRequests.push(req);
		}
	}

	gpuRequests.selectBestNodes(gpuRequests._requests.size());

	pushTransferts(gpuRequests, nc);

	// Here launch asynchronous transferts
	startNextTransfert();
}

bool GPU::isOutOfMemory()
{
	return m_outOfMemory;
}

void GPU::pushTransferts(const NodeRequests& nodeRequests, const NodeCache& nc) {

	for (NodeRequest req : nodeRequests._requests) {
		ScanId scanId = req.uNodeId.scanId;
		NodeId nodeId = req.uNodeId.nodeId;

		auto searchCPUCache = nc._cacheCPU.find(req.uNodeId);
		if (searchCPUCache == nc._cacheCPU.end()) continue;
		Allocation* cpuAlloc = searchCPUCache->second;

		_transfertsScheduled[req.uNodeId] = { TFT_NOT_STARTED,
											  cpuAlloc,
											  GPUData()	};
		_queue.push(req.uNodeId);
	}
}

void GPU::startNextTransfert() {

	for (int i = 0; i < NUM_MAX_REQUESTS_PER_FRAME; i++) {

		if (_queue.size() == 0) return;

		UniqueNodeId node = _queue.front();

		_transfertsScheduled[node].state = TFT_STARTED;
		_transfertsScheduled[node].gpuData = storePoints(_transfertsScheduled[node].cpuAlloc);
		_transfertsScheduled[node].state = TFT_FINISHED;

		_queue.pop();
	}
}

static void drawCube(const float center[3], float halfSize) {

	glBegin(GL_LINES);

	// X lines
	glVertex3f(center[0] - halfSize, center[1] - halfSize, center[2] - halfSize);
	glVertex3f(center[0] + halfSize, center[1] - halfSize, center[2] - halfSize);
	glVertex3f(center[0] - halfSize, center[1] + halfSize, center[2] - halfSize);
	glVertex3f(center[0] + halfSize, center[1] + halfSize, center[2] - halfSize);
	glVertex3f(center[0] - halfSize, center[1] - halfSize, center[2] + halfSize);
	glVertex3f(center[0] + halfSize, center[1] - halfSize, center[2] + halfSize);
	glVertex3f(center[0] - halfSize, center[1] + halfSize, center[2] + halfSize);
	glVertex3f(center[0] + halfSize, center[1] + halfSize, center[2] + halfSize);

	// Y lines
	glVertex3f(center[0] - halfSize, center[1] - halfSize, center[2] - halfSize);
	glVertex3f(center[0] - halfSize, center[1] + halfSize, center[2] - halfSize);
	glVertex3f(center[0] + halfSize, center[1] - halfSize, center[2] - halfSize);
	glVertex3f(center[0] + halfSize, center[1] + halfSize, center[2] - halfSize);
	glVertex3f(center[0] - halfSize, center[1] - halfSize, center[2] + halfSize);
	glVertex3f(center[0] - halfSize, center[1] + halfSize, center[2] + halfSize);
	glVertex3f(center[0] + halfSize, center[1] - halfSize, center[2] + halfSize);
	glVertex3f(center[0] + halfSize, center[1] + halfSize, center[2] + halfSize);

	// Z lines
	glVertex3f(center[0] - halfSize, center[1] - halfSize, center[2] - halfSize);
	glVertex3f(center[0] - halfSize, center[1] - halfSize, center[2] + halfSize);
	glVertex3f(center[0] - halfSize, center[1] + halfSize, center[2] - halfSize);
	glVertex3f(center[0] - halfSize, center[1] + halfSize, center[2] + halfSize);
	glVertex3f(center[0] + halfSize, center[1] - halfSize, center[2] - halfSize);
	glVertex3f(center[0] + halfSize, center[1] - halfSize, center[2] + halfSize);
	glVertex3f(center[0] + halfSize, center[1] + halfSize, center[2] - halfSize);
	glVertex3f(center[0] + halfSize, center[1] + halfSize, center[2] + halfSize);

	glEnd();
}
