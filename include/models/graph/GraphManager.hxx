#ifndef OPENSCANTOOLS_GRAPH_MANAGER_HXX
#define OPENSCANTOOLS_GRAPH_MANAGER_HXX

#include "models/graph/GraphManager.h"

template<typename T>
inline std::unordered_set<SafePtr<T>> GraphManager::getNodesOnFilter(
	std::function<bool(ReadPtr<AGraphNode>&)> graphNodeFilter,
	std::function<bool(ReadPtr<T>&)> objectFilter) const
{
	std::unordered_set<SafePtr<T>> nodes;

	std::unordered_set<SafePtr<AGraphNode>> geoChildren = AGraphNode::getGeometricChildren_rec(m_root);


	for (const SafePtr<AGraphNode>& graphNode : geoChildren)
	{
		{
			ReadPtr<AGraphNode> readGraph = graphNode.cget();
			if (!readGraph || !graphNodeFilter(readGraph))
				continue;
		}

		//Le filtre du graphNode doit suffir pour assurer le cast
		{
			SafePtr<T> node = static_pointer_cast<T>(graphNode);
			ReadPtr<T> readNode = node.cget();
			if (readNode && objectFilter(readNode))
				nodes.insert(node);
		}
	}
	
	return (nodes);
}

#endif //! OPENSCANTOOLS_GRAPH_MANAGER_HXX
