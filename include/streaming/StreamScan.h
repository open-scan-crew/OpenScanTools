#pragma once

#include "algo_helpers.h"

#ifdef _USE_FAKE_OCTREE_
typedef unsigned NodeId;
#else
#include "Octree_real.h"
typedef ExploitNode* NodeId;
#endif

typedef unsigned ScanId;			// TODO(nico) valider que ce ScanId est bien un 'int'

struct UniqueNodeId {

	bool operator<(UniqueNodeId const& rhs) const {

		if (scanId < rhs.scanId) {
			return true;
		}
		else {
			if (scanId == rhs.scanId) {
				return (nodeId < rhs.nodeId);
			}
			else return false;
		}
	}

	ScanId scanId;
	NodeId nodeId;
};


class IScan {

public:

	virtual NodeId const getRootNodeId() const = 0;
	virtual NodeId const getNodeChildId(NodeId nodeId, unsigned _child) const = 0;
	virtual bool isNodeALeaf(NodeId nodeId) const = 0;

	virtual Position getOctreeCenter() const = 0;
	virtual float getOctreeSize() const = 0;

	virtual Color getColor() const = 0;

	virtual uint32_t getSizeofPoints(NodeId nodeId) const = 0;
	virtual void readPoints(NodeId nodeId, void *buffer, uint32_t bufferSize) = 0;

	// TODO
};


class AllScans {			// NOTE(nico) j'introduis ça pour ne pas trimballer des IScan** dans le code
public:

	AllScans(int numScans_, IScan *scans_[]) : numScans(numScans_), _scanPtrs(scans_) {};

	int getNumScans() const { return numScans; }
	IScan& getScan(ScanId scanId) const { return *_scanPtrs[scanId]; }

private:
	int numScans;
	IScan** _scanPtrs;
};
