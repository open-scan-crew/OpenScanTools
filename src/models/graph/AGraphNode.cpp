#include "models/graph/AGraphNode.h"

#include "utils/Logger.h"

IdGiver<uint32_t> AGraphNode::m_graphicIdGiver = IdGiver<uint32_t>(0);

inline bool verifNode(const ReadPtr<AGraphNode>& readNode)
{
	return (readNode && !readNode->isDead());
}

inline bool verifNode(const WritePtr<AGraphNode>& writeNode)
{
	return (writeNode && !writeNode->isDead());
}

inline bool isInSet(const SafePtr<AGraphNode>& isInSetPtr, const std::unordered_set<SafePtr<AGraphNode>>& dataSet)
{
	ReadPtr<AGraphNode> isInSet = isInSetPtr.cget();
	if (!verifNode(isInSet))
		return false;

	return dataSet.find(isInSetPtr) != dataSet.end();
}

std::unordered_set<SafePtr<AGraphNode>> verifNodes(const std::unordered_set<SafePtr<AGraphNode>>& nodes)
{
	std::unordered_set<SafePtr<AGraphNode>> viableNodes;
	for (const SafePtr<AGraphNode>& node : nodes)
	{
		ReadPtr<AGraphNode> rNode = node.cget();
		if (verifNode(rNode))
			viableNodes.insert(node);
	}
	return viableNodes;
}

bool recIsAncestor(const SafePtr<AGraphNode>& isAncestorPtr, const SafePtr<AGraphNode>& currentPtr, std::function<std::unordered_set<SafePtr<AGraphNode>>(const SafePtr<AGraphNode>&)> getterSet, std::unordered_set<SafePtr<AGraphNode>>& visitedNodes, bool useVerifNode = true)
{
	{
		ReadPtr<AGraphNode> current = currentPtr.cget();
		if (useVerifNode && !verifNode(current))
			return false;
	}

	std::unordered_set<SafePtr<AGraphNode>> datas;
	datas = getterSet(currentPtr);

	if (datas.find(isAncestorPtr) != datas.end())
		return true;

	if (visitedNodes.find(currentPtr) != visitedNodes.end())
		return false;

	visitedNodes.insert(currentPtr);

	for (const SafePtr<AGraphNode>& data : datas)
	{
		if (recIsAncestor(isAncestorPtr, data, getterSet, visitedNodes))
			return true;
	}
	return false;
}

AGraphNode::AGraphNode(const AGraphNode& node)
	: TransformationModule(node)
	, Data(node)
{
	m_graphicId = m_graphicIdGiver.giveAutoId();
}

AGraphNode::AGraphNode()
{
	m_graphicId = m_graphicIdGiver.giveAutoId();
}

AGraphNode::~AGraphNode()
{
	// QUESTION(robin) - Pourquoi ne pas remove le child de son parent ?
	//m_geometricParent->removeChild(m_id);

	// QUESTION(robin) - Doit-on détruire les enfants de ce noeud ? Ou bien un autre mécanisme s’en charge ?
	//std::unordered_map<xg::Guid, AGraphNode*> copyChildren = m_geometricChildren;
	//for (auto child : copyChildren)
	//{
	//	delete child.second;
	//}
	//assert(m_geometricChildren.empty());
}

void AGraphNode::duplicateLink(const SafePtr<AGraphNode>& copyNode, const AGraphNode& dataToCopy)
{
	AGraphNode::addGeometricLink(dataToCopy.m_geometricParent, copyNode);
	for (const SafePtr<AGraphNode>& geoChildToCopy : dataToCopy.m_geometricChildren)
		AGraphNode::addGeometricLink(copyNode, geoChildToCopy);

	AGraphNode::addOwningLink(dataToCopy.m_owningHierarchyParent, copyNode);
	AGraphNode::addOwningLink(dataToCopy.m_owningObjectParent, copyNode);
	AGraphNode::addOwningLink(dataToCopy.m_owningPipingParent, copyNode);
	for (const SafePtr<AGraphNode>& ownChildToCopy : dataToCopy.m_owningChildren)
		AGraphNode::addOwningLink(copyNode, ownChildToCopy);

}

void AGraphNode::recOnAncestors(const SafePtr<AGraphNode>& currentPtr, std::function<std::unordered_set<SafePtr<AGraphNode>>(const SafePtr<AGraphNode>&)> getterSet,
	std::unordered_set<SafePtr<AGraphNode>>& visitedNodes, std::function<bool(const SafePtr<AGraphNode>&)> recAction)
{
	{
		ReadPtr<AGraphNode> current = currentPtr.cget();
		if (!verifNode(current))
			return;
	}

	std::unordered_set<SafePtr<AGraphNode>> datas;
	datas = getterSet(currentPtr);

	if (!recAction(currentPtr))
		return;

	if (visitedNodes.find(currentPtr) != visitedNodes.end())
		return;

	visitedNodes.insert(currentPtr);

	for (const SafePtr<AGraphNode>& data : datas)
		recOnAncestors(data, getterSet, visitedNodes, recAction);
}

void AGraphNode::recOnAncestors_read(const SafePtr<AGraphNode>& currentPtr, std::function<std::unordered_set<SafePtr<AGraphNode>>(const ReadPtr<AGraphNode>&)> getterSet, std::unordered_set<SafePtr<AGraphNode>>& visitedNodes, std::function<bool(const SafePtr<AGraphNode>&)> recAction)
{
	std::unordered_set<SafePtr<AGraphNode>> datas;
	{
		ReadPtr<AGraphNode> current = currentPtr.cget();
		if (!verifNode(current))
			return;
		datas = getterSet(current);
	}

	if (!recAction(currentPtr))
		return;

	if (visitedNodes.find(currentPtr) != visitedNodes.end())
		return;

	visitedNodes.insert(currentPtr);

	for (const SafePtr<AGraphNode>& data : datas)
		recOnAncestors_read(data, getterSet, visitedNodes, recAction);
}


SafePtr<AGraphNode> AGraphNode::getGeometricParent(const SafePtr<AGraphNode>& child)
{
	SafePtr<AGraphNode> parent;
	{
		ReadPtr<AGraphNode> rChild = child.cget();
		if (!verifNode(rChild))
			return parent;
		parent = rChild->m_geometricParent;
	}

	ReadPtr<AGraphNode> rParent = parent.cget();
	if(!verifNode(rParent))
		return SafePtr<AGraphNode>();

	return parent;
}

std::unordered_set<SafePtr<AGraphNode>> AGraphNode::getGeometricChildren_rec(const SafePtr<AGraphNode>& parent)
{
    std::unordered_set<SafePtr<AGraphNode>> geoChildren;
    {
        ReadPtr<AGraphNode> rParent = parent.cget();
        if (!verifNode(rParent))
            return {};
        geoChildren = rParent->m_geometricChildren;
    }

    std::unordered_set<SafePtr<AGraphNode>> retGeoChildren;

    for (const SafePtr<AGraphNode>& geoChild : geoChildren)
        recOnAncestors_read(geoChild, [](const ReadPtr<AGraphNode>& rPtr) { return getGeometricChildren_read(rPtr); }, retGeoChildren);

	retGeoChildren.erase(parent);
    return retGeoChildren;
}

std::unordered_set<SafePtr<AGraphNode>> AGraphNode::getGeometricChildren(const SafePtr<AGraphNode>& parent)
{
    ReadPtr<AGraphNode> rParent = parent.cget();
    if (!verifNode(rParent))
        return {};
    return verifNodes(rParent->m_geometricChildren);
}

std::unordered_set<SafePtr<AGraphNode>> AGraphNode::getGeometricChildren_read(const ReadPtr<AGraphNode>& parent)
{
    if (!verifNode(parent))
        return {};
    return verifNodes(parent->m_geometricChildren);
}

void AGraphNode::cleanLinks(const SafePtr<AGraphNode>& nodeToClean)
{
	cleanGeometricLinks(nodeToClean);
	cleanOwningLinks(nodeToClean);
}

bool AGraphNode::addGeometricLink(const SafePtr<AGraphNode>& parentPtr, const SafePtr<AGraphNode>& childPtr)
{
	//Si l'enfant a rajouté au parent sont égaux (on ne veut pas d'auto lien géométrique)
	//alors on ne fait rien et on continue la boucle for
	//  
	//TODO Possible : verification de boucle ? c'est à dire vérifier que l'enfant n'est pas un ascendant géométrique du parent.
	std::unordered_set<SafePtr<AGraphNode>> visitedNodes;
	if (parentPtr == childPtr ||
		recIsAncestor(childPtr, parentPtr, [](const SafePtr<AGraphNode>& node) {
			std::unordered_set<SafePtr<AGraphNode>> ret;
			ret.insert(getGeometricParent(node));
			return ret;
		}, visitedNodes, false))
		return false;

	SafePtr<AGraphNode> oldParentPtr = getGeometricParent(childPtr);
	if (parentPtr == oldParentPtr)
		return true;

	TransformationModule childTransfo;
	{
		ReadPtr<AGraphNode> rChild = childPtr.cget();
		if (!rChild)
			return false;
		childTransfo = rChild->getCumulTransformationModule();
	}

	WritePtr<AGraphNode> wChild, wParent, wOldParentPtr;
	if(oldParentPtr)
		multi_get(childPtr, parentPtr, oldParentPtr, wChild, wParent, wOldParentPtr);
	else
		multi_get(childPtr, parentPtr, wChild, wParent);

	if (!(wChild) || !(wParent))
		return false;

	//Si l'enfant avait déjà un parent, on l'enlève des enfants de son ancien parent (différent de son nouveau)
	if (wOldParentPtr)
		wOldParentPtr->m_geometricChildren.erase(childPtr);

	TransformationModule moduleParent = wParent->getCumulTransformationModule();
	childTransfo.compose_inverse_left(moduleParent);
	wChild->setTransformationModule(childTransfo);

	//On rajoute l'enfant au parent
	wParent->m_geometricChildren.insert(childPtr);
	wChild->m_geometricParent = parentPtr;

	return true;
}

bool AGraphNode::removeGeometricLink(const SafePtr<AGraphNode>& parentPtr, const SafePtr<AGraphNode>& childPtr)
{
	WritePtr<AGraphNode> wChild, wParent;

	multi_get(childPtr, parentPtr, wChild, wParent);
	if (!verifNode(wChild) || !verifNode(wParent))
		return false;

	wChild->m_geometricParent.reset();
	wParent->m_geometricChildren.erase(childPtr);

	return true;
}

void AGraphNode::cleanGeometricLinks(const SafePtr<AGraphNode>& ptrToClean)
{
	SafePtr<AGraphNode> parent;
	std::unordered_set<SafePtr<AGraphNode>> children;
	{
		ReadPtr<AGraphNode> toClean = ptrToClean.cget();
		if (!verifNode(toClean))
			return;
		children = getGeometricChildren_read(toClean);
	}

	parent = getGeometricParent(ptrToClean);

	AGraphNode::removeGeometricLink(parent, ptrToClean);
	for(const SafePtr<AGraphNode>& childPtr : children)
		AGraphNode::removeGeometricLink(ptrToClean, childPtr);
}

SafePtr<AGraphNode> AGraphNode::getOwningObjectParent() const
{
	return m_owningObjectParent;
}

SafePtr<AGraphNode> AGraphNode::getOwningParent(const SafePtr<AGraphNode>& child, TreeType treetype)
{
	SafePtr<AGraphNode> owningParent;
	{
		ReadPtr<AGraphNode> rChild = child.cget();
		if (!verifNode(rChild))
			return SafePtr<AGraphNode>();

		switch (treetype)
		{
		case TreeType::Hierarchy:
			owningParent = rChild->m_owningHierarchyParent;
			break;
		case TreeType::Piping:
			owningParent = rChild->m_owningPipingParent;
			break;
		default:
			owningParent = rChild->m_owningObjectParent;
			break;

		}
	}

	{
		ReadPtr<AGraphNode> rOwnParent = owningParent.cget();
		if (!verifNode(rOwnParent))
			return SafePtr<AGraphNode>();
	}
	return owningParent;
}

std::unordered_set<SafePtr<AGraphNode>> AGraphNode::getOwningParent_rec(const SafePtr<AGraphNode>& child, TreeType type)
{
	std::unordered_set<SafePtr<AGraphNode>> owningParents;
	recOnAncestors(child, [type](const SafePtr<AGraphNode>& node) { return std::unordered_set<SafePtr<AGraphNode>>({ getOwningParent(node, type) }); }, owningParents);
	owningParents.erase(child);
	return verifNodes(owningParents);
}

std::unordered_set<SafePtr<AGraphNode>> AGraphNode::getOwningParents(const SafePtr<AGraphNode>& child)
{
	std::unordered_set<SafePtr<AGraphNode>> parents;
	parents.insert(getOwningParent(child));
	parents.insert(getOwningParent(child, TreeType::Hierarchy));
	parents.insert(getOwningParent(child, TreeType::Piping));

	return parents;
}

std::unordered_set<SafePtr<AGraphNode>> AGraphNode::getOwningParents_rec(const SafePtr<AGraphNode>& child)
{
	std::unordered_set<SafePtr<AGraphNode>> parents;
	parents.merge(getOwningParent_rec(child));
	parents.merge(getOwningParent_rec(child, TreeType::Hierarchy));
	parents.merge(getOwningParent_rec(child, TreeType::Piping));

	return parents;
}

std::unordered_set<SafePtr<AGraphNode>> AGraphNode::getOwningChildren(const SafePtr<AGraphNode>& parent)
{
	std::unordered_set<SafePtr<AGraphNode>> owningChildren;
	{
		ReadPtr<AGraphNode> rParent = parent.cget();
		if (!verifNode(rParent))
			return {};
		owningChildren = rParent->m_owningChildren;
	}

	return verifNodes(owningChildren);;
}

std::unordered_set<SafePtr<AGraphNode>> AGraphNode::getOwningChildren_rec(const SafePtr<AGraphNode>& parent)
{
	std::unordered_set<SafePtr<AGraphNode>> owningChildren;
	recOnAncestors(parent, [](const SafePtr<AGraphNode>& node) { return getOwningChildren(node); }, owningChildren);
	owningChildren.erase(parent);
	return verifNodes(owningChildren);
}

bool AGraphNode::isAcceptableOwningChild(const SafePtr<AGraphNode>& child) const
{
	return false;
}

bool AGraphNode::isOwningAncestor(const SafePtr<AGraphNode>& ancestor, const SafePtr<AGraphNode>& node)
{
	std::unordered_set<SafePtr<AGraphNode>> visitedNodes;
	return (ancestor == node || recIsAncestor(ancestor, node, [](const SafePtr<AGraphNode>& node) { return getOwningParents(node); }, visitedNodes));
}

bool AGraphNode::addOwningLink(const SafePtr<AGraphNode>& parentPtr, const SafePtr<AGraphNode>& childPtr)
{

	if (!parentPtr || !childPtr)
		return false;
	//Si l'enfant a rajouté au parent sont égaux (on ne veut pas d'auto lien géométrique)
	//alors on ne fait rien
	std::unordered_set<SafePtr<AGraphNode>> visitedNodes;
	if (parentPtr == childPtr || recIsAncestor(childPtr, parentPtr, [](const SafePtr<AGraphNode>& node) { return getOwningChildren(node); }, visitedNodes, false))
		return false;

	TreeType ttype;
	{
		ReadPtr<AGraphNode> rParent = parentPtr.cget();
		if (!rParent)
			return false;
		if (!rParent->isAcceptableOwningChild(childPtr))
			return false;
		ttype = rParent->getDefaultTreeType();
	}

	SafePtr<AGraphNode> oldParent = getOwningParent(childPtr, ttype);

	if (parentPtr == oldParent)
		return true;

	WritePtr<AGraphNode> wChild, wParent, wOldParent;

	if(oldParent)
		multi_get(childPtr, parentPtr, oldParent, wChild, wParent, wOldParent);
	else
		multi_get(childPtr, parentPtr, wChild, wParent);

	if (!(wChild) || !(wParent))
		return false;

	if (wOldParent)
		wOldParent->m_owningChildren.erase(childPtr);

	//On rajoute l'enfant au parent
	wParent->m_owningChildren.insert(childPtr);
	switch (ttype)
	{
		case TreeType::Hierarchy:
			wChild->m_owningHierarchyParent = parentPtr;
			break;
		case TreeType::Piping:
			wChild->m_owningPipingParent = parentPtr;
			break;
		default:
			wChild->m_owningObjectParent = parentPtr;
			break;

	};

	return true;
}

bool AGraphNode::removeOwningLink(const SafePtr<AGraphNode>& parentPtr, const SafePtr<AGraphNode>& childPtr)
{
	WritePtr<AGraphNode> wChild, wParent;

	multi_get(childPtr, parentPtr, wChild, wParent);
	if (!verifNode(wChild) || !verifNode(wParent))
		return false;

	if (wChild->m_owningHierarchyParent == parentPtr)
		wChild->m_owningHierarchyParent = SafePtr<AGraphNode>();
	if (wChild->m_owningPipingParent == parentPtr)
		wChild->m_owningPipingParent = SafePtr<AGraphNode>();
	if (wChild->m_owningObjectParent == parentPtr)
		wChild->m_owningObjectParent = SafePtr<AGraphNode>();

	wParent->m_owningChildren.erase(childPtr);
	return true;
}

void AGraphNode::cleanOwningLinks(const SafePtr<AGraphNode>& ptrToClean)
{
	std::unordered_set<SafePtr<AGraphNode>> parents;
	std::unordered_set<SafePtr<AGraphNode>> children;

	parents = getOwningParents(ptrToClean);
	children = getOwningChildren(ptrToClean);

	for (const SafePtr<AGraphNode>& parentPtr : parents)
		AGraphNode::removeOwningLink(parentPtr, ptrToClean);

	for (const SafePtr<AGraphNode>& childPtr : children)
		AGraphNode::removeOwningLink(ptrToClean, childPtr);
}

uint32_t AGraphNode::getGraphicId() const
{
	return m_graphicId;
}

AGraphNode::Type AGraphNode::getGraphType() const
{
	return Type::Default;
}

bool AGraphNode::isDead() const
{
	return m_isDead;
}

bool AGraphNode::isDisplayed() const
{
	return isVisible();
}

void AGraphNode::setDead(bool dead)
{
	m_isDead = dead;
}

glm::dvec3 AGraphNode::getTranslation(const bool& cumulated) const
{
	{
		ReadPtr<AGraphNode> rGeoParent = m_geometricParent.cget();
		if (rGeoParent && cumulated)
			return rGeoParent->getCumulatedTransformation() * glm::dvec4(m_center, 1.0);
	}
    return m_center;
}

glm::dquat AGraphNode::getRotation(const bool& cumulated) const
{
	{
		ReadPtr<AGraphNode> rGeoParent = m_geometricParent.cget();
		if (rGeoParent && cumulated)
			return rGeoParent->getRotation(cumulated) * m_quaternion;
	}
	return m_quaternion;
}

glm::dvec3 AGraphNode::getScale(const bool& cumulated) const
{
    // QUESTION(robin) - Peut-on vraiment cumuler les scales anisotropes ?
	{
		ReadPtr<AGraphNode> rGeoParent = m_geometricParent.cget();
		if (rGeoParent && cumulated)
			return rGeoParent->getCumulatedTransformation() * glm::dvec4(m_scale, 0.0);
	}
    return m_scale;
}

glm::dmat4 AGraphNode::getCumulatedTransformation() const
{
	{
		ReadPtr<AGraphNode> rGeoParent = m_geometricParent.cget();
		if (rGeoParent)
			return rGeoParent->getCumulatedTransformation() * getTransformation();
	}
	return getTransformation();
}

TransformationModule AGraphNode::getCumulTransformationModule() const
{
	{
		ReadPtr<AGraphNode> rGeoParent = m_geometricParent.cget();
		if (rGeoParent)
			return rGeoParent->getCumulTransformationModule() * (*this);
	}
	return (*this);
}

void AGraphNode::manipulateTransfo(const ManipulateData& manipData)
{
	addGlobalTranslation(manipData.translation);
	addScale(manipData.addScale);
	glm::dvec3 relativeRotationCenter = manipData.globalRotationCenter - getTranslation(true);
	addPostRotation(manipData.addRotation, relativeRotationCenter);
}

void AGraphNode::setDefaultData(const Controller& controller)
{
	return;
}

ElementType AGraphNode::getType() const
{
	return ElementType::None;
}

TreeType AGraphNode::getDefaultTreeType() const
{
	return TreeType::MAXENUM;
}

std::vector<TreeType> AGraphNode::getArboTreeTypes() const
{
	std::vector<TreeType> treetypes;

	treetypes.push_back(getDefaultTreeType());

	if (m_owningHierarchyParent)
		treetypes.push_back(TreeType::Hierarchy);
	if (m_owningPipingParent)
		treetypes.push_back(TreeType::Piping);

	return treetypes;
}
