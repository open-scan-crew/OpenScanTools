#include "io/DataComparator.h"
#include "io/SerializerKeys.h"

//new
/*
std::set<std::string> DataComparator::CompareGenericProperties(const Data& d1, const Data& d2)
{
	std::set<std::string> diff;

	if (d1.getName() != d2.getName())
	{
		diff.insert(Key_Name);
	}
	if (d1.getDescription() != d2.getDescription())
	{
		diff.insert(Key_Description);
	}
	if (d1.getIdentifier() != d2.getIdentifier())
	{
		diff.insert(Key_Identifier);
	}
	if (d1.getDiscipline() != d2.getDiscipline())
	{
		diff.insert(Key_Discipline);
	}
	if (d1.getUserIndex() != d2.getUserIndex())
	{
		diff.insert(Key_UserId);
	}
	if (d1.getCreationTime() != d2.getCreationTime())
	{
		diff.insert(Key_ImportTime);
	}
	if (d1.getModificationTime() != d2.getModificationTime())
	{
		diff.insert(Key_ModificationTime);
	}
	if (d1.getId() != d2.getId())
	{
		diff.insert(Key_Id);
	}
	if (d1.getPhase() != d2.getPhase())
	{
		diff.insert(Key_Phase);
	}
	if (d1.isVisible() != d2.isVisible())
	{
		diff.insert(Key_ShowHide);
	}
	if (d1.getForm() != d2.getForm())
	{
		diff.insert(Key_Type);
	}
	if (d1.getAuthorId() != d2.getAuthorId())
	{
		diff.insert(Key_Author);
	}
	if (d1.getHyperlinks() != d2.getHyperlinks())
	{
		diff.insert(Key_Hyperlinks);
	}
	return diff;
}

std::set<std::string> DataComparator::CompareClippingProperties(const UIClipping& d1, const UIClipping& d2)
{
	std::set<std::string> diff;

	if (d1.getClippingMode() != d2.getClippingMode())
	{
		diff.insert(Key_ClippingMode);
	}

	if (d1.getUserScale() != d2.getUserScale())
	{
		diff.insert(Key_ClippingUserValue);
	}

	if (d1.isClippingActive() != d2.isClippingActive())
	{
		diff.insert(Key_Active);
	}
	return diff;
}

std::set<std::string> DataComparator::CompareObject3DProperties(const UIObject3D& d1, const UIObject3D& d2)
{
	std::set<std::string> diff;

	if (d1.getColor().r != d2.getColor().r || d1.getColor().g != d2.getColor().g || d1.getColor().b != d2.getColor().b || d1.getColor().a != d2.getColor().a)
	{
		diff.insert(Key_ColorRGBA);
	}
	if (d1.getCenter().x != d2.getCenter().x || d1.getCenter().y != d2.getCenter().y || d1.getCenter().z != d2.getCenter().z)
	{
		diff.insert(Key_Center);
	}
	if (d1.getOrientation()[0] != d2.getOrientation()[0] || d1.getOrientation()[1] != d2.getOrientation()[1] || d1.getOrientation()[2] != d2.getOrientation()[2] || d1.getOrientation()[3] != d2.getOrientation()[3])
	{
		diff.insert(Key_Quaternion);
	}
	if (d1.getScale().x != d2.getScale().x || d1.getScale().y != d2.getScale().y || d1.getScale().z != d2.getScale().z)
	{
		diff.insert(Key_Size);
	}
	return diff;
}

/*
std::set<std::string> DataComparator::Compare(const UITag& data1, const UITag& data2)
{
	std::set<std::string> diff;
	std::set<std::string> diff2;
	std::set<std::string> diff3;
	std::set<std::string> diff4;

	diff2 = CompareClippingProperties(data1, data2);
	diff3 = CompareObject3DProperties(data1, data2);
	diff4 = CompareGenericProperties(data1, data2);

	for (std::string i : diff2)
	{
		diff.insert(i);
	}
	for (std::string i : diff3)
	{
		diff.insert(i);
	}
	for (std::string i : diff4)
	{
		diff.insert(i);
	}

	if (data1.getTemplateId().str() != data2.getTemplateId().str())
	{
		diff.insert(Key_TemplateId);
	}
	if (data1.getMarkerIcon() != data2.getMarkerIcon())
	{
		diff.insert(Key_IconId);
	}
	if (data1.getFields() != data2.getFields())
	{
		diff.insert(Key_Fields);
	}
	return diff;
}

std::set<std::string> DataComparator::Compare(const UICluster& data1, const UICluster& data2)
{
	std::set<std::string> diff;
	std::set<std::string> diff2;
	std::set<std::string> diff3;

	diff2 = CompareGenericProperties(data1, data2);

	diff3 = CompareObject3DProperties(data1,data2);


	for (std::string i : diff2)
	{
		diff.insert(i);
	}
	for (std::string i : diff3)
	{
		diff.insert(i);
	}

	return diff;
}

std::set<std::string> DataComparator::Compare(const UIMasterCluster& data1, const UIMasterCluster& data2)
{
	std::set<std::string> diff;
	std::set<std::string> diff2;

	//CompareObject3DProperties(data1, data2);
	diff2 = CompareGenericProperties(data1, data2);

	for (std::string i : diff2)
	{
		diff.insert(i);
	}
	return diff;
}

std::set<std::string> DataComparator::Compare(const UIScan& data1, const UIScan& data2)
{
	std::set<std::string> diff;
	std::set<std::string> diff2;
	std::set<std::string> diff3;

	diff2 = CompareObject3DProperties(data1, data2);
	diff3 = CompareGenericProperties(data1, data2);

	for (std::string i : diff2)
	{
		diff.insert(i);
	}
	for (std::string i : diff3)
	{
		diff.insert(i);
	}

	if (data1.getScanPath().wstring() != data2.getScanPath().wstring())
	{
		diff.insert(Key_Path);
	}
	if (data1.getClippable() != data2.getClippable())
	{
		diff.insert(Key_Clippable);
	}
	// TODO - Add a data comparator for ScanData
	return diff;
}

std::set<std::string> DataComparator::Compare(const UIBox& data1, const UIBox& data2)
{
	std::set<std::string> diff;
	std::set<std::string> diff2;
	std::set<std::string> diff3;
	std::set<std::string> diff4;

	diff2 = CompareClippingProperties(data1, data2);
	diff3 = CompareObject3DProperties(data1, data2);
	diff4 = CompareGenericProperties(data1, data2);

	for (std::string i : diff2)
	{
		diff.insert(i);
	}
	for (std::string i : diff3)
	{
		diff.insert(i);
	}
	for (std::string i : diff4)
	{
		diff.insert(i);
	}

	return diff;
}

std::set<std::string> DataComparator::Compare(const UIGrid& data1, const UIGrid& data2)
{
	std::set<std::string> diff;
	std::set<std::string> diff2;
	std::set<std::string> diff3;
	std::set<std::string> diff4;

	diff2 = CompareClippingProperties(data1, data2);
	diff3 = CompareObject3DProperties(data1, data2);
	diff4 = CompareGenericProperties(data1, data2);

	for (std::string i : diff2)
	{
		diff.insert(i);
	}
	for (std::string i : diff3)
	{
		diff.insert(i);
	}
	for (std::string i : diff4)
	{
		diff.insert(i);
	}

	if (data1.getGridType() != data2.getGridType())
	{
		diff.insert(Key_GridType);
	}
	if (data1.getGridDivision().x != data2.getGridDivision().x || data1.getGridDivision().y != data2.getGridDivision().y || data1.getGridDivision().z != data2.getGridDivision().z)
	{
		diff.insert(Key_Division);
	}
	return diff;
}

std::set<std::string> DataComparator::Compare(const UICylinder& data1, const UICylinder& data2)
{
	std::set<std::string> diff;

	std::set<std::string> diff2;
	std::set<std::string> diff3;
	std::set<std::string> diff4;

	diff2 = CompareClippingProperties(data1, data2);
	diff3 = CompareObject3DProperties(data1, data2);
	diff4 = CompareGenericProperties(data1, data2);

	for (std::string i : diff2)
	{
		diff.insert(i);
	}
	for (std::string i : diff3)
	{
		diff.insert(i);
	}
	for (std::string i : diff4)
	{
		diff.insert(i);
	}
	

	if (data1.getDetectedRadius() != data2.getDetectedRadius())
	{
		diff.insert(Key_DetectedRadius);
	}
	if (data1.getStandard().str() != data2.getStandard().str())
	{
		diff.insert(Key_TubeStandardId);
	}
	if (data1.getDiameterSet() != data2.getDiameterSet())
	{
		diff.insert(Key_DiameterSet);
	}
	return diff;
}

std::set<std::string> DataComparator::Compare(const UITorus& data1, const UITorus& data2)
{
	std::set<std::string> diff;

	std::set<std::string> diff2;
	std::set<std::string> diff3;
	std::set<std::string> diff4;

	diff2 = CompareClippingProperties(data1, data2);
	diff3 = CompareObject3DProperties(data1, data2);
	diff4 = CompareGenericProperties(data1, data2);

	for (std::string i : diff2)
	{
		diff.insert(i);
	}
	for (std::string i : diff3)
	{
		diff.insert(i);
	}
	for (std::string i : diff4)
	{
		diff.insert(i);
	}

	if (data1.getMainAngle() != data2.getMainAngle())
	{
		diff.insert(Key_MainAngle);
	}
	if (data1.getMainRadius() != data2.getMainRadius())
	{
		diff.insert(Key_MainRadius);
	}
	if (data1.getTubeRadius() != data2.getTubeRadius())
	{
		diff.insert(Key_TubeRadius);
	}
	if (data1.getTorusCenter().x != data2.getTorusCenter().x || data1.getTorusCenter().y != data2.getTorusCenter().y || data1.getTorusCenter().z != data2.getTorusCenter().z)
	{
		diff.insert(Key_TorusCenter);
	}
	return diff;
}

std::set<std::string> DataComparator::Compare(const UIPiping& data1, const UIPiping& data2)
{
	std::set<std::string> diff;
	std::set<std::string> diff2;


	diff2 = CompareGenericProperties(data1, data2);

	for (std::string i : diff2)
	{
		diff.insert(i);
	}

	if (data1.getPipingList() != data2.getPipingList())
	{
		diff.insert(Key_PipingElems);
	}
	return diff;
}


std::set<std::string> DataComparator::Compare(const UISphere& data1, const UISphere& data2)
{
	std::set<std::string> diff;

	std::set<std::string> diff2;
	std::set<std::string> diff3;
	std::set<std::string> diff4;

	diff2 = CompareClippingProperties(data1, data2);
	diff3 = CompareObject3DProperties(data1, data2);
	diff4 = CompareGenericProperties(data1, data2);

	for (std::string i : diff2)
	{
		diff.insert(i);
	}
	for (std::string i : diff3)
	{
		diff.insert(i);
	}
	for (std::string i : diff4)
	{
		diff.insert(i);
	}

	if (data1.getDetectedRadius() != data2.getDetectedRadius())
	{
		diff.insert(Key_DetectedRadius);
	}
	if (data1.getDiameterSet() != data1.getDiameterSet())
	{
		diff.insert(Key_DiameterSet);
	}
	return diff;
}

std::set<std::string> DataComparator::Compare(const UIPoint& data1, const UIPoint& data2)
{
	std::set<std::string> diff;

	std::set<std::string> diff2;
	std::set<std::string> diff3;
	std::set<std::string> diff4;

	diff2 = CompareClippingProperties(data1, data2);
	diff3 = CompareObject3DProperties(data1, data2);
	diff4 = CompareGenericProperties(data1, data2);

	for (std::string i : diff2)
	{
		diff.insert(i);
	}
	for (std::string i : diff3)
	{
		diff.insert(i);
	}
	for (std::string i : diff4)
	{
		diff.insert(i);
	}
	return diff;
}

std::set<std::string> DataComparator::Compare(const UIPCObject& data1, const UIPCObject& data2)
{
	std::set<std::string> diff;

	std::set<std::string> diff2;
	std::set<std::string> diff3;
	std::set<std::string> diff4;

	diff2 = CompareClippingProperties(data1, data2);
	diff3 = CompareObject3DProperties(data1, data2);
	diff4 = CompareGenericProperties(data1, data2);

	for (std::string i : diff2)
	{
		diff.insert(i);
	}
	for (std::string i : diff3)
	{
		diff.insert(i);
	}
	for (std::string i : diff4)
	{
		diff.insert(i);
	}

	if (data1.getScanPath().wstring() != data2.getScanPath().wstring())
	{
		diff.insert(Key_Path);
	}
	if (data1.getClippingType() != data2.getClippingType())
	{
		diff.insert(Key_ClippingType);
	}
	if (data1.getClippable() != data2.getClippable())
	{
		diff.insert(Key_Clippable);
	}
	return diff;
	// TODO - Add a comparator for ScanData
}

std::set<std::string> DataComparator::Compare(const UIMeshObject& data1, const UIMeshObject& data2)
{
	std::set<std::string> diff;
	std::set<std::string> diff2;
	std::set<std::string> diff3;

	diff2 = CompareObject3DProperties(data1, data2);
	diff3 = CompareGenericProperties(data1, data2);

	for (std::string i : diff2)
	{
		diff.insert(i);
	}
	for (std::string i : diff3)
	{
		diff.insert(i);
	}

	if (data1.getFilePath().wstring() != data2.getFilePath().wstring())
	{
		diff.insert(Key_Path);
	}
	if (data1.getObjectName() != data2.getObjectName())
	{
		diff.insert(Key_ObjectName);
	}
	if (data1.getMeshId() != data2.getMeshId())
	{
		diff.insert(Key_MeshId);
	}
	return diff;
}

std::set<std::string> DataComparator::Compare(const UISimpleMeasure& data1, const UISimpleMeasure& data2)
{
	std::set<std::string> diff;
	std::set<std::string> diff2;
	std::set<std::string> diff3;

	diff2 = CompareClippingProperties(data1, data2);
	diff3 = CompareGenericProperties(data1, data2);

	for (std::string i : diff2)
	{
		diff.insert(i);
	}
	for (std::string i : diff3)
	{
		diff.insert(i);
	}
	

	if (data1.getOriginPos().x != data2.getOriginPos().x || data1.getOriginPos().y != data2.getOriginPos().y || data1.getOriginPos().z != data2.getOriginPos().z)
	{
		diff.insert(Key_OriginPos);
	}
	if (data1.getDestinationPos().x != data2.getDestinationPos().x || data1.getDestinationPos().y != data2.getDestinationPos().y || data1.getDestinationPos().z != data2.getDestinationPos().z)
	{
		diff.insert(Key_DestPos);
	}
	return diff;
}

std::set<std::string> DataComparator::Compare(const UIPolylineMeasure& data1, const UIPolylineMeasure& data2)
{
	std::set<std::string> diff;
	std::set<std::string> diff2;
	std::set<std::string> diff3;

	diff2 = CompareClippingProperties(data1, data2);
	diff3 = CompareGenericProperties(data1, data2);

	for (std::string i : diff2)
	{
		diff.insert(i);
	}
	for (std::string i : diff3)
	{
		diff.insert(i);
	}
	
	if (!data1.getMeasures().empty() && !data2.getMeasures().empty())
	{
		if (data1.getMeasures().front().origin.x != data2.getMeasures().front().origin.x || data1.getMeasures().front().origin.y != data2.getMeasures().front().origin.y || data1.getMeasures().front().origin.z != data2.getMeasures().front().origin.z)
		{
			diff.insert(Key_Points);
		}
	}
	return diff;
}

std::set<std::string> DataComparator::Compare(const UIPointToPlaneMeasure& data1, const UIPointToPlaneMeasure& data2)
{
	std::set<std::string> diff;

	diff = CompareGenericProperties(data1, data2);

	if (data1.getPointToPlaneD() != data2.getPointToPlaneD())
	{
		diff.insert(Key_PointToPlaneDist);
	}
	if (data1.getHorizontal() != data2.getHorizontal())
	{
		diff.insert(Key_Horizontal);
	}
	if (data1.getVertical() != data2.getVertical())
	{
		diff.insert(Key_Vertical);
	}
	if (data1.getPointCoord().x != data2.getPointCoord().x || data1.getPointCoord().y != data2.getPointCoord().y || data1.getPointCoord().z != data2.getPointCoord().z)
	{
		diff.insert(Key_PointCoord);
	}
	if (data1.getPointOnPlane().x != data2.getPointOnPlane().x || data1.getPointOnPlane().y != data2.getPointOnPlane().y || data1.getPointOnPlane().z != data1.getPointOnPlane().z)
	{
		diff.insert(Key_PointOnPlane);
	}
	if (data1.getNormalToPlane().x != data2.getNormalToPlane().x || data1.getNormalToPlane().y != data2.getNormalToPlane().y || data1.getNormalToPlane().z != data2.getNormalToPlane().z)
	{
		diff.insert(Key_NormalToPlane);
	}
	if (data1.getProjPoint().x != data2.getProjPoint().x || data1.getProjPoint().y != data2.getProjPoint().y || data1.getProjPoint().z != data2.getProjPoint().z)
	{
		diff.insert(Key_ProjPoint);
	}
	return diff;
}

std::set<std::string> DataComparator::Compare(const UIPointToPipeMeasure& data1, const UIPointToPipeMeasure& data2)
{
	std::set<std::string> diff;

	diff = CompareGenericProperties(data1, data2);

	if (data1.getPointToAxeDist() != data2.getPointToAxeDist())
	{
		diff.insert(Key_PointToAxeDist);
	}
	if (data1.getPointToAxeHorizontal() != data2.getPointToAxeHorizontal())
	{
		diff.insert(Key_PointToAxeHorizontal);
	}
	if (data1.getPointToAxeVertical() != data2.getPointToAxeVertical())
	{
		diff.insert(Key_PointToAxeVertical);
	}
	if (data1.getFreeDist() != data2.getFreeDist())
	{
		diff.insert(Key_FreeDist);
	}
	if (data1.getFreeDistHorizontal() != data2.getFreeDistHorizontal())
	{
		diff.insert(Key_FreeDistHorizontal);
	}
	if (data1.getFreeDistVertical() != data2.getFreeDistVertical())
	{
		diff.insert(Key_FreeDistVertical);
	}
	if (data1.getTotalFootprint() != data2.getTotalFootprint())
	{
		diff.insert(Key_TotalFootprint);
	}
	if (data1.getPipeDiameter() != data2.getPipeDiameter())
	{
		diff.insert(Key_PipeDiameter);
	}
	if (data1.getPipeCenter().x != data2.getPipeCenter().x || data1.getPipeCenter().y != data2.getPipeCenter().y || data1.getPipeCenter().z != data2.getPipeCenter().z)
	{
		diff.insert(Key_PipeCenter);
	}
	if (data1.getPointCoord().x != data2.getPointCoord().x || data1.getPointCoord().y != data2.getPointCoord().y || data1.getPointCoord().z != data2.getPointCoord().z)
	{
		diff.insert(Key_PointCoord);
	}
	if (data1.getProjPoint().x != data2.getProjPoint().x || data1.getProjPoint().y != data2.getProjPoint().y || data1.getProjPoint().z != data2.getProjPoint().z)
	{
		diff.insert(Key_ProjPoint);
	}
	if (data1.getPipeCenterToProj() != data2.getPipeCenterToProj())
	{
		diff.insert(Key_PipeCenterToProj);
	}
	return diff;
}



std::set<std::string> DataComparator::Compare(const UIPipeToPlaneMeasure& data1, const UIPipeToPlaneMeasure& data2)
{
	std::set<std::string> diff;

	diff = CompareGenericProperties(data1, data2);

	if (data1.getCenterToPlaneDist() != data2.getCenterToPlaneDist())
	{
		diff.insert(Key_CenterToPlaneDist);
	}
	if (data1.getPlaneCenterHorizontal() != data2.getPlaneCenterHorizontal())
	{
		diff.insert(Key_PlaneCenterHorizontal);
	}
	if (data1.getPlaneCenterVertical() != data2.getPlaneCenterVertical())
	{
		diff.insert(Key_PlaneCenterVertical);
	}
	if (data1.getFreeDist() != data2.getFreeDist())
	{
		diff.insert(Key_FreeDist);
	}
	if (data1.getFreeDistHorizontal() != data2.getFreeDistHorizontal())
	{
		diff.insert(Key_FreeDistHorizontal);
	}
	if (data1.getFreeDistVertical() != data2.getFreeDistVertical())
	{
		diff.insert(Key_FreeDistVertical);
	}
	if (data1.getTotalFootprint() != data2.getTotalFootprint())
	{
		diff.insert(Key_TotalFootprint);
	}
	if (data1.getPipeDiameter() != data2.getPipeDiameter())
	{
		diff.insert(Key_PipeDiameter);
	}
	if (data1.getPipeCenter().x != data2.getPipeCenter().x || data1.getPipeCenter().y != data2.getPipeCenter().y || data1.getPipeCenter().z != data2.getPipeCenter().z)
	{
		diff.insert(Key_PipeCenter);
	}
	if (data1.getPointOnPlane() != data2.getPointOnPlane())
	{
		diff.insert(Key_PointOnPlane);
	}
	if (data1.getPointOnPlane() != data2.getPointOnPlane())
	{
		diff.insert(Key_NormalOnPlane);
	}
	if (data1.getProjPoint() != data2.getProjPoint())
	{
		diff.insert(Key_ProjPoint);
	}
	if (data1.getPointOnPlaneToProj() != data2.getPointOnPlaneToProj())
	{
		diff.insert(Key_PointOnPlaneToProj);
	}
	return diff;
}

std::set<std::string> DataComparator::Compare(const UIPipeToPipeMeasure& data1, const UIPipeToPipeMeasure& data2)
{
	std::set<std::string> diff;

	diff = CompareGenericProperties(data1, data2);

	if (data1.getCenterP1ToAxeP2() != data2.getCenterP1ToAxeP2())
	{
		diff.insert(Key_CenterP1ToAxeP2);
	}
	if (data1.getP1ToP2Horizontal() != data2.getP1ToP2Horizontal())
	{
		diff.insert(Key_P1ToP2Horizontal);
	}
	if (data1.getP1ToP2Vertical() != data2.getP1ToP2Vertical())
	{
		diff.insert(Key_P1ToP2Vertical);
	}
	if (data1.getFreeDist() != data2.getFreeDist())
	{
		diff.insert(Key_FreeDist);
	}
	if (data1.getFreeDistHorizontal() != data2.getFreeDistHorizontal())
	{
		diff.insert(Key_FreeDistHorizontal);
	}
	if (data1.getFreeDistVertical() != data2.getFreeDistVertical())
	{
		diff.insert(Key_FreeDistVertical);
	}
	if (data1.getTotalFootprint() != data2.getTotalFootprint())
	{
		diff.insert(Key_TotalFootprint);
	}
	if (data1.getPipe1Diameter() != data2.getPipe1Diameter())
	{
		diff.insert(Key_Pipe1Diameter);
	}
	if (data1.getPipe2Diameter() != data2.getPipe2Diameter())
	{
		diff.insert(Key_Pipe2Diameter);
	}
	if (data1.getPipe1Center() != data2.getPipe1Center())
	{
		diff.insert(Key_Pipe1Center);
	}
	if (data1.getPipe2Center() != data2.getPipe2Center())
	{
		diff.insert(Key_Pipe2Center);
	}
	if (data1.getProjPoint() != data2.getProjPoint())
	{
		diff.insert(Key_ProjPoint);
	}
	if (data1.getPipe2CenterToProj() != data2.getPipe2CenterToProj())
	{
		diff.insert(Key_Pipe2CenterToProj);
	}
	return diff;
}

std::set<std::string> DataComparator::Compare(const UIBeamBendingMeasure& data1, const UIBeamBendingMeasure& data2)
{
	std::set<std::string> diff;

	diff = CompareGenericProperties(data1, data2);

	if (data1.getPoint1Pos() != data2.getPoint1Pos())
	{
		diff.insert(Key_Point1);
	}
	if (data1.getPoint2Pos() != data2.getPoint2Pos())
	{
		diff.insert(Key_Point2);
	}
	if (data1.getMaxBendingPos() != data2.getMaxBendingPos())
	{
		diff.insert(Key_MaxBendingPos);
	}
	if (data1.getBendingValue() != data2.getBendingValue())
	{
		diff.insert(Key_BendingValue);
	}
	if (data1.getLength() != data2.getLength())
	{
		diff.insert(Key_Length);
	}
	if (data1.getRatio() != data2.getRatio())
	{
		diff.insert(Key_MaxRatio);
	}
	if (data1.getMaxRatio() != data2.getMaxRatio())
	{
		diff.insert(Key_Ratio);
	}
	if (data1.getRatioSup() != data2.getRatioSup())
	{
		diff.insert(Key_RatioSup);
	}
	if (data1.getResult() != data2.getResult())
	{
		diff.insert(Key_Result);
	}
	if (data1.getColor() != data2.getColor())
	{
		diff.insert(Key_ColorRGBA);
	}
	return diff;
}

std::set<std::string> DataComparator::Compare(const UIColumnTiltMeasure& data1, const UIColumnTiltMeasure& data2)
{
	std::set<std::string> diff;

	diff = CompareGenericProperties(data1, data2);

	if (data1.getPoint1() != data2.getPoint1())
	{
		diff.insert(Key_Point1);
	}
	if (data1.getPoint2() != data2.getPoint2())
	{
		diff.insert(Key_Point2);
	}
	if (data1.getBottomPoint() != data2.getBottomPoint())
	{
		diff.insert(Key_BottomPoint);
	}
	if (data1.getTopPoint() != data2.getTopPoint())
	{
		diff.insert(Key_TopPoint);
	}
	if (data1.getTiltValue() != data2.getTiltValue())
	{
		diff.insert(Key_TiltValue);
	}
	if (data1.getHeight() != data2.getHeight())
	{
		diff.insert(Key_Height);
	}
	if (data1.getRatio() != data2.getRatio())
	{
		diff.insert(Key_Ratio);
	}
	if (data1.getRatioSup() != data2.getRatioSup())
	{
		diff.insert(Key_RatioSup);
	}
	if (data1.getMaxRatio() != data2.getMaxRatio())
	{
		diff.insert(Key_MaxRatio);
	}
	if (data1.getResultReliability() != data2.getResultReliability())
	{
		diff.insert(Key_Result);
	}
	if (data1.getColor() != data2.getColor())
	{
		diff.insert(Key_ColorRGBA);
	}
	return diff;
}

std::set<std::string> DataComparator::Compare(const UserOrientation& data1, const UserOrientation& data2)
{
	std::set<std::string> diff;

	if (data1.getId() != data2.getId())
	{
		diff.insert(Key_Id);
	}
	if (data1.getName() != data2.getName())
	{
		diff.insert(Key_Name);
	}
	if (data1.getOrder() != data2.getOrder())
	{
		diff.insert(Key_Order);
	}
	if (data1.getCustomAxis() != data2.getCustomAxis())
	{
		diff.insert(Key_CustomAxis);
	}
	if (data1.getAxisType() != data2.getAxisType())
	{
		diff.insert(Key_AxisType);
	}
	if (data1.getAxisPoints()[0] != data2.getAxisPoints()[0])
	{
		diff.insert(Key_Point1);
	}
	if (data1.getAxisPoints()[1] != data2.getAxisPoints()[1])
	{
		diff.insert(Key_Point2);
	}
	if (data1.getOldPoint() != data2.getOldPoint())
	{
		diff.insert(Key_OldPoint);
	}
	if (data1.getNewPoint() != data2.getNewPoint())
	{
		diff.insert(Key_NewPoint);
	}
	return diff;
}

std::set<std::string> DataComparator::Compare(const ProjectInfos& data1, const ProjectInfos& data2)
{
	std::set<std::string> diff;

	if (data1.m_company != data2.m_company)
	{
		diff.insert(Key_Company);
	}
	if (data1.m_author != data2.m_author)
	{
		diff.insert(Key_Author);
	}
	if (data1.m_location != data2.m_location)
	{
		diff.insert(Key_Location);
	}
	if (data1.m_description != data2.m_description)
	{
		diff.insert(Key_Description);
	}
	if (data1.m_beamBendingTolerance != data2.m_beamBendingTolerance)
	{
		diff.insert(Key_BeamBendingTolerance);
	}
	if (data1.m_columnTiltTolerance != data2.m_columnTiltTolerance)
	{
		diff.insert(Key_ColumnTiltTolerance);
	}
	if (data1.m_pipesUserClippingValue != data2.m_pipesUserClippingValue)
	{
		diff.insert(Key_PipesUserClippingValue);
	}
	if (data1.m_planesUserClippingValue != data2.m_planesUserClippingValue)
	{
		diff.insert(Key_PlanesUserClippingValue);
	}
	if (data1.m_pointsUserClippingValue != data2.m_pointsUserClippingValue)
	{
		diff.insert(Key_PointsUserClippingValue);
	}
	if (data1.m_linesUserClippingValue != data2.m_linesUserClippingValue)
	{
		diff.insert(Key_LinesUserClippingValue);
	}
	if (data1.m_spheresUserClippingValue != data2.m_spheresUserClippingValue)
	{
		diff.insert(Key_SpheresUserClippingValue);
	}
	if (data1.m_importScanTranslation != data2.m_importScanTranslation)
	{
		diff.insert(Key_ImportScanTranslation);
	}
	return diff;
}

std::set<std::string> DataComparator::Compare(const UIViewPoint& data1, const UIViewPoint& data2)
{
	std::set<std::string> diff;
	std::set<std::string> diff2;
	std::set<std::string> diff3;

	diff2 = CompareGenericProperties(data1, data2);
	diff3 = CompareObject3DProperties(data1, data2);

	for (std::string i : diff2)
	{
		diff.insert(i);
	}
	for (std::string i : diff3)
	{
		diff.insert(i);
	}

	if (data1.getProjectionMode() != data2.getProjectionMode())
	{
		diff.insert(Key_Projection_Mode);
	}
	if (data1.getProjectionFrustum().l != data2.getProjectionFrustum().l || data1.getProjectionFrustum().r != data2.getProjectionFrustum().r || data1.getProjectionFrustum().b != data2.getProjectionFrustum().b || data1.getProjectionFrustum().t != data2.getProjectionFrustum().t || data1.getProjectionFrustum().n != data2.getProjectionFrustum().n || data1.getProjectionFrustum().f != data2.getProjectionFrustum().f)
	{
		diff.insert(Key_Projection_Box);
	}
	//????
	//json[Key_ScreenRatio] =
	if (data1.getDisplayParameters().m_mode != data2.getDisplayParameters().m_mode)
	{
		diff.insert(Key_Rendering_Mode);
	}
	if (data1.getDisplayParameters().m_blendMode != data2.getDisplayParameters().m_blendMode)
	{
		diff.insert(Key_Blend_Mode);
	}
	if (data1.getDisplayParameters().m_alphaObject != data2.getDisplayParameters().m_alphaObject)
	{
		diff.insert(Key_Alpha_Object);
	}
	if (data1.getDisplayParameters().m_negativeEffect != data2.getDisplayParameters().m_negativeEffect)
	{
		diff.insert(Key_NegativeEffect);
	}
	if (data1.getDisplayParameters().m_transparency != data2.getDisplayParameters().m_transparency)
	{
		diff.insert(Key_Transparency);
	}
	if (data1.getDisplayParameters().m_rampMin != data2.getDisplayParameters().m_rampMin || data1.getDisplayParameters().m_rampMax != data2.getDisplayParameters().m_rampMax || data1.getDisplayParameters().m_rampStep != data2.getDisplayParameters().m_rampStep)
	{
		diff.insert(Key_Ramp);
	}
	if (data1.getDisplayParameters().m_contrast != data2.getDisplayParameters().m_contrast)
	{
		diff.insert(Key_Contrast);
	}
	if (data1.getDisplayParameters().m_brightness != data2.getDisplayParameters().m_brightness)
	{
		diff.insert(Key_Brightness);
	}
	if (data1.getDisplayParameters().m_saturation != data2.getDisplayParameters().m_saturation)
	{
		diff.insert(Key_Saturation);
	}
	if (data1.getDisplayParameters().m_luminance != data2.getDisplayParameters().m_luminance)
	{
		diff.insert(Key_Luminance);
	}
	if (data1.getDisplayParameters().m_hue != data2.getDisplayParameters().m_hue)
	{
		diff.insert(Key_Blending);
	}
	if (data1.getDisplayParameters().m_flatColor != data2.getDisplayParameters().m_flatColor)
	{
		diff.insert(Key_Flat_Color);
	}
	if (data1.getDisplayParameters().m_backgroundColor.float32[0] != data2.getDisplayParameters().m_backgroundColor.float32[0] || data1.getDisplayParameters().m_backgroundColor.float32[1] != data2.getDisplayParameters().m_backgroundColor.float32[1] || data1.getDisplayParameters().m_backgroundColor.float32[2] != data2.getDisplayParameters().m_backgroundColor.float32[2])
	{
		diff.insert(Key_Background_Color);
	}
	if (data1.getDisplayParameters().m_pointSize != data2.getDisplayParameters().m_pointSize)
	{
		diff.insert(Key_Point_Size);
	}
	if (data1.getDisplayParameters().m_valueDisplayParameters.distanceUnit != data2.getDisplayParameters().m_valueDisplayParameters.distanceUnit || data1.getDisplayParameters().m_valueDisplayParameters.diameterUnit != data2.getDisplayParameters().m_valueDisplayParameters.diameterUnit || data1.getDisplayParameters().m_valueDisplayParameters.displayedDigits != data2.getDisplayParameters().m_valueDisplayParameters.displayedDigits)
	{
		diff.insert(Key_Display_Parameters);
	}
	if (data1.getDisplayParameters().m_measureShowMask != data2.getDisplayParameters().m_measureShowMask)
	{
		diff.insert(Key_Measure_Show_Mask);
	}
	if (data1.getDisplayParameters().m_markerTextParameters.m_textFilter != data2.getDisplayParameters().m_markerTextParameters.m_textFilter || data1.getDisplayParameters().m_markerTextParameters.m_textTheme != data2.getDisplayParameters().m_markerTextParameters.m_textTheme || data1.getDisplayParameters().m_markerTextParameters.m_textFontSize != data2.getDisplayParameters().m_markerTextParameters.m_textFontSize)
	{
		diff.insert(Key_Marker_Text_Parameters);
	}
	if (data1.getDisplayParameters().m_displayAllMarkersTexts != data2.getDisplayParameters().m_displayAllMarkersTexts)
	{
		diff.insert(Key_Display_All_Marker_Texts);
	}
	if (data1.getDisplayParameters().m_displayAllMeasures != data2.getDisplayParameters().m_displayAllMeasures)
	{
		diff.insert(Key_Display_All_Measures);
	}
	if (data1.getDisplayParameters().m_mkRenderingParams.improveVisibility != data2.getDisplayParameters().m_mkRenderingParams.improveVisibility || data1.getDisplayParameters().m_mkRenderingParams.maximumDisplayDistance != data2.getDisplayParameters().m_mkRenderingParams.maximumDisplayDistance || data1.getDisplayParameters().m_mkRenderingParams.nearLimit != data2.getDisplayParameters().m_mkRenderingParams.nearLimit || data1.getDisplayParameters().m_mkRenderingParams.farLimit != data2.getDisplayParameters().m_mkRenderingParams.farLimit || data1.getDisplayParameters().m_mkRenderingParams.nearSize != data2.getDisplayParameters().m_mkRenderingParams.nearSize || data1.getDisplayParameters().m_mkRenderingParams.farSize != data2.getDisplayParameters().m_mkRenderingParams.farSize)
	{
		diff.insert(Key_Marker_Rendering_Parameters);
	}
	if (data1.getDisplayParameters().m_postRenderingNormals.show != data2.getDisplayParameters().m_postRenderingNormals.show || data1.getDisplayParameters().m_postRenderingNormals.inverseTone != data2.getDisplayParameters().m_postRenderingNormals.inverseTone || data1.getDisplayParameters().m_postRenderingNormals.blendColor != data2.getDisplayParameters().m_postRenderingNormals.blendColor || data1.getDisplayParameters().m_postRenderingNormals.normalStrength != data2.getDisplayParameters().m_postRenderingNormals.normalStrength || data1.getDisplayParameters().m_postRenderingNormals.gloss != data2.getDisplayParameters().m_postRenderingNormals.gloss)
	{
		diff.insert(Key_Post_Rendering_Normals);
	}
	if (data1.getDisplayParameters().m_displayGizmo != data2.getDisplayParameters().m_displayGizmo)
	{
		diff.insert(Key_Display_Guizmo);
	}
	if (data1.getActiveClippings() != data2.getActiveClippings())
	{
		diff.insert(Key_Active_Clippings);
	}
	if (data1.getInteriorClippings() != data2.getInteriorClippings())
	{
		diff.insert(Key_Interior_Clippings);
	}
	if (data1.getVisibleObjects() != data2.getVisibleObjects())
	{
		diff.insert(Key_Visible_Objects);
	}
	if (data1.getScanClusterColors() != data2.getScanClusterColors())
	{
		diff.insert(Key_Objects_Colors);
	}
	return diff;
}

std::set<std::string> DataComparator::Compare(const sma::TagTemplate& data1, const sma::TagTemplate& data2)
{
	std::set<std::string> diff;


	if (data1.getId() != data2.getId())
	{
		diff.insert(Key_Id);
	}
	if (data1.getName() != data2.getName())
	{
		diff.insert(Key_Name);
	}
	if (data1.isAOriginTemplate() != data2.isAOriginTemplate())
	{
		diff.insert(Key_OriginTemplate);
	}
	if (data1.getFieldsCopy() != data2.getFieldsCopy())
	{
		diff.insert(Key_Fields);
	}
	return diff;
}
*/