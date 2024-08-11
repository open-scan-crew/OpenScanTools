#include "io/DataEditor.h"
#include "io/SerializerKeys.h"
#include <magic_enum/magic_enum.hpp>
#include "models/application/TagTemplate.h"

/*
void DataEditor::EditGenericProperties(const Data& input, Data& output, const std::unordered_set<std::string> carac)
{
	for (std::string i : carac)
	{
		if (i == Key_Name)
		{
			output.setName(input.getName());
		}
		if (i == Key_Description)
		{
			output.setDescription(input.getDescription());
		}
		if (i == Key_Identifier)
		{
			output.setIdentifier(input.getIdentifier());
		}
		if (i == Key_Discipline)
		{
			output.setDiscipline(input.getDiscipline());
		}
		if (i == Key_UserId)
		{
			output.setUserIndex(input.getUserIndex());
		}
		if (i == Key_ImportTime)
		{
			output.setCreationTime(input.getCreationTime());
		}
		if (i == Key_ModificationTime)
		{
			output.setModificationTime(input.getModificationTime());
		}
		if (i == Key_Id)
		{
			output.setId(input.getId());
		}
		if (i == Key_Phase)
		{
			output.setPhase(input.getPhase());
		}
		if (i == Key_ShowHide)
		{
			output.setVisible(input.isVisible());
		}
		if (i == Key_Author)
		{
			output.setAuthorId(input.getAuthorId());
		}
		if (i == Key_Hyperlinks)
		{
			output.setHyperlinks(input.getHyperlinks());
		}
	}
}

void DataEditor::EditClippingProperties(const Clipping& input, Clipping& output, const std::unordered_set<std::string> carac)
{
	for (std::string i : carac)
	{
		if (i == Key_Active)
		{
			if (input.isClippingActive() == true)
			{
				output.setClippingActive(true);
			}
			else {
				output.setClippingActive(false);
			}
		}
		if (i == Key_ClippingMode)
		{
			output.setClippingMode(input.getClippingMode());
		}
	}
}

void DataEditor::EditObject3DProperties(const Object3D& input, Object3D& output, const std::unordered_set<std::string> carac)
{
	for (std::string i : carac)
	{
		if (i == Key_ColorRGBA)
		{
			output.setColor(input.getColor());
		}
		if (i == Key_Center)
		{
			output.setPosition(input.getCenter());
		}
		if (i == Key_Quaternion)
		{
			output.setRotation(input.getOrientation());
		}
		if (i == Key_Size)
		{
			output.setSize(input.getSize());
		}
	}
}

/*
void DataEditor::Edit(const Tag& input, Tag& output, const std::unordered_set<std::string> carac)
{
	EditClippingProperties(input, output,carac);
	EditObject3DProperties(input, output,carac);
	EditGenericProperties(input, output,carac);

	for (std::string i : carac)
	{
		if (i == Key_TemplateId)
		{
			output.setTemplateId(input.getTemplateId());
		}
		if (i == Key_IconId)
		{
			output.setMarkerIcon(input.getMarkerIcon());
		}
		if (i == Key_Fields)
		{
			output.setFields(input.getFields());
		}
	}
}

void DataEditor::Edit(const Cluster& input, Cluster& output, const std::unordered_set<std::string> carac)
{
	EditObject3DProperties(input, output, carac);
	EditGenericProperties(input, output, carac);
}

void DataEditor::Edit(const MasterCluster& input, MasterCluster& output, const std::unordered_set<std::string> carac)
{
	EditGenericProperties(input, output, carac);
}

void DataEditor::Edit(const Scan& input, Scan& output, const std::unordered_set<std::string> carac)
{
	EditObject3DProperties(input, output,carac);
	EditGenericProperties(input,output,carac);
	
	for (std::string i : carac)
	{
		if (i == Key_Path)
		{
			output.setScanPath(input.getScanPath());
		}
		if (i == Key_Clippable)
		{
			output.setClippable(input.getClippable());
		}
	}
}

void DataEditor::Edit(const Box& input, Box& output, const std::unordered_set<std::string> carac)
{
	EditClippingProperties(input, output,carac);
	EditObject3DProperties(input, output, carac);
	EditGenericProperties(input, output, carac);
}

void DataEditor::Edit(const Grid& input, Grid& output, const std::unordered_set<std::string> carac)
{
	EditClippingProperties(input, output, carac);
	EditObject3DProperties(input, output, carac);
	EditGenericProperties(input, output, carac);

	for (std::string i : carac)
	{
		if (i == Key_GridType)
		{
			output.setGridType(input.getGridType());
		}
		if (i == Key_Division)
		{
			output.setGridDivision(input.getGridDivision());
		}
	}
}

void DataEditor::Edit(const Cylinder& input, Cylinder& output, const std::unordered_set<std::string> carac)
{
	EditClippingProperties(input, output, carac);
	EditObject3DProperties(input, output, carac);
	EditGenericProperties(input, output, carac);
	
	for (std::string i : carac)
	{
		if (i == Key_DetectedRadius)
		{
			output.setDetectedRadius(input.getDetectedRadius());
		}
		if (i == Key_TubeStandardId)
		{
			output.setStandard(input.getStandard());
		}
		if (i == Key_DiameterSet)
		{
			output.setDiameterSet(input.getDiameterSet());
		}
	}
}

void DataEditor::Edit(const Torus& input, Torus& output, const std::unordered_set<std::string> carac)
{
	EditClippingProperties(input, output, carac);
	EditObject3DProperties(input, output, carac);
	EditGenericProperties(input, output, carac);

	for (std::string i : carac)
	{
		if (i == Key_MainAngle)
		{
			output.setMainAngle(input.getMainAngle());
		}
		if (i == Key_MainRadius)
		{
			output.setMainRadius(output.getMainRadius());
		}
		if (i == Key_TubeRadius)
		{
			output.setTubeRadius(input.getTubeRadius());
		}
		if (i == Key_TorusCenter)
		{
			output.setTorusCenter(input.getTorusCenter());
		}
	}
}

void DataEditor::Edit(const Piping& input, Piping& output, const std::unordered_set<std::string> carac)
{
	EditGenericProperties(input, output, carac);

	for (std::string i : carac)
	{
		if (i == Key_PipingElems)
		{
			output.setPipingList(input.getPipingList());
		}
	}
}

void DataEditor::Edit(const Sphere& input, Sphere& output, const std::unordered_set<std::string> carac)
{
	EditClippingProperties(input, output, carac);
	EditObject3DProperties(input, output, carac);
	EditGenericProperties(input, output, carac);

	for (std::string i : carac)
	{
		if (i == Key_DetectedRadius)
		{
			output.setDetectedRadius(input.getDetectedRadius());
		}
		if (i == Key_DiameterSet)
		{
			output.setDiameterSet(input.getDiameterSet());
		}
	}
}

void DataEditor::Edit(const Point& input, Point& output, const std::unordered_set<std::string> carac)
{
	EditClippingProperties(input, output, carac);
	EditObject3DProperties(input, output, carac);
	EditGenericProperties(input, output, carac);
}

void DataEditor::Edit(const PCObject& input, PCObject& output, const std::unordered_set<std::string> carac)
{
	EditClippingProperties(input, output, carac);
	EditObject3DProperties(input, output, carac);
	EditGenericProperties(input, output, carac);

	for (std::string i : carac)
	{
		if (i == Key_Path)
		{
			output.setScanPath(input.getScanPath());
		}
		if (i == Key_ClippingType)
		{
			output.setClippingType(input.getClippingType());
		}
		if (i == Key_Clippable)
		{
			output.setClippable(input.getClippable());
		}

	}
}

void DataEditor::Edit(const MeshObject& input, MeshObject& output, const std::unordered_set<std::string> carac)
{
	EditObject3DProperties(input, output, carac);
	EditGenericProperties(input, output, carac);

	for (std::string i : carac)
	{
		if (i == Key_Path)
		{
			output.setFilePath(input.getFilePath());
		}
		if (i == Key_ObjectName)
		{
			output.setObjectName(input.getObjectName());
		}
		if (i == Key_MeshId)
		{
			output.setMeshId(input.getMeshId());
		}
	}
}

void DataEditor::Edit(const SimpleMeasure& input, SimpleMeasure& output, const std::unordered_set<std::string> carac)
{
	EditClippingProperties(input, output, carac);
	EditGenericProperties(input, output, carac);

	for (std::string i : carac)
	{
		if (i == Key_OriginPos)
		{
			output.setOriginPos(input.getOriginPos());
		}
		if (i == Key_DestPos)
		{
			output.setDestinationPos(input.getDestinationPos());
		}
	}
}

void DataEditor::Edit(const PolylineMeasure& input, PolylineMeasure& output, const std::unordered_set<std::string> carac)
{
	EditClippingProperties(input, output, carac);
	EditGenericProperties(input, output, carac);

	for (std::string i : carac)
	{
		if (i == Key_Points)
		{
			if (!input.getMeasures().empty())
			{
				output.setMeasures(input.getMeasures());
			}
		}
	}
}

void DataEditor::Edit(const PointToPlaneMeasure& input, PointToPlaneMeasure& output, const std::unordered_set<std::string> carac)
{
	EditGenericProperties(input, output, carac);

	for (std::string i : carac)
	{
		if (i == Key_PointToPlaneDist)
		{
			output.setPointToPlaneD(input.getPointProjToPlaneD());
		}
		if (i == Key_Horizontal)
		{
			output.setHorizontal(input.getHorizontal());
		}
		if (i == Key_Vertical)
		{
			output.setVertical(input.getVertical());
		}
		if (i == Key_PointCoord)
		{
			output.setPointCoord(input.getPointCoord());
		}
		if (i == Key_PointOnPlane)
		{
			output.setpointOnPlane(input.getPointOnPlane());
		}
		if (i == Key_NormalToPlane)
		{
			output.setNormalToPlane(input.getNormalToPlane());
		}
		if (i == Key_ProjPoint)
		{
			output.setProjPoint(input.getProjPoint());
		}
	}
}

void DataEditor::Edit(const PointToPipeMeasure& input, PointToPipeMeasure& output, const std::unordered_set<std::string> carac)
{
	EditGenericProperties(input, output, carac);

	for (std::string i : carac)
	{
		if (i == Key_PointToAxeDist)
		{
			output.setPointToAxeDist(input.getPointToAxeDist());
		}
		if (i == Key_PointToAxeHorizontal)
		{
			output.setPointToAxeHorizontal(input.getPointToAxeHorizontal());
		}
		if (i == Key_PointToAxeVertical)
		{
			output.setPointToAxeVertical(input.getPointToAxeVertical());
		}
		if (i == Key_FreeDist)
		{
			output.setFreeD(input.getFreeDist());
		}
		if (i == Key_FreeDistHorizontal)
		{
			output.setFreeDistHorizontal(input.getFreeDistHorizontal());
		}
		if (i == Key_FreeDistVertical)
		{
			output.setFreeDistVertical(input.getFreeDistVertical());
		}
		if (i == Key_TotalFootprint)
		{
			output.setTotalFootprint(input.getTotalFootprint());
		}
		if (i == Key_PipeDiameter)
		{
			output.setPipeDiameter(input.getPipeDiameter());
		}
		if (i == Key_PipeCenter)
		{
			output.setPipeCenter(input.getPipeCenter());
		}
		if (i == Key_PointCoord)
		{
			output.setPointCoord(input.getPointCoord());
		}
		if (i == Key_ProjPoint)
		{
			output.setProjPoint(input.getProjPoint());
		}
		if (i == Key_PipeCenterToProj)
		{
			output.setPipeCenterToProj(input.getPipeCenterToProj());
		}

	}
}

void DataEditor::Edit(const PipeToPlaneMeasure& input, PipeToPlaneMeasure& output, const std::unordered_set<std::string> carac)
{
	EditGenericProperties(input, output, carac);

	for (std::string i : carac)
	{
		if (i == Key_CenterToPlaneDist)
		{
			output.setCenterToPlaneDist(input.getCenterToPlaneDist());
		}
		if (i == Key_PlaneCenterHorizontal)
		{
			output.setPlaneCenterHorizontal(input.getPlaneCenterHorizontal());
		}
		if (i == Key_PlaneCenterVertical)
		{
			output.setPlaneCenterVertical(input.getPlaneCenterHorizontal());
		}
		if (i == Key_FreeDist)
		{
			output.setFreeDist(input.getFreeDist());
		}
		if (i == Key_FreeDistHorizontal)
		{
			output.setFreeDistHorizontal(input.getFreeDistHorizontal());
		}
		if (i == Key_FreeDistVertical)
		{
			output.setFreeDistVertical(input.getFreeDistVertical());
		}
		if (i == Key_TotalFootprint)
		{
			output.setTotalFootprint(input.getTotalFootprint());
		}
		if (i == Key_PipeDiameter)
		{
			output.setPipeDiameter(input.getPipeDiameter());
		}
		if (i == Key_PipeCenter)
		{
			output.setPipeCenter(input.getPipeCenter());
		}
		if (i == Key_NormalOnPlane)
		{
			output.setNormalOnPlane(input.getNormalOnPlane());
		}
		if (i == Key_ProjPoint)
		{
			output.setProjPoint(input.getProjPoint());
		}
		if (i == Key_PointOnPlaneToProj)
		{
			output.setPointOnPlaneToProj(input.getPointOnPlaneToProj());
		}
		if (i == Key_PointOnPlane)
		{
			output.setPointOnPlane(input.getPointOnPlane());
		}
	}
}

void DataEditor::Edit(const PipeToPipeMeasure& input, PipeToPipeMeasure& output, const std::unordered_set<std::string> carac)
{
	EditGenericProperties(input, output, carac);

	for (std::string i : carac)
	{
		if (i == Key_CenterP1ToAxeP2)
		{
			output.setCenterP1ToAxeP2(input.getCenterP1ToAxeP2());
		}
		if (i == Key_P1ToP2Horizontal)
		{
			output.setP1ToP2Horizontal(input.getP1ToP2Horizontal());
		}
		if (i == Key_P1ToP2Vertical)
		{
			output.setP1ToP2Vertical(input.getP1ToP2Vertical());
		}
		if (i == Key_FreeDist)
		{
			output.setFreeDist(input.getFreeDist());
		}
		if (i == Key_FreeDistHorizontal)
		{
			output.setFreeDistHorizontal(input.getFreeDistHorizontal());
		}
		if (i == Key_FreeDistVertical)
		{
			output.setFreeDistVertical(input.getFreeDistVertical());
		}
		if (i == Key_TotalFootprint)
		{
			output.setTotalFootprint(input.getTotalFootprint());
		}
		if (i == Key_Pipe2Diameter)
		{
			output.setPipe2Diameter(input.getPipe2Diameter());
		}
		if (i == Key_Pipe1Diameter)
		{
			output.setPipe1Diameter(input.getPipe1Diameter());
		}
		if (i == Key_Pipe1Center)
		{
			output.setPipe1Center(input.getPipe1Center());
		}
		if (i == Key_Pipe2Center)
		{
			output.setPipe2Center(input.getPipe2Center());
		}
		if (i == Key_ProjPoint)
		{
			output.setProjPoint(input.getProjPoint());
		}
		if (i == Key_Pipe2CenterToProj)
		{
			output.setPipe2CenterToProj(input.getPipe2CenterToProj());
		}
	}
}

void DataEditor::Edit(const BeamBendingMeasure& input, BeamBendingMeasure& output, const std::unordered_set<std::string> carac)
{
	EditGenericProperties(input, output, carac);

	for (std::string i : carac)
	{
		if (i == Key_Point1)
		{
			output.setPoint1Pos(input.getPoint1Pos());
		}
		if (i == Key_Point2)
		{
			output.setPoint2Pos(input.getPoint2Pos());
		}
		if (i == Key_MaxBendingPos)
		{
			output.setMaxBendingPos(input.getMaxBendingPos());
		}
		if (i == Key_BendingValue)
		{
			output.setBendingValue(input.getBendingValue());
		}
		if (i == Key_Length)
		{
			output.setLength(input.getLength());
		}
		if (i == Key_MaxRatio)
		{
			output.setMaxRatio(input.getMaxRatio());
		}
		if (i == Key_Ratio)
		{
			output.setRatio(input.getRatio());
		}
		if (i == Key_RatioSup)
		{
			output.setRatioSup(input.getRatioSup());
		}
		if (i == Key_Result)
		{
			output.setResultReliability(input.getResult());
		}
		if (i == Key_ColorRGBA)
		{
			output.setColor(input.getColor());
		}
	}
}

void DataEditor::Edit(const ColumnTiltMeasure& input, ColumnTiltMeasure& output, const std::unordered_set<std::string> carac)
{
	EditGenericProperties(input, output, carac);

	for (std::string i : carac)
	{
		if (i == Key_Point1)
		{
			output.setPoint1(input.getPoint1());
		}
		if (i == Key_Point2)
		{
			output.setPoint2(input.getPoint2());
		}
		if (i == Key_BottomPoint)
		{
			output.setBottomPoint(input.getBottomPoint());
		}
		if (i == Key_TopPoint)
		{
			output.setTopPoint(input.getTopPoint());
		}
		if (i == Key_TiltValue)
		{
			output.setTiltValue(input.getTiltValue());
		}
		if (i == Key_Height)
		{
			output.setHeight(input.getHeight());
		}
		if (i == Key_Ratio)
		{
			output.setRatio(input.getRatio());
		}
		if (i == Key_RatioSup)
		{
			output.setRatioSup(input.getRatioSup());
		}
		if (i == Key_MaxRatio)
		{
			output.setMaxRatio(input.getMaxRatio());
		}
		if (i == Key_Result)
		{
			output.setResultReliability(input.getResultReliability());
		}
		if (i == Key_ColorRGBA)
		{
			output.setColor(input.getColor());
		}
	}
}

void DataEditor::Edit(const UserOrientation& input, UserOrientation& output, const std::unordered_set<std::string> carac)
{
	for (std::string i : carac)
	{
		if (i == Key_Id)
		{
			output.setId(input.getId());
		}
		if (i == Key_Name)
		{
			output.setName(input.getName());
		}
		if (i == Key_Order)
		{
			output.setOrder(input.getOrder());
		}
		if (i == Key_CustomAxis)
		{
			output.setCustomAxis(input.getCustomAxis());
		}
		if (i == Key_AxisType)
		{
			output.setAxisType(input.getAxisType());
		}
		if (i == Key_Point1)
		{
			output.setPoint1(input.getAxisPoints()[0]);
		}
		if (i == Key_Point2)
		{
			output.setPoint2(input.getAxisPoints()[1]);
		}
		if (i == Key_OldPoint)
		{
			output.setOldPoint(input.getOldPoint());
		}
		if (i == Key_NewPoint)
		{
			output.setNewPoint(input.getNewPoint());
		}
	}
}

void DataEditor::Edit(const ViewPoint& input, ViewPoint& output, const std::unordered_set<std::string> carac)
{
	for (std::string i : carac)
	{
		if (i == Key_Projection_Mode)
		{
			output.setProjectionMode(input.getProjectionMode());
		}
		if (i == Key_Projection_Box)
		{
			output.setProjectionFrustum(input.getProjectionFrustum());
		}
		if (i == Key_Rendering_Mode)
		{
			output.m_mode = input.m_mode;
		}
		if (i == Key_Blend_Mode)
		{
			output.m_blendMode = input.m_blendMode;
		}
		if (i == Key_Alpha_Object)
		{
			output.m_transparency = input.m_transparency;
		}
		if (i == Key_NegativeEffect)
		{
			output.m_negativeEffect = input.m_negativeEffect;
		}
		if (i == Key_Transparency)
		{
			output.m_transparency = input.m_transparency;
		}
		if (i == Key_Ramp)
		{
			output.m_rampMin = input.m_rampMin;
			output.m_rampMax = input.m_rampMax;
			output.m_rampStep = input.m_rampStep;
		}
		if (i == Key_Contrast)
		{
			output.m_contrast = input.m_contrast;
		}
		if (i == Key_Brightness)
		{
			output.m_brightness = input.m_brightness;
		}
		if (i == Key_Saturation)
		{
			output.m_saturation = input.m_saturation;
		}
		if (i == Key_Luminance)
		{
			output.m_luminance = input.m_luminance;
		}
		if (i == Key_Blending)
		{
			output.m_hue = input.m_hue;
		}
		if (i == Key_Flat_Color)
		{
			output.m_flatColor = input.m_flatColor;
		}
		if (i == Key_Background_Color)
		{
			output.m_backgroundColor = input.m_backgroundColor;
		}
		if (i == Key_Point_Size)
		{
			output.m_pointSize = input.m_pointSize;
		}
		if (i == Key_Display_Parameters)
		{
			output.m_valueDisplayParameters.distanceUnit = input.getDisplayParameters().m_valueDisplayParameters.distanceUnit;
			output.m_valueDisplayParameters.diameterUnit = input.getDisplayParameters().m_valueDisplayParameters.diameterUnit;
			output.m_valueDisplayParameters.displayedDigits = input.getDisplayParameters().m_valueDisplayParameters.displayedDigits;
		}
		if (i == Key_Measure_Show_Mask)
		{
			output.m_measureShowMask = input.m_measureShowMask;
		}
		if (i == Key_Marker_Text_Parameters)
		{
			output.m_markerTextParameters.m_textFilter = input.getDisplayParameters().m_markerTextParameters.m_textFilter;
			output.m_markerTextParameters.m_textTheme = input.getDisplayParameters().m_markerTextParameters.m_textTheme;
			output.m_markerTextParameters.m_textFontSize = input.getDisplayParameters().m_markerTextParameters.m_textFontSize;
		}
		if (i == Key_Display_All_Marker_Texts)
		{
			output.m_displayAllMarkersTexts = input.m_displayAllMarkersTexts;
		}
		if (i == Key_Display_All_Measures)
		{
			output.m_displayAllMeasures = input.m_displayAllMeasures;
		}
		if (i == Key_Marker_Rendering_Parameters)
		{
			output.m_mkRenderingParams.improveVisibility = input.getDisplayParameters().m_mkRenderingParams.improveVisibility;
			output.m_mkRenderingParams.maximumDisplayDistance = input.getDisplayParameters().m_mkRenderingParams.maximumDisplayDistance;
			output.m_mkRenderingParams.nearLimit = input.getDisplayParameters().m_mkRenderingParams.nearLimit;
			output.m_mkRenderingParams.farLimit = input.getDisplayParameters().m_mkRenderingParams.farLimit;
			output.m_mkRenderingParams.nearSize = input.getDisplayParameters().m_mkRenderingParams.nearSize;
			output.m_mkRenderingParams.farSize = input.getDisplayParameters().m_mkRenderingParams.farSize;
		}
		if (i == Key_Post_Rendering_Normals)
		{
			output.m_postRenderingNormals.show = input.getDisplayParameters().m_postRenderingNormals.show;
			output.m_postRenderingNormals.inverseTone = input.getDisplayParameters().m_postRenderingNormals.inverseTone;
			output.m_postRenderingNormals.blendColor = input.getDisplayParameters().m_postRenderingNormals.blendColor;
			output.m_postRenderingNormals.normalStrength = input.getDisplayParameters().m_postRenderingNormals.normalStrength;
			output.m_postRenderingNormals.gloss = input.getDisplayParameters().m_postRenderingNormals.gloss;
		}
		if (i == Key_Display_Guizmo)
		{
			output.m_displayGizmo = input.m_displayGizmo;
		}
		if (i == Key_Active_Clippings)
		{
			output.setActiveClippings(input.getActiveClippings());
		}
		if (i == Key_Interior_Clippings)
		{
			output.setInteriorClippings(input.getInteriorClippings());
		}
		if (i == Key_Visible_Objects)
		{
			output.setVisibleObjects(input.getVisibleObjects());
		}
		if (i == Key_Objects_Colors)
		{
			output.setScanClusterColors(input.getScanClusterColors());
		}
	}
}

void DataEditor::Edit(const ProjectInfos& input, ProjectInfos& output, const std::unordered_set<std::string> carac)
{

	for (std::string i : carac)
	{
		if (i == Key_Company)
		{
			output.m_company = input.m_company;
		}
		if (i == Key_Author)
		{
			output.m_author = input.m_author;
		}
		if (i == Key_Location)
		{
			output.m_location = input.m_location;
		}
		if (i == Key_Description)
		{
			output.m_description = input.m_description;
		}
		if (i == Key_BeamBendingTolerance)
		{
			output.m_beamBendingTolerance = input.m_beamBendingTolerance;
		}
		if (i == Key_ColumnTiltTolerance)
		{
			output.m_columnTiltTolerance = input.m_columnTiltTolerance;
		}
		if (i == Key_PipesUserClippingValue)
		{
			output.m_pipesUserClippingValue = input.m_pipesUserClippingValue;
		}
		if (i == Key_PlanesUserClippingValue)
		{
			output.m_pointsUserClippingValue = input.m_pointsUserClippingValue;
		}
		if (i == Key_PointsUserClippingValue)
		{
			output.m_pointsUserClippingValue = input.m_pointsUserClippingValue;
		}
		if (i == Key_LinesUserClippingValue)
		{
			output.m_linesUserClippingValue = input.m_linesUserClippingValue;
		}
		if (i == Key_SpheresUserClippingValue)
		{
			output.m_spheresUserClippingValue = input.m_spheresUserClippingValue;
		}
		if (i == Key_ImportScanTranslation)
		{
			output.m_importScanTranslation = input.m_importScanTranslation;
		}
	}
}

/*
void DataEditor::Edit(const sma::TagTemplate& input, sma::TagTemplate& output, const std::unordered_set<std::string> carac)
{
	for (std::string i : carac)
	{
		if (i == Key_Id)
		{
			output.setId(input.getId());
		}
		if (i == Key_OriginTemplate)
		{
			output.setOriginTemplate(input.isAOriginTemplate());
		}
		if (i == Key_Name)
		{
			output.renameTagTemplate(input.getName());
		}
		if (i == Key_Fields)
		{
			output.clearFields();
			for (auto j : input.getFieldsCopy())
			{
				sma::tField field;
				field.m_id = j.m_id;
				field.m_name = j.m_name;
				field.m_type = j.m_type;
				field.m_fieldReference = j.m_fieldReference;
				field.m_defaultValue = j.m_defaultValue;
				output.addNewField(field);
			}
		}
	}
}
*/