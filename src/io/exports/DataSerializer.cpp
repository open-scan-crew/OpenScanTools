#include "io/exports/DataSerializer.h"

#include "utils/Logger.h"

#include "models/graph/TagNode.h"
#include "models/graph/PointNode.h"
#include "models/graph/PointCloudNode.h"

#include "models/graph/ViewPointNode.h"
#include "models/graph/CameraNode.h"

#include "models/graph/CylinderNode.h"
#include "models/graph/SphereNode.h"
#include "models/graph/TorusNode.h"

#include "models/graph/SimpleMeasureNode.h"
#include "models/graph/PolylineMeasureNode.h"
#include "models/graph/ColumnTiltMeasureNode.h"
#include "models/graph/BeamBendingMeasureNode.h"
#include "models/graph/PointToPipeMeasureNode.h"
#include "models/graph/PointToPlaneMeasureNode.h"
#include "models/graph/PipeToPipeMeasureNode.h"
#include "models/graph/PipeToPlaneMeasureNode.h"

#include "models/graph/ClusterNode.h"
#include "models/graph/BoxNode.h"
#include "models/graph/MeshObjectNode.h"

#include "models/application/TagTemplate.h"

#include "models/application/UserOrientation.h"
#include "models/project/ProjectInfos.h"
#include "models/application/Author.h"

#include "magic_enum/magic_enum.hpp"

#define IOLOG Logger::log(LoggerMode::IOLog)

//new
/*
void DataSerializer::ExportModification(nlohmann::json& modif_json,std::map<xg::Guid, std::pair <std::array<Data*, 2>, std::set<std::string>>> modification)
{
	nlohmann::json details_supp = nlohmann::json::array();
	nlohmann::json details_ajout = nlohmann::json::array();
	nlohmann::json details_modif = nlohmann::json::array();

	for (auto p = modification.begin(); p != modification.end(); p++)
	{
		nlohmann::json tempObj;

		if (p->second.first[0] != nullptr && p->second.first[1] == nullptr)
		{
			tempObj["id_object_deleted"] = p->first;
			details_supp.push_back(tempObj);
			tempObj.clear();
		}
		
		else if (p->second.first[0] == nullptr && p->second.first[1] != nullptr )
		{
			tempObj["id_object_added"] = p->first;
			details_ajout.push_back(tempObj);
			tempObj.clear();
		}
		else if ((p->second.first[1] != nullptr && p->second.first[0] != nullptr && p->second.second.empty() == false))
		{
			std::set<std::string> champ = p->second.second;
			tempObj["Field_changed"] = champ;
			tempObj["id_object_updated"] = p->first;
			details_modif.push_back(tempObj);
			tempObj.clear();
		}
	}

	modif_json["object_added"]=details_ajout;
	modif_json["object_deleted"]=details_supp;
	modif_json["object_updated"]=details_modif;

	details_ajout.clear();
	details_supp.clear();
	details_modif.clear();
}
*/
void ExportLinks(nlohmann::json& json, const SafePtr<AGraphNode>& object)
{
	{
		ReadPtr<AGraphNode> rParent = AGraphNode::getGeometricParent(object).cget();
		if (rParent)
			json[Key_GeometricParent] = rParent->getId();
	}

	nlohmann::json jsonArray = nlohmann::json::array();

	for (const SafePtr<AGraphNode>& geoChild : AGraphNode::getGeometricChildren(object))
	{
		ReadPtr<AGraphNode> rChild = geoChild.cget();
		if (rChild)
			jsonArray.push_back(rChild->getId());
	}
	json[Key_GeometricChildrens] = jsonArray;
	jsonArray.clear();

	for (const SafePtr<AGraphNode>& ownParent : AGraphNode::getOwningParents(object))
	{
		ReadPtr<AGraphNode> rOwnParent = ownParent.cget();
		if (rOwnParent)
			jsonArray.push_back(rOwnParent->getId());
	}
	json[Key_OwningParents] = jsonArray;
	jsonArray.clear();

	for (const SafePtr<AGraphNode>& ownChild : AGraphNode::getOwningChildren(object))
	{
		ReadPtr<AGraphNode> rChild = ownChild.cget();
		if (rChild)
			jsonArray.push_back(rChild->getId());
	}
	json[Key_OwningChildrens] = jsonArray;
	jsonArray.clear();
}

void ExportData(nlohmann::json& json, const Data& data)
{
	json[Key_Name] = Utils::to_utf8(data.getName());
	json[Key_Description] = Utils::to_utf8(data.getDescription());
	json[Key_Identifier] = Utils::to_utf8(data.getIdentifier());
	json[Key_Discipline] = Utils::to_utf8(data.getDiscipline());
	json[Key_Phase] = Utils::to_utf8(data.getPhase());
	json[Key_UserId] = data.getUserIndex();
	json[Key_ImportTime] = data.getCreationTime();
	json[Key_ModificationTime] = data.getModificationTime();
	json[Key_Id] = data.getId();
	json[Key_ShowHide] = data.isVisible();
	nlohmann::json hLinkarray = nlohmann::json::array();
	for (const std::pair<hLinkId, s_hyperlink>& itMap : data.getHyperlinks())
	{
		nlohmann::json tempObj;
		tempObj[Key_LinkId] = itMap.first;
		tempObj[Key_LinkName] = Utils::to_utf8(itMap.second.name);
		tempObj[Key_LinkURL] = Utils::to_utf8(itMap.second.hyperlink);
		hLinkarray.push_back(tempObj);
	}
	json[Key_Hyperlinks] = hLinkarray;
	json[Key_ColorRGBA] = { data.getColor().r, data.getColor().g, data.getColor().b, data.getColor().a };
	json[Key_IconId] = magic_enum::enum_name(data.getMarkerIcon());
}

void ExportClippingData(nlohmann::json& json, const ClippingData& data)
{
	json[Key_ClippingMode] = magic_enum::enum_name(data.getClippingMode());
	json[Key_MinClipDistance] = data.getMinClipDist();
	json[Key_MaxClipDistance] = data.getMaxClipDist();
	json[Key_Active] = data.isClippingActive();


	json[Key_RampActive] = data.isRampActive();
	json[Key_MinRampDistance] = data.getRampMin();
	json[Key_MaxRampDistance] = data.getRampMax();
	json[Key_RampSteps] = data.getRampSteps();
	json[Key_RampClamped] = data.isRampClamped();
}

void ExportTransformationModule(nlohmann::json& json, const TransformationModule& data)
{
	json[Key_Center] = { data.getCenter().x, data.getCenter().y , data.getCenter().z };
	json[Key_Quaternion] = { data.getOrientation()[0], data.getOrientation()[1], data.getOrientation()[2], data.getOrientation()[3] };
	json[Key_Size] = { data.getScale().x, data.getScale().y , data.getScale().z };
}

void ExportTagData(nlohmann::json& json, const TagData& data)
{
	xg::Guid templateId;
	ReadPtr<sma::TagTemplate> rTemp = data.getTemplate().cget();
	if (rTemp)
		templateId = rTemp->getId();

	json[Key_TemplateId] = templateId.str();

	const std::unordered_map<sma::tFieldId, std::wstring>& fields = data.getFields();
	for (auto it = fields.cbegin(); it != fields.cend(); it++)
		json[it->first.str()] = Utils::to_utf8(it->second);
}

void ExportScanData(nlohmann::json& json, const ScanData& data)
{
	json[Key_Clippable] = data.getClippable();
	//json[Key_PointCloud_IsObject] = data.is_object_;
}

void ExportClusterData(nlohmann::json& json, const ClusterData& data)
{
	json[Key_TreeType] = magic_enum::enum_name(data.getClusterTreeType());
}

void ExportGridData(nlohmann::json& json, const BoxNode& data)
{
	json[Key_GridType] = magic_enum::enum_name(data.getGridType());
	json[Key_Division] = { data.getGridDivision().x, data.getGridDivision().y , data.getGridDivision().z };
}

void ExportCylinderData(nlohmann::json& json, const StandardRadiusData& data)
{
	json[Key_DetectedRadius] = data.getDetectedRadius();
	json[Key_ForcedRadius] = data.getForcedRadius();
	xg::Guid standId;
	{
		ReadPtr<StandardList> rStand = data.getStandard().cget();
		if (rStand)
			standId = rStand->getId();
	}
	json[Key_TubeStandardId] = standId;
	json[Key_DiameterSet] = magic_enum::enum_name(data.getDiameterSet());
}

void ExportInsulationData(nlohmann::json& json, const InsulationData& data)
{
	json[Key_InsulationRadius] = data.getInsulationRadius();
}

void ExportTorusData(nlohmann::json& json, const TorusData& data)
{
	json[Key_MainAngle] = data.getMainAngle();
	json[Key_MainRadius] = data.getMainRadius();
	json[Key_TubeRadius] = data.getTubeRadius();
}

void ExportMeshObjectData(nlohmann::json& json, const MeshObjectData& data)
{
	json[Key_Path] = Utils::to_utf8(data.getFilePath().wstring());
	json[Key_ObjectName] = Utils::to_utf8(data.getObjectName());
	json[Key_MeshId] = data.getMeshId();
}

void ExportSimpleMeasureData(nlohmann::json& json, const SimpleMeasureData& data)
{
	json[Key_OriginPos] = { data.getOriginPos().x, data.getOriginPos().y , data.getOriginPos().z };
	json[Key_DestPos] = { data.getDestinationPos().x, data.getDestinationPos().y , data.getDestinationPos().z };
}

void ExportPolylineMeasureData(nlohmann::json& json, const PolylineMeasureData& data)
{
	nlohmann::json points(nlohmann::json::array());
	if (!data.getMeasures().empty())
	{
		points.push_back({ data.getMeasures().front().origin.x, data.getMeasures().front().origin.y, data.getMeasures().front().origin.z });
		for (const Measure& measure : data.getMeasures())
			points.push_back({ measure.final.x, measure.final.y, measure.final.z });
		json[Key_Points] = points;
	}
}

void ExportPointToPipeMeasureData(nlohmann::json& json, const PointToPipeMeasureData& data)
{
	json[Key_PointToAxeDist] = data.getPointToAxeDist();
	json[Key_PointToAxeHorizontal] = data.getPointToAxeHorizontal();
	json[Key_PointToAxeVertical] = data.getPointToAxeVertical();
	json[Key_FreeDist] = data.getFreeDist();
	json[Key_FreeDistHorizontal] = data.getFreeDistHorizontal();
	json[Key_FreeDistVertical] = data.getFreeDistVertical();
	json[Key_TotalFootprint] = data.getTotalFootprint();
	json[Key_PipeDiameter] = data.getPipeDiameter();
	json[Key_PipeCenter] = { data.getPipeCenter().x, data.getPipeCenter().y , data.getPipeCenter().z };
	json[Key_PointCoord] = { data.getPointCoord().x, data.getPointCoord().y , data.getPointCoord().z };
	json[Key_ProjPoint] = { data.getProjPoint().x, data.getProjPoint().y , data.getProjPoint().z };
	json[Key_PipeCenterToProj] = data.getPipeCenterToProj();
}

void ExportPointToPlaneMeasureData(nlohmann::json& json, const PointToPlaneMeasureData& data)
{
	json[Key_PointToPlaneDist] = data.getPointToPlaneD();
	json[Key_Horizontal] = data.getHorizontal();
	json[Key_Vertical] = data.getVertical();
	json[Key_PointCoord] = { data.getPointCoord().x, data.getPointCoord().y , data.getPointCoord().z };
	json[Key_PointOnPlane] = { data.getPointOnPlane().x, data.getPointOnPlane().y , data.getPointOnPlane().z };
	json[Key_NormalToPlane] = { data.getNormalToPlane().x, data.getNormalToPlane().y , data.getNormalToPlane().z };
	json[Key_ProjPoint] = { data.getProjPoint().x, data.getProjPoint().y , data.getProjPoint().z };
}

void ExportPipeToPipeMeasureData(nlohmann::json& json, const PipeToPipeMeasureData& data)
{
	json[Key_CenterP1ToAxeP2] = data.getCenterP1ToAxeP2();
	json[Key_P1ToP2Horizontal] = data.getP1ToP2Horizontal();
	json[Key_P1ToP2Vertical] = data.getP1ToP2Vertical();
	json[Key_FreeDist] = data.getFreeDist();
	json[Key_FreeDistHorizontal] = data.getFreeDistHorizontal();
	json[Key_FreeDistVertical] = data.getFreeDistVertical();
	json[Key_TotalFootprint] = data.getTotalFootprint();
	json[Key_Pipe1Diameter] = data.getPipe1Diameter();
	json[Key_Pipe2Diameter] = data.getPipe2Diameter();
	json[Key_Pipe1Center] = { data.getPipe1Center().x, data.getPipe1Center().y , data.getPipe1Center().z };
	json[Key_Pipe2Center] = { data.getPipe2Center().x, data.getPipe2Center().y , data.getPipe2Center().z };
	json[Key_ProjPoint] = { data.getProjPoint().x, data.getProjPoint().y , data.getProjPoint().z };
	json[Key_Pipe2CenterToProj] = data.getPipe2CenterToProj();
}

void ExportPipeToPlaneMeasureData(nlohmann::json& json, const PipeToPlaneMeasureData& data)
{
	json[Key_CenterToPlaneDist] = data.getCenterToPlaneDist();
	json[Key_PlaneCenterHorizontal] = data.getPlaneCenterHorizontal();
	json[Key_PlaneCenterVertical] = data.getPlaneCenterVertical();
	json[Key_FreeDist] = data.getFreeDist();
	json[Key_FreeDistHorizontal] = data.getFreeDistHorizontal();
	json[Key_FreeDistVertical] = data.getFreeDistVertical();
	json[Key_TotalFootprint] = data.getTotalFootprint();
	json[Key_PipeDiameter] = data.getPipeDiameter();
	json[Key_PipeCenter] = { data.getPipeCenter().x, data.getPipeCenter().y , data.getPipeCenter().z };
	json[Key_PointOnPlane] = { data.getPointOnPlane().x, data.getPointOnPlane().y , data.getPointOnPlane().z };
	json[Key_NormalOnPlane] = { data.getNormalOnPlane().x, data.getNormalOnPlane().y , data.getNormalOnPlane().z };
	json[Key_ProjPoint] = { data.getProjPoint().x, data.getProjPoint().y , data.getProjPoint().z };
	json[Key_PointOnPlaneToProj] = data.getPointOnPlaneToProj();
}

void ExportBeamBendingMeasureData(nlohmann::json& json, const BeamBendingMeasureData& data)
{
	json[Key_Point1] = { data.getPoint1Pos().x, data.getPoint1Pos().y , data.getPoint1Pos().z };
	json[Key_Point2] = { data.getPoint2Pos().x, data.getPoint2Pos().y , data.getPoint2Pos().z };
	json[Key_MaxBendingPos] = { data.getMaxBendingPos().x, data.getMaxBendingPos().y , data.getMaxBendingPos().z };
	json[Key_BendingValue] = data.getBendingValue();
	json[Key_Length] = data.getLength();
	json[Key_Ratio] = data.getRatio();
	json[Key_MaxRatio] = data.getMaxRatio();
	json[Key_RatioSup] = magic_enum::enum_name(data.getRatioSup());
	json[Key_Result] = magic_enum::enum_name(data.getResult());
}

void ExportColumnTiltMeasureData(nlohmann::json& json, const ColumnTiltMeasureData& data)
{
	json[Key_Point1] = { data.getPoint1().x, data.getPoint1().y , data.getPoint1().z };
	json[Key_Point2] = { data.getPoint2().x, data.getPoint2().y , data.getPoint2().z };
	json[Key_BottomPoint] = { data.getBottomPoint().x, data.getBottomPoint().y , data.getBottomPoint().z };
	json[Key_TopPoint] = { data.getTopPoint().x, data.getTopPoint().y , data.getTopPoint().z };
	json[Key_TiltValue] = data.getTiltValue();
	json[Key_Height] = data.getHeight();
	json[Key_Ratio] = data.getRatio();
	json[Key_MaxRatio] = data.getMaxRatio();
	json[Key_RatioSup] = magic_enum::enum_name(data.getRatioSup());
	json[Key_Result] = magic_enum::enum_name(data.getResultReliability());
}

void ExportRenderingParameters(nlohmann::json& json, const RenderingParameters& data)
{
	json[Key_Projection_Mode] = magic_enum::enum_name(data.getProjectionMode());
	json[Key_Projection_Box] = { data.getProjectionFrustum().l, data.getProjectionFrustum().r, data.getProjectionFrustum().b, data.getProjectionFrustum().t, data.getProjectionFrustum().n, data.getProjectionFrustum().f };

        DisplayParameters params = data.getDisplayParameters();
        json[Key_Rendering_Mode] = magic_enum::enum_name(params.m_mode);
        json[Key_Background_Color] = { params.m_backgroundColor.Red(), params.m_backgroundColor.Green(), params.m_backgroundColor.Blue() };
        json[Key_Point_Size] = params.m_pointSize;
        json[Key_Adaptive_Point_Size] = params.m_adaptivePointSize;
        json[Key_Point_Filling_Strength] = params.m_pointFillingStrength;
        json[Key_Reduce_Point_Size_Distance] = params.m_reducePointSizeWithDistance;
        json[Key_Point_Distance_Attenuation] = params.m_pointDistanceAttenuation;
        json[Key_Delta_Filling] = params.m_deltaFilling;
        json[Key_Gap_Filling_Texel_Threshold] = params.m_gapFillingTexelThreshold;

	json[Key_Contrast] = params.m_contrast;
	json[Key_Brightness] = params.m_brightness;
	json[Key_Saturation] = params.m_saturation;
	json[Key_Luminance] = params.m_luminance;
	json[Key_Blending] = params.m_hue;
	json[Key_Flat_Color] = { params.m_flatColor.x, params.m_flatColor.y, params.m_flatColor.z };

	json[Key_DistRamp] = { params.m_distRampMin, params.m_distRampMax };
	json[Key_DistRampSteps] = params.m_distRampSteps;

	json[Key_Blend_Mode] = magic_enum::enum_name(params.m_blendMode);
	json[Key_NegativeEffect] = params.m_negativeEffect;
	json[Key_ReduceFlash] = params.m_reduceFlash;
	json[Key_Transparency] = params.m_transparency;

    json[Key_Post_Rendering_Normals] = { params.m_postRenderingNormals.show, params.m_postRenderingNormals.inverseTone, params.m_postRenderingNormals.blendColor, params.m_postRenderingNormals.normalStrength, params.m_postRenderingNormals.gloss };
    json[Key_Edge_Aware_Blur] = { params.m_edgeAwareBlur.enabled, params.m_edgeAwareBlur.radius, params.m_edgeAwareBlur.depthThreshold, params.m_edgeAwareBlur.blendStrength, params.m_edgeAwareBlur.resolutionScale };
    json[Key_Depth_Lining] = { params.m_depthLining.enabled, params.m_depthLining.strength, params.m_depthLining.threshold, params.m_depthLining.sensitivity, params.m_depthLining.strongMode };

    json[Key_Display_Guizmo] = params.m_displayGizmo;
	json[Key_Ramp_Scale_Options] = { params.m_rampScale.showScale, params.m_rampScale.graduationCount, params.m_rampScale.centerBoxScale };

	json[Key_Alpha_Object] = params.m_alphaObject;
	json[Key_Distance_Unit] = magic_enum::enum_name(params.m_unitUsage.distanceUnit);
	json[Key_Diameter_Unit] = magic_enum::enum_name(params.m_unitUsage.diameterUnit);
	json[Key_Volume_Unit] = magic_enum::enum_name(params.m_unitUsage.volumeUnit);
	json[Key_Displayed_Digits] = params.m_unitUsage.displayedDigits;
	json[Key_Measure_Show_Mask] = params.m_measureMask;
	json[Key_Marker_Show_Mask] = params.m_markerMask;
	json[Key_Marker_Rendering_Parameters] = { params.m_markerOptions.improveVisibility, params.m_markerOptions.maximumDisplayDistance, params.m_markerOptions.nearLimit, params.m_markerOptions.farLimit, params.m_markerOptions.nearSize, params.m_markerOptions.farSize };
	json[Key_Text_Display_Options] = { params.m_textOptions.m_filter, params.m_textOptions.m_textTheme, params.m_textOptions.m_textFontSize };
	json[Key_Display_All_Marker_Texts] = params.m_displayAllMarkersTexts;
	json[Key_Display_All_Measures] = params.m_displayAllMeasures;

	json[Key_Ortho_Grid_Active] = params.m_orthoGridActive;
	json[Key_Ortho_Grid_Color] = { params.m_orthoGridColor.r, params.m_orthoGridColor.g, params.m_orthoGridColor.b, params.m_orthoGridColor.a };
	json[Key_Ortho_Grid_Step] = params.m_orthoGridStep;
	json[Key_Ortho_Grid_Step] = params.m_orthoGridLineWidth;
}

void ExportViewPointData(nlohmann::json& json, const ViewPointData& data)
{
        ExportRenderingParameters(json, data);

        const DisplayParameters& displayParams = data.getDisplayParameters();
        json[Key_Edge_Aware_Blur] = { displayParams.m_edgeAwareBlur.enabled, displayParams.m_edgeAwareBlur.radius,
                displayParams.m_edgeAwareBlur.depthThreshold, displayParams.m_edgeAwareBlur.blendStrength,
                displayParams.m_edgeAwareBlur.resolutionScale };
        json[Key_Depth_Lining] = { displayParams.m_depthLining.enabled, displayParams.m_depthLining.strength,
                displayParams.m_depthLining.threshold, displayParams.m_depthLining.sensitivity,
                displayParams.m_depthLining.strongMode };

	nlohmann::json childrenElem = nlohmann::json::array();
	for (const SafePtr<AClippingNode>& clip : data.getActiveClippings())
	{
		ReadPtr<AClippingNode> rClip = clip.cget();
		if (!rClip)
			continue;
		childrenElem.push_back(rClip->getId());
	}
	json[Key_Active_Clippings] = childrenElem;

	childrenElem.clear();
	for (const SafePtr<AClippingNode>& clip : data.getInteriorClippings())
	{
		ReadPtr<AClippingNode> rClip = clip.cget();
		if (!rClip)
			continue;
		childrenElem.push_back(rClip->getId());
	}
	json[Key_Interior_Clippings] = childrenElem;

	childrenElem.clear();
	for (const SafePtr<AClippingNode>& ramp : data.getActiveRamps())
	{
		ReadPtr<AClippingNode> rRamp = ramp.cget();
		if (!rRamp)
			continue;
		childrenElem.push_back(rRamp->getId());
	}
	json[Key_Active_Ramps] = childrenElem;

	childrenElem.clear();
	for (const SafePtr<AGraphNode>& obj : data.getVisibleObjects())
	{
		ReadPtr<AGraphNode> rObj = obj.cget();
		if (!rObj)
			continue;
		childrenElem.push_back(rObj->getId());
	}
	json[Key_Visible_Objects] = childrenElem;

	childrenElem.clear();
	for (std::pair<SafePtr<AGraphNode>,Color32> colorSet : data.getScanClusterColors())
	{
		ReadPtr<AGraphNode> rObj = colorSet.first.cget();
		if (!rObj)
			continue;
		childrenElem.push_back({ rObj->getId(), colorSet.second.r, colorSet.second.g, colorSet.second.b, colorSet.second.a });
	}
	json[Key_Objects_Colors] = childrenElem;
}

void DataSerializer::Serialize(nlohmann::json& json, const SafePtr<TagNode>& object)
{
	{
		ReadPtr<TagNode> rObj = object.cget();
		if (!rObj)
			return;
		ExportData(json, *&rObj);
		ExportClippingData(json, *&rObj);
		ExportTransformationModule(json, *&rObj);
		ExportTagData(json, *&rObj);
	}
	ExportLinks(json, object);
}

void DataSerializer::Serialize(nlohmann::json& json, const SafePtr<PointCloudNode>& object)
{
	{
		ReadPtr<PointCloudNode> rObj = object.cget();
		if (!rObj)
			return;
		ExportData(json, *&rObj);
		ExportTransformationModule(json, *&rObj);
		ExportScanData(json, *&rObj);
		json[Key_Path] = Utils::to_utf8(rObj->getTlsFilePath().wstring());
	}
	ExportLinks(json, object);
}

void DataSerializer::Serialize(nlohmann::json& json, const SafePtr<ClusterNode>& object)
{
	{
		ReadPtr<ClusterNode> rObj = object.cget();
		if (!rObj)
			return;
		ExportData(json, *&rObj);
		ExportTransformationModule(json, *&rObj);
		ExportClusterData(json, *&rObj);
	}
	ExportLinks(json, object);
}

void DataSerializer::Serialize(nlohmann::json& json, const SafePtr<BoxNode>& object)
{
	{
		ReadPtr<BoxNode> rObj = object.cget();
		if (!rObj)
			return;
		ExportData(json, *&rObj);
		ExportTransformationModule(json, *&rObj);
		ExportClippingData(json, *&rObj);
		ExportGridData(json, *&rObj);
	}
	ExportLinks(json, object);
}

void DataSerializer::Serialize(nlohmann::json& json, const SafePtr<CylinderNode>& object)
{
	{
		ReadPtr<CylinderNode> rObj = object.cget();
		if (!rObj)
			return;
		ExportData(json, *&rObj);
		ExportTransformationModule(json, *&rObj);
		ExportClippingData(json, *&rObj);
		ExportCylinderData(json, *&rObj);
		ExportInsulationData(json, *&rObj);

		json[Key_Length] = rObj->getLength();
	}
	ExportLinks(json, object);

}

void DataSerializer::Serialize(nlohmann::json& json, const SafePtr<TorusNode>& object)
{
	{
		ReadPtr<TorusNode> rObj = object.cget();
		if (!rObj)
			return;
		ExportData(json, *&rObj);
		ExportTransformationModule(json, *&rObj);
		ExportClippingData(json, *&rObj);
		ExportTorusData(json, *&rObj);
		ExportInsulationData(json, *&rObj);
	}
	ExportLinks(json, object);
}

void DataSerializer::Serialize(nlohmann::json& json, const SafePtr<SphereNode>& object)
{
	{
		ReadPtr<SphereNode> rObj = object.cget();
		if (!rObj)
			return;
		ExportData(json, *&rObj);
		ExportTransformationModule(json, *&rObj);
		ExportClippingData(json, *&rObj);

		json[Key_ForcedRadius] = rObj->getRadius();
	}
	ExportLinks(json, object);
}

void DataSerializer::Serialize(nlohmann::json& json, const SafePtr<PointNode>& object)
{
	{
		ReadPtr<PointNode> rObj = object.cget();
		if (!rObj)
			return;
		ExportData(json, *&rObj);
		ExportTransformationModule(json, *&rObj);
		ExportClippingData(json, *&rObj);
	}
	ExportLinks(json, object);
}

void DataSerializer::Serialize(nlohmann::json& json, const SafePtr<MeshObjectNode>& object)
{
	{
		ReadPtr<MeshObjectNode> rObj = object.cget();
		if (!rObj)
			return;
		ExportData(json, *&rObj);
		ExportTransformationModule(json, *&rObj);
		ExportMeshObjectData(json, *&rObj);
	}
	ExportLinks(json, object);
}

void DataSerializer::Serialize(nlohmann::json& json, const SafePtr<SimpleMeasureNode>& object)
{
	{
		ReadPtr<SimpleMeasureNode> rObj = object.cget();
		if (!rObj)
			return;
		ExportData(json, *&rObj);
		ExportClippingData(json, *&rObj);
		ExportSimpleMeasureData(json, *&rObj);
	}
	ExportLinks(json, object);
}

void DataSerializer::Serialize(nlohmann::json& json, const SafePtr<PolylineMeasureNode>& object)
{
	{
		ReadPtr<PolylineMeasureNode> rObj = object.cget();
		if (!rObj)
			return;
		ExportData(json, *&rObj);
		ExportClippingData(json, *&rObj);
		ExportPolylineMeasureData(json, *&rObj);
	}
	ExportLinks(json, object);
}

void DataSerializer::Serialize(nlohmann::json& json, const SafePtr<PointToPlaneMeasureNode>& object)
{
	{
		ReadPtr<PointToPlaneMeasureNode> rObj = object.cget();
		if (!rObj)
			return;
		ExportData(json, *&rObj);
		ExportClippingData(json, *&rObj);
		ExportPointToPlaneMeasureData(json, *&rObj);
	}
	ExportLinks(json, object);
}

void DataSerializer::Serialize(nlohmann::json& json, const SafePtr<PointToPipeMeasureNode>& object)
{
	{
		ReadPtr<PointToPipeMeasureNode> rObj = object.cget();
		if (!rObj)
			return;
		ExportData(json, *&rObj);
		ExportPointToPipeMeasureData(json, *&rObj);
	}
	ExportLinks(json, object);
}

void DataSerializer::Serialize(nlohmann::json& json, const SafePtr<PipeToPlaneMeasureNode>& object)
{
	{
		ReadPtr<PipeToPlaneMeasureNode> rObj = object.cget();
		if (!rObj)
			return;
		ExportData(json, *&rObj);
		ExportPipeToPlaneMeasureData(json, *&rObj);
	}
	ExportLinks(json, object);
}

void DataSerializer::Serialize(nlohmann::json& json, const SafePtr<PipeToPipeMeasureNode>& object)
{
	{
		ReadPtr<PipeToPipeMeasureNode> rObj = object.cget();
		if (!rObj)
			return;
		ExportData(json, *&rObj);
		ExportPipeToPipeMeasureData(json, *&rObj);
	}
	ExportLinks(json, object);
}

void DataSerializer::Serialize(nlohmann::json& json, const SafePtr<BeamBendingMeasureNode>& object)
{
	{
		ReadPtr<BeamBendingMeasureNode> rObj = object.cget();
		if (!rObj)
			return;
		ExportData(json, *&rObj);
		ExportBeamBendingMeasureData(json, *&rObj);
	}
	ExportLinks(json, object);
}

void DataSerializer::Serialize(nlohmann::json& json, const SafePtr<ColumnTiltMeasureNode>& object)
{
	{
		ReadPtr<ColumnTiltMeasureNode> rObj = object.cget();
		if (!rObj)
			return;
		ExportData(json, *&rObj);
		ExportColumnTiltMeasureData(json, *&rObj);
	}
	ExportLinks(json, object);
}

void DataSerializer::Serialize(nlohmann::json& json, const SafePtr<ViewPointNode>& object)
{
	{
		ReadPtr<ViewPointNode> rObj = object.cget();
		if (!rObj)
			return;
		ExportData(json, *&rObj);
		ExportTransformationModule(json, *&rObj);
		ExportViewPointData(json, *&rObj);
	}
	ExportLinks(json, object);
}

void DataSerializer::Serialize(nlohmann::json& json, const SafePtr<CameraNode>& object)
{
	{
		ReadPtr<CameraNode> rObj = object.cget();
		if (!rObj)
			return;
		ExportData(json, *&rObj);
		ExportTransformationModule(json, *&rObj);
		ExportRenderingParameters(json, *&rObj);
	}
}

nlohmann::json DataSerializer::Serialize(const UserOrientation& data)
{
	nlohmann::json json;
	json[Key_Id] = data.getId();
	json[Key_Name] = Utils::to_utf8(data.getName().toStdWString());
	json[Key_Order] = data.getOrder();

	json[Key_CustomAxis] = { { data.getCustomAxis()[0].x, data.getCustomAxis()[0].y, data.getCustomAxis()[0].z },
							 { data.getCustomAxis()[1].x, data.getCustomAxis()[1].y, data.getCustomAxis()[1].z } };
	json[Key_AxisType] = magic_enum::enum_name(data.getAxisType());
	json[Key_Point1] = { data.getAxisPoints()[0].x, data.getAxisPoints()[0].y, data.getAxisPoints()[0].z };
	json[Key_Point2] = { data.getAxisPoints()[1].x, data.getAxisPoints()[1].y, data.getAxisPoints()[1].z };

	json[Key_OldPoint] = { data.getOldPoint().x, data.getOldPoint().y, data.getOldPoint().z };
	json[Key_NewPoint] = { data.getNewPoint().x, data.getNewPoint().y, data.getNewPoint().z };

	return json;
}

nlohmann::json DataSerializer::Serialize(const Author& auth)
{
	nlohmann::json jauthor;
	jauthor[Key_Id] = auth.getId();
	jauthor[Key_Name] = Utils::to_utf8(auth.getName());
	return jauthor;
}

nlohmann::json DataSerializer::Serialize(const ProjectInfos& data)
{
	nlohmann::json json;
	{
		ReadPtr<Author> rAuth = data.m_author.cget();
		if(rAuth)
			json[Key_Author] = Serialize(*&rAuth);
	}
	json[Key_Location] = Utils::to_utf8(data.m_location);
	json[Key_Description] = Utils::to_utf8(data.m_description);
	json[Key_Company] = Utils::to_utf8(data.m_company);
	json[Key_BeamBendingTolerance] = data.m_beamBendingTolerance;
	json[Key_ColumnTiltTolerance] = data.m_columnTiltTolerance;
	json[Key_DefaultClipMode] = data.m_defaultClipMode;
	json[Key_DefaultClipDistances] = { data.m_defaultMinClipDistance, data.m_defaultMaxClipDistance };
	json[Key_DefaultRampDistances] = { data.m_defaultMinRampDistance, data.m_defaultMaxRampDistance };
	json[Key_DefaultRampSteps] = data.m_defaultRampSteps;
	json[Key_ImportScanTranslation] = { data.m_importScanTranslation.x, data.m_importScanTranslation.y, data.m_importScanTranslation.z };
	json[Key_Project_Id] = data.m_id;
	json[Key_CustomScanFolderPath] = Utils::to_utf8(data.m_customScanFolderPath.wstring());
	return json;
}

nlohmann::json DataSerializer::Serialize(const sma::TagTemplate& data)
{
	nlohmann::json json;
	json[Key_Name] = Utils::to_utf8(data.getName());
	json[Key_Id] = data.getId();
	json[Key_OriginTemplate] = data.isAOriginTemplate();
	nlohmann::json childrenElem = nlohmann::json::array();
	for (const sma::tField& iterator : data.getFieldsCopy())
	{
		nlohmann::json item;
		item[Key_Name] = Utils::to_utf8(iterator.m_name);
		item[Key_Id] = iterator.m_id.str();
		item[Key_Type] = magic_enum::enum_name<sma::tFieldType>(iterator.m_type);
		xg::Guid id;
		{
			ReadPtr<UserList> rList = iterator.m_fieldReference.cget();
			if (rList)
				id = rList->getId();
		}
		item[Key_Reference] = id;
		item[Key_DefaultValue] = Utils::to_utf8(iterator.m_defaultValue);
		childrenElem.push_back(item);
	}
	json[Key_Fields] = childrenElem;
	return json;
}

nlohmann::json DataSerializer::SerializeStandard(const std::unordered_set<StandardList>& data, StandardType type)
{
	nlohmann::json json;
	json[Key_StandardType] = magic_enum::enum_name<StandardType>(type);
	nlohmann::json childrenElem = nlohmann::json::array();
	for (const StandardList& std : data)
		childrenElem.push_back(DataSerializer::SerializeList<StandardList>(std));
	json[Key_StandardLists] = childrenElem;

	return json;
}