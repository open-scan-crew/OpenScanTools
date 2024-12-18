#include "algo.h"

#include <imgui.h>


void frame(PermanentResources& pRes, Viewport const& viewport_, unsigned frameId, bool isDebugView) {

	Viewport viewports[] = { viewport_ };
	unsigned const numViewports = 1;

	unsigned const numScans = pRes.scans.getNumScans();
	
	ClippingBoxes const clippingBoxes;
	FrameNodesToDraw frameNodesToDraw;
	NodeRequests frameNodeRequests;

	OctreeCullingStats octreeStats = {};

	for (unsigned scanId = 0; scanId < numScans; scanId++) {

		IScan const &scan = pRes.scans.getScan(scanId);

		for (unsigned viewportId = 0; viewportId < numViewports; viewportId++) {

			Viewport const &viewport = viewports[viewportId];

			NodesToDraw nodesToDraw;
			NodeRequests nodesToRequest;

			cullOctree(pRes.scans, scanId, viewport.camera,
				clippingBoxes, pRes.nodeCache,
				nodesToDraw, nodesToRequest,
				octreeStats
			);

			frameNodesToDraw.append(viewportId, nodesToDraw);

			frameNodeRequests.append(nodesToRequest);
		}
	}

	SortedDrawCommands sortedDrawCommands = sortNodesToDraw(frameNodesToDraw);

	static GPU::DrawOptions options = {};
	if (!isDebugView) {
		if (ImGui::CollapsingHeader("Draw options", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Checkbox("Show cubes", &options.debugNodeCubes);
			ImGui::Checkbox("Hide points", &options.debugDisablePoints);
		}
	}

	pRes.gpu.commitDrawCommands(pRes.scans, sortedDrawCommands, options);

	pRes.allNodesInFlight.set(frameId, frameNodesToDraw);

	pRes.nodeCache.processRequests(frameNodeRequests, pRes.cpu, pRes.gpu);
	// NOTE(robin)
	// Free the cache memory after the processed requests have been refreshed,
	// this ensure that we will not free the nodes queued for transfer.
	// But we are missing ongoing transfer and we can make an error by freeing the source of an ongoing transfer
	pRes.nodeCache.freeMemory(pRes.allNodesInFlight, frameNodeRequests, pRes.cpu, pRes.gpu);

	if (!isDebugView) {			// NOTE(nico) pour éviter d'afficher les infos plusieurs fois

		ImGui::Text("Traversed nodes: %d", octreeStats.traversedNodes);
		ImGui::Text("Visible nodes: %d", octreeStats.visibleNodes);
		ImGui::Text("Visible leaves: %d", octreeStats.visibleLeafs);
		ImGui::Spacing();

		ImGui::Text("Node requests: %d", frameNodeRequests._requests.size());
		ImGui::Text("Node draw commands: %d", sortedDrawCommands._commands.size());
		ImGui::Text("Nodes in CPU Mem: %d", pRes.nodeCache._cacheCPU.size());
		ImGui::Text("Nodes in GPU Mem: %d", pRes.nodeCache._cacheGPU.size());
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


static void impl_drawOctree(
	AllScans &scans, ScanId scanId, NodeId nodeId,
	const Position camPos, const float frustumPlanes[6][4], const int frustumMask, const Position octreeCenter, float octreeSize,
	const NodeCache &nodeCache, NodesToDraw& nodesToDraw, NodeRequests& nodesToRequest,
	OctreeCullingStats& stats
);


void cullOctree(
	AllScans &scans, ScanId scanId, const Camera &camera, const ClippingBoxes &clippingBoxes, const NodeCache &nodeCache, NodesToDraw& nodesToDraw, NodeRequests& nodesToRequest,
	OctreeCullingStats &stats
) {
	(void)clippingBoxes;

	const int ALL_PLANES = 0x3F;
	//const NodeId nodeId = 0;
	const NodeId nodeId = scans.getScan(scanId).getRootNodeId();

	// FIXME(nico) gérer une vraie transformation en objectSpace pour la camera
	impl_drawOctree(scans, scanId, nodeId, camera.position, camera.frustumPlanes, ALL_PLANES, Position(0, 0, 0), scans.getScan(scanId).getOctreeSize(),
		nodeCache, nodesToDraw, nodesToRequest,
		stats
	);
	/*
	if visible(node, camera) && visible(node, clippingBoxes)
		if lodSufficient(node, camera)
			if isGPUResident(node, nodeCache)
				nodesToDraw += {node, lod}
			else
				nodesToRequest += node
		else
			for child in node.children
				visibility(child, camera, clippingBoxes, nodeCache,
						   nodesToDraw, nodesToRequest)

	else
		// nothing
	*/
}

static void impl_drawOctree(AllScans &_scans, ScanId scanId, NodeId nodeId, 
	const Position wsCamPos, const float wsFrustumPlanes[6][4], const int frustumMask, const Position osNodeCenter, float nodeSize,
	const NodeCache &nodeCache, NodesToDraw& nodesToDraw, NodeRequests& nodesToRequest,
	OctreeCullingStats &stats
) {
	const IScan &scan = _scans.getScan(scanId);
	//const OctreeNode &octreeNode = scan.getNode(nodeId);

	nllVec3 wsNodeCenter;
	nllVec3Add(wsNodeCenter, osNodeCenter.coord, scan.getOctreeCenter().coord);

	stats.traversedNodes += 1;

	int activePlanes = frustumMask;
	{
		const float extent[3] = { 0.5f*nodeSize, 0.5f*nodeSize, 0.5f*nodeSize };
		if (frustumMask != 0 && !nllIsAABBInFrustum(wsNodeCenter, extent, wsFrustumPlanes, &activePlanes)) {
			return;
		}
	}

	stats.visibleNodes += 1;

	nllVec3 camToCenter;
	nllVec3Sub(camToCenter, wsCamPos.coord, wsNodeCenter);

	float TAN_HALF_FOV = tanf(45.f * PI_ / 180.f);				// FIXME(nico) hardcodé
	float distance = sqrtf(camToCenter[0] * camToCenter[0] + camToCenter[1] * camToCenter[1] + camToCenter[2] * camToCenter[2]);
	float screenSize = nodeSize / (distance * 2 * TAN_HALF_FOV);
	if (screenSize > nodeSize) screenSize = nodeSize;		// RELECTURE(robin) Pourquoi ?

	const UniqueNodeId uNodeId = { scanId, nodeId };

	GPUData nodeGPUData;
	const bool gpuResident = nodeCache.getGPUData(uNodeId, nodeGPUData);

	//

	NodeId childrenId[8];
	bool allChildrenReady = true;
	bool oneChildReady = false;
	for (int i = 0; i < 8; i++) {
		const NodeId childId = childrenId[i] = scan.getNodeChildId(nodeId, i);
		if (childId != 0) {
			const UniqueNodeId uChildId = { scanId, childId };
			const bool childReady = nodeCache.hasGPUData(uChildId);
			allChildrenReady = allChildrenReady && childReady;
			oneChildReady = oneChildReady || childReady;
		}
	}

	//

	const bool isLeaf = scan.isNodeALeaf(nodeId);
	const bool needsSplitting = !isLeaf && (screenSize >= 0.1f);

	const bool drawMe = !needsSplitting || !allChildrenReady;

	if (isLeaf) stats.visibleLeafs += 1;

	if (drawMe) {

		if (gpuResident) {

			NodeDrawCommand command;
			command.uNodeId = uNodeId;
			command.screenSize = screenSize;
			command.center = osNodeCenter;
			command.size = 0.5f*nodeSize;
			command.gpuData = nodeGPUData;
			nodesToDraw.push(command);
		}
		else {

			NodeRequest request = { uNodeId, screenSize, osNodeCenter, nodeSize };
			nodesToRequest.push(request);
		}

		if (needsSplitting) {

			for (int i = 0; i < 8; i++) {
				const NodeId childId = childrenId[i] = scan.getNodeChildId(nodeId, i);
				if (childId != 0) {
					const UniqueNodeId uChildId = { scanId, childId };
					NodeRequest request = { uChildId, screenSize*.5f, osNodeCenter, nodeSize };
					nodesToRequest.push(request);
				}
			}
		}
	}
	else {

		assert(allChildrenReady);

		const float childSize = nodeSize * 0.5f;
		for (int i = 0; i < 8; i++) {

			const float o = childSize * 0.5f;
			// NOTE(robin) offset NestedOctree
			const nllVec3 offset = { ((i & 4) ? o : -o), ((i & 2) ? o : -o), ((i & 1) ? o : -o) };
			const Position childCenter(osNodeCenter.coord[0] + offset[0], osNodeCenter.coord[1] + offset[1], osNodeCenter.coord[2] + offset[2]);
			const NodeId childId = childrenId[i];
			// NOTE(robin) this test works if childId is a nullptr
			if (childId != 0) {

				impl_drawOctree(_scans, scanId, childId, wsCamPos, wsFrustumPlanes, activePlanes, childCenter, childSize, nodeCache, nodesToDraw, nodesToRequest, stats);
			}
		}
	}
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void FrameNodesToDraw::append(ViewportId viewportId, const NodesToDraw& nodeSet) {

	const SortKey key(viewportId);

	std::vector<NodeDrawCommand> &commands = _commands[key];

	commands.insert(commands.end(), nodeSet._commands.begin(), nodeSet._commands.end());
}

void NodesToDraw::push(NodeDrawCommand const& command) {

	_commands.push_back(command);
}

void NodeRequests::append(const NodeRequests& nodeSet) {

	_requests.insert(_requests.end(), nodeSet._requests.begin(), nodeSet._requests.end());
}

void NodeRequests::push(NodeRequest const& uNodeId) {

	_requests.push_back(uNodeId);
}

void AllNodesInFlight::set(unsigned frameId, const FrameNodesToDraw& nodesToDraw) {

	frameNodes[frameId] = nodesToDraw;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void NodeCache::freeMemory(AllNodesInFlight &allNodesInFlight, const NodeRequests &frameNodeRequests, CPU &cpu, GPU &gpu) {

	if (cpu.isOutOfMemory())
		freeCPUMemory(allNodesInFlight, frameNodeRequests, cpu);

	if (gpu.isOutOfMemory())
		freeGPUMemory(allNodesInFlight, frameNodeRequests, gpu);
}
/*
	if not cache_needsToFreeMemory(nodeCache, allNodesInFlight):
		return

	unusedNodesInCache = allNodes(nodeCache) - allNodesInFlight
	nodesToEvict = cache_calcNodesToEvict(unusedNodesInCache, frameNodeRequests)

	nodeCache = cache_releaseNodes(nodeCache, nodesToEvict)
	return nodeCache
	*/
void NodeCache::freeCPUMemory(AllNodesInFlight &allNodesInFlight, const NodeRequests &frameNodeRequests, CPU &cpu) {

	std::map<UniqueNodeId, Allocation*> unusedNodesInCache = _cacheCPU;
	for (unsigned frame = 0; frame < 3; frame++) {
		for (auto commands : allNodesInFlight.frameNodes[frame]._commands) {
			for (unsigned i = 0; i < commands.second.size(); i++) {
				// NOTE(robin) why not use: unusedNodesInCache.erase(commands.second[i].uNodeId);
				auto search = unusedNodesInCache.find(commands.second[i].uNodeId);
				if (search != unusedNodesInCache.end()) {
					unusedNodesInCache.erase(search);
				}
			}
		}
	}
	for (auto requests : frameNodeRequests._requests) {
		auto search = unusedNodesInCache.find(requests.uNodeId);
		if (search != unusedNodesInCache.end()) {
			unusedNodesInCache.erase(search);
		}
	}

	for (auto unusedNode : unusedNodesInCache) {
		cpu.freeAllocation(unusedNode.second);
		_cacheCPU.erase(unusedNode.first);
	}

}

void NodeCache::freeGPUMemory(AllNodesInFlight &allNodesInFlight, const NodeRequests &frameNodeRequests, GPU &gpu) {

	std::map<UniqueNodeId, GPUData> unusedNodesInCache = _cacheGPU;
	for (unsigned frame = 0; frame < 3; frame++) {
		for (auto commands : allNodesInFlight.frameNodes[frame]._commands) {
			for (unsigned i = 0; i < commands.second.size(); i++) {
				auto search = unusedNodesInCache.find(commands.second[i].uNodeId);
				if (search != unusedNodesInCache.end()) {
					unusedNodesInCache.erase(search);
				}
			}
		}
	}
	for (auto requests : frameNodeRequests._requests) {
		auto search = unusedNodesInCache.find(requests.uNodeId);
		if (search != unusedNodesInCache.end()) {
			unusedNodesInCache.erase(search);
		}
	}
}


void NodeRequests::selectBestNodes(size_t count) {

	// NOTE(nico) on sélection les 'count' requests qui ont les plus grands 'screenSize'

	struct comp {

		bool operator()(NodeRequest const& lhs, NodeRequest const& rhs) const {

			if (lhs.screenSize > rhs.screenSize) return true;
			else {
				if (lhs.screenSize == rhs.screenSize) {
					return (lhs.uNodeId < rhs.uNodeId);
				}
				else return false;
			}
		}
	};

	std::sort(_requests.begin(), _requests.end(), comp());
	_requests.resize(std::min(count, _requests.size()));

}

void NodeCache::processRequests(const NodeRequests& nodeRequests, CPU& cpu, GPU& gpu) {

	if (nodeRequests._requests.empty()) return;

	cpu.processRequests(nodeRequests, *this);
	gpu.processRequests(nodeRequests, *this);

	/*
	nodeCache = cache_processNodeCPUResidentEvents(nodeCache)
	nodeCache = cache_processNodeGPUResidentEvents(nodeCache)

	nodeCandidates = frameNodeRequests - nodesBeingProcessed(nodeCache)
	nodesToAdd = cache_selectNodesToAdd(nodeCache, nodeCandidates)

	nodeCache = cache_startCPUTransfer(nodeCache, nodesToAdd)

	return nodeCache


def cache_processNodeCPUResidentEvents(nodeCache):

	nodesToSendToGPU = nodesCPUReady(nodeCache)

	nodeCache = cache_startGPUTransfer(nodeCache, nodesToSendToGPU)

	return nodeCache
	*/
}

bool NodeCache::hasGPUData(UniqueNodeId uNodeId) const {

	auto end = _cacheGPU.end();
	auto pos = _cacheGPU.find(uNodeId);
	if (pos != end) {

		return true;
	}
	return false;
}

bool NodeCache::getGPUData(UniqueNodeId uNodeId, GPUData &out_gpuData) const {

	auto end = _cacheGPU.end();
	auto pos = _cacheGPU.find(uNodeId);
	if (pos != end) {

		out_gpuData = pos->second;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

SortedDrawCommands sortNodesToDraw(const FrameNodesToDraw &nodesToDraw) {

	SortedDrawCommands result;

	for (auto pair_key_value : nodesToDraw._commands) {

		FrameNodesToDraw::SortKey const& sortKey = pair_key_value.first;
		const ViewportId viewportId = sortKey;

		auto commands = pair_key_value.second;

		for (auto nodeCommand : commands) {

			result._commands.push_back(nodeCommand);
		}
	}

	return result;
}

/*

frameNodeRequests = None
frameNodesToDraw = allNodesToDraw[frameId]

for scan in scans:
  for view in viewports:

	nodesToDraw = none
	nodesToRequest = none

	visibility(scan.octree.rootNode, view.camera, clippingBoxes, nodeCache,
			 nodesToDraw, nodesToRequest)

	frameNodesToDraw = merge(frameNodesToDraw, scan, view, nodesToDraw)

	frameNodeRequests = merge(frameNodeRequests, scan, nodesToRequest)

sortedDrawCommands = processNodesToDraw(frameNodesToDraw)

commitVkDrawCommands(sortedDrawCommands)

allNodesInFlight[frameId] = frameNodesToDraw
nodeCache = cache_freeMemory(nodeCache, allNodesInFlight, frameNodeRequests)

nodeCache = cache_processRequests(nodeCache, frameNodeRequests)

*/
