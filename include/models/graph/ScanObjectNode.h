#ifndef SCAN_OBJECT_NODE_H
#define SCAN_OBJECT_NODE_H

#include "models/graph/APointCloudNode.h"
#include "models/graph/AClippingNode.h"
#include "tls_def.h"
#include "models/data/Clipping/ClippingData.h"

class ScanObjectNode : public APointCloudNode
{
public:
	ScanObjectNode();
	ScanObjectNode(const ScanObjectNode& node);
	~ScanObjectNode();

	ElementType getType() const override;
	TreeType getDefaultTreeType() const override;

	std::unordered_set<Selection> getAcceptableSelections(const ManipulationMode& mode) const;
	std::unordered_set<ManipulationMode> getAcceptableManipulationModes() const;
};

#endif //! SCAN_OBJECT_NODE_H_