#include "models/3d/NodeFunctions.h"
#include "models/graph/TorusNode.h"
#include "models/graph/SphereNode.h"
#include "models/graph/BoxNode.h"
#include "models/graph/CylinderNode.h"
#include "models/graph/APointCloudNode.h"
#include "models/graph/MeshObjectNode.h"

#include "utils/math/basic_define.h"

double nodeFunctions::calculateVolume(const SafePtr<AGraphNode>& node)
{
	ElementType type;
	{
		ReadPtr<AGraphNode> rNode = node.cget();
		if (!rNode)
			return 0.0;
		type = rNode->getType();
	}

	switch (type)
	{
		case ElementType::Box:
		case ElementType::Grid:
		{
			ReadPtr<BoxNode> rBox = static_pointer_cast<BoxNode>(node).cget();
			if (!rBox)
				break;
			glm::dvec3 size = rBox->getSize();
			return size.x * size.y * size.z;
		}
		break;
		case ElementType::Sphere:
		{
			ReadPtr<SphereNode> rSphere = static_pointer_cast<SphereNode>(node).cget();
			if (!rSphere)
				break;
			double r = rSphere->getRadius();
			return double((4.0 / 3.0) * M_PI * r * r * r);
		}
		break;
		case ElementType::Cylinder:
		{
			ReadPtr<CylinderNode> rCyl = static_pointer_cast<CylinderNode>(node).cget();
			if (!rCyl)
				break;
			double r = rCyl->getRadius();
			double h = rCyl->getLength();
			return h * M_PI * r * r;
		}
		break;
		case ElementType::Torus:
		{
			ReadPtr<TorusNode> rTorus = static_pointer_cast<TorusNode>(node).cget();
			if (!rTorus)
				break;

			double tubeRadius = rTorus->getAdjustedTubeRadius();
			return rTorus->getMainAngle() * rTorus->getMainRadius() * M_PI * tubeRadius * tubeRadius;
		}
		break;
	}

	return 0.0;
}

bool nodeFunctions::isMissingFile(const SafePtr<AGraphNode>& node)
{
	bool missingFile = false;

	ElementType type;
	{
		ReadPtr<AGraphNode> rNode = node.cget();
		if (!rNode)
			return false;
		type = rNode->getType();
	}

	switch (type)
	{
	case ElementType::PCO:
	case ElementType::Scan:
	{
		ReadPtr<APointCloudNode> rPc = static_pointer_cast<APointCloudNode>(node).cget();
		if (!rPc)
			break;
		missingFile = !(rPc->getScanGuid().isValid());
	}
	break;
	case ElementType::MeshObject:
	{
		ReadPtr<MeshObjectNode> rMesh = static_pointer_cast<MeshObjectNode>(node).cget();
		if (!rMesh)
			break;
		missingFile = !(rMesh->getMeshId().isValid());
	}
	break;
	}

	return missingFile;
}
