/*

NOTE(nico) convention de nommage pour les variables de coordonnÃ©es (diffÃ©rents espaces) :

* wsPosition : world space position
* osPosition : object space position
* esPosition : eye space position
* csPosition : clip space position

*/

// nodeCache // est celui de la frame prÃ©cÃ©dente
// allNodesInFlight[] // contient autant de liste que de frames d'avance prÃ©parÃ©es pour le GPU
// frameId // id de la frame en prÃ©paration, [0-1] pour la gestion d'un back-buffer simple par exemple

// TODO(nico) API NodeReader asynchrone
// TODO(nico) API GPU asynchrone

// TODO(nico) utiliser glm pour les maths ?

// TODO(nico) NodeCache::getPoints ?
// TODO(nico) identifier les nodes pour : points, Ã©tat de chargement !! doit inclure le scanId ??
// TODO(nico) simuler un chargement de points
// TODO(nico) consulter le NodeCache pour les enfants lors du culling
// TODO(nico) freeMemory
// TODO(nico) identifier les nodes pour : enfants
// TODO(nico) gÃ©rer les noderequest
// TODO(nico) gÃ©rer le multiviewport
// TODO(nico) gÃ©rer la clipping box
// TODO(nico) comment associer des donnÃ©es GPU aux scanId (qui Ã©voluent lorsque le projet est modifiÃ©) ?
// TODO(nico) gÃ©rer le toggle de visibilitÃ© d'un scan (+imgui)
// TODO(nico) sÃ©parer le rendu du calcul de streaming, pour afficher plusieurs fois pour nos viewport de debug

// BUG(nico) on devrait d'abord voir apparaitre le cube 'racine' de l'octree avant qu'il se raffine

#pragma once

#include <algorithm>
#include <map>
#include <vector>

#include "StreamScan.h"
#include "algo_helpers.h"
#include "nll_math.h"
#include "StreamGPU_GL.h"
#include "StreamCPU.h"


//#include "../src/Octree_old.h"
#include "memory.h"

#define PI_ 3.1415926535f

//----------------------------------------------------------------------------

struct OctreeCullingStats {

	unsigned visibleNodes;
	unsigned visibleLeafs;
	unsigned traversedNodes;
};


struct Camera {

	Position position;
	float frustumPlanes[6][4];
};

struct Viewport {
    Camera camera;
};

typedef unsigned ViewportId;

struct ClippingBoxes {
};

struct NodeDrawCommand {

	UniqueNodeId uNodeId;
	float screenSize;
	Position center;
	float size;
	GPUData gpuData;
};

struct NodesToDraw {

    void push(NodeDrawCommand const& command);

    std::vector<NodeDrawCommand> _commands;
};

struct FrameNodesToDraw {

    void append(ViewportId viewportId, const NodesToDraw& nodeSet);

    typedef ViewportId SortKey;
    std::map<SortKey, std::vector<NodeDrawCommand> > _commands;
};

struct NodeRequest {

	UniqueNodeId uNodeId;
	float screenSize;
	Position center;
	float size;
};

struct NodeRequests {

	void append(const NodeRequests& nodeSet);
	void push(NodeRequest const& request);
	void selectBestNodes(size_t count);

	std::vector<NodeRequest> _requests;
};

struct AllNodesInFlight {

    void set(unsigned frameId, const FrameNodesToDraw& nodesToDraw);

	FrameNodesToDraw frameNodes[3];
};

struct SortedDrawCommands {

	std::vector<NodeDrawCommand> _commands;
};

//----- Responsibilities -------
//
// * references all the nodes cached in the memory
//   -> 2 type of memory : CPU and GPU -> 2 responsibilities ?
// * allow to test if a node is present in memory (2 types again)
// * allow to test if there is enough free memory, if not, free unused nodes
// * refresh the presence of nodes in the cache based on the requests results
struct NodeCache {

	//----- Memory ---------//
    void freeMemory(AllNodesInFlight &allNodesInFlight, const NodeRequests &frameNodeRequests, CPU &cpu, GPU &gpu);

private:
	void freeCPUMemory(AllNodesInFlight &allNodesInFlight, const NodeRequests &frameNodeRequests, CPU &cpu);
	void freeGPUMemory(AllNodesInFlight &allNodesInFlight, const NodeRequests &frameNodeRequests, GPU &gpu);

public:
	//----- Transfert ------//
	void processRequests(const NodeRequests& nodeRequests, CPU &cpu, GPU &gpu);


	//----- Cache ----------//
	bool hasGPUData(UniqueNodeId uNodeId) const;
	bool getGPUData(UniqueNodeId uNodeId, GPUData &data) const;

	std::map<UniqueNodeId, Allocation*> _cacheCPU;
	std::map<UniqueNodeId, GPUData> _cacheGPU;
};

struct PermanentResources{

	AllScans scans;
	NodeCache nodeCache;
	AllNodesInFlight allNodesInFlight;
	CPU& cpu;
	GPU gpu;
};

//----------------------------------------------------------------------------

void cullOctree(AllScans &scans, ScanId scanId, const Camera &camera, const ClippingBoxes &clippingBoxes, const NodeCache &nodeCache, NodesToDraw& nodesToDraw, NodeRequests& nodesToRequest, OctreeCullingStats &stats);

SortedDrawCommands sortNodesToDraw(const FrameNodesToDraw &nodesToDraw);

void frame(PermanentResources& pRes, Viewport const& viewport_, unsigned frameId, bool isDebugView);
