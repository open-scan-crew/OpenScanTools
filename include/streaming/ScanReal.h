#pragma once

#include <vector>
#include <iostream>
#include <assert.h>
#include "algo_helpers.h"
#include "Octree_real.h"
#include "StreamScan.h"

// TODO(robin) write the API for reading a NestedOctree from a path

// typedef ExploitNode* NodeId; // already defined in StreamScan.h
struct OctreeNode;

struct ScanReal : public IScan {

	ScanReal(const std::string& path) : m_path(path), color(172, 136, 196) {
		std::ifstream inFile;
		inFile.open(path, std::ios::in | std::ios::binary);
		if (inFile.fail()) {
			std::cout << "An error occured while opening the scan file: " << path.c_str() << std::endl;
			octree = nullptr;
			return;
		}
		else {
			octree = new NestedOctree(inFile, false, m_encodingSize, m_pointsOffsetInFile, m_instancesOffsetInFile);
			if (octree == nullptr) {
				std::cout << "Failed to create the octree from file." << std::endl;
			}
			inFile.close();
		}
	};

	// NOTE(robin) pointer or not for the octree ?
	NestedOctree* octree;
	const std::string m_path;
	// mat4 transfo;
	unsigned m_encodingSize;  // octree.m_sizePoint
	uint64_t m_pointsOffsetInFile;
	uint64_t m_instancesOffsetInFile;

	Color color;

	NodeId const getRootNodeId() const override {
		return octree->getRootNode();
	}

	NodeId const getNodeChildId(NodeId nodeId, unsigned _child) const override {
		// check for _child < 8 inside getChildren()
		return nodeId->getChildren(_child);
	}

	bool isNodeALeaf(NodeId nodeId) const override {
		return nodeId->isLeaf();
	}

	Position getOctreeCenter() const override {
		Position center;
		octree->getCenter(center.coord[0], center.coord[1], center.coord[2]);
		return center;
	}

	float getOctreeSize() const override { return octree->getSize(); }
	Color getColor() const { return color; }

	uint32_t getSizeofPoints(NodeId nodeId) const override {

		return nodeId->getPointCount() * m_encodingSize;
	}

	void readPoints(NodeId nodeId, void *buffer, uint32_t bufferSize) {
		std::ifstream inFile;
		inFile.open(m_path, std::ios::in | std::ios::binary);
		if (inFile.fail()) {
			std::cout << "An error occured while opening the scan file." << std::endl;
			return;
		}
		else {
			uint64_t srcByteOffset = m_pointsOffsetInFile + nodeId->getPointIndex() * m_encodingSize;
			inFile.seekg(srcByteOffset);

			uint32_t sizeofSrc = nodeId->getPointCount() * m_encodingSize;

			assert(bufferSize >= sizeofSrc);
			inFile.read((char*)buffer, sizeofSrc);

			inFile.close();
		}
	}
};
