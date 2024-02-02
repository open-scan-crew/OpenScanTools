#ifndef NODE_FUNCTIONS_H_
#define NODE_FUNCTIONS_H_

#include "models/OpenScanToolsModelEssentials.h"

class AGraphNode;

namespace nodeFunctions
{
	double calculateVolume(const SafePtr<AGraphNode>& node);
	bool isMissingFile(const SafePtr<AGraphNode>& node);
}

#endif //!GEOMETRY_GENERATOR_H_
