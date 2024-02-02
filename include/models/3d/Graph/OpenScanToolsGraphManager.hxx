#ifndef OPENSCANTOOLS_GRAPH_MANAGER_HXX
#define OPENSCANTOOLS_GRAPH_MANAGER_HXX

#include "models/3d/Graph/OpenScanToolsGraphManager.h"

template<class NodeClass>
inline SafePtr<NodeClass> OpenScanToolsGraphManager::createCopyNode(const NodeClass& nodeClass)
{
	SafePtr<NodeClass> newNode = make_safe<NodeClass>(nodeClass);
	//AGraphNode::duplicateLink(newNode, nodeClass);
	return newNode;
}

template<class MeasureClass>
inline SafePtr<MeasureClass> OpenScanToolsGraphManager::createMeasureNode()
{
	SafePtr<MeasureClass> measureNode = make_safe<MeasureClass>();
	return measureNode;
}

template<typename T>
inline std::unordered_set<SafePtr<T>> OpenScanToolsGraphManager::getNodesOnFilter(std::function<bool(ReadPtr<AGraphNode>&)> graphNodeFilter, std::function<bool(ReadPtr<T>&)> objectFilter) const
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
