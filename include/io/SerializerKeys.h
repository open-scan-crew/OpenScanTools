#ifndef SERIALIZERKEYS_H_
#define Key_SERIALIZERKEYS_H_

/*	Note(Aurélien)
	* Bien vérfier qu'il n'y a pas de doublon ou de proche.
	* Respecter la case.
*/

//SaveLoadSystem
#define Key_OpenScanToolsVersion ">OpenScanToolsVersion"
#define Key_SaveLoadSystemVersion ">SaveLoadSystemVersion"
#define Key_DefaultScanId "DefaultScanId"
#define Key_UserOrientations "UserOrientations"
#define Key_Author "Author"
#define Key_Authors "Authors"
#define Key_Tags "Tags"
#define Key_Objects "Objects"
#define Key_Clusters "Clusters"
#define Key_ViewPoints "Viewpoints"
#define Key_Tree "Tree"
#define Key_HierarchyMasterCluster "HierarchyMasterCluster"

// AGraphNode
#define Key_GeometricParent "GeometricParent"
#define Key_GeometricChildrens "GeometricChildrens"
#define Key_OwningParents "OwningParents"
#define Key_OwningObjectParent "OwningObjectParent"
#define Key_OwningHierarchyParent "OwningHierarchyParent"
#define Key_OwningChildrens "OwningChildrens"

//GenericProperties
#define Key_Type "Type"
#define Key_Name "Name"
#define Key_Description "Description"
#define Key_Identifier "Identifier"
#define Key_Discipline "Discipline"
#define Key_UserId "UserId"
#define Key_ImportTime "ImportTime"
#define Key_ModificationTime "ModificationTime"
#define Key_InternId "InternId"
#define Key_Phase "Phase"
#define Key_ShowHide "ShowHide"
#define Key_Id "Id"

//ClippingProperties
#define Key_ClippingType "ClippingType"
#define Key_ClippingMode "ClippingMode"
#define Key_MinClipDistance "MinClipDist"
#define Key_MaxClipDistance "MaxClipDist"
#define Key_Active "Active"

//RampProperties
#define Key_RampActive "RampActive"
#define Key_MinRampDistance "MinRampDist"
#define Key_MaxRampDistance "MaxRampDist"
#define Key_RampSteps "RampSteps"
#define Key_RampClamped "RampClamped"

//Object3DProperties
#define Key_ColorRGBA "ColorRGBA"
#define Key_Center "Center"
#define Key_Quaternion "Quaternion"
#define Key_Size "Size"

//Tag
#define Key_TemplateId "TemplateId"
#define Key_IconId "IconId"
#define Key_LinkId "LinkId"
#define Key_LinkName "LinkName"
#define Key_LinkURL "LinkURL"
#define Key_Hyperlinks "Hyperlinks"

//Cluster
#define Key_ClusterId "ClusterId"
#define Key_Children "ZChildren"
#define Key_TreeType "TreeType"

//Scan, PCO & meshObject
#define Key_Path "Path"
#define Key_Clippable "Clippable"

//Grid
#define Key_GridType "GridType"
#define Key_Division "Division"

//Cylinder & Sphere
#define Key_DetectedRadius "DetectedRadius"
#define Key_ForcedRadius "ForcedRadius"
#define Key_TubeStandardId "TubeStandardId"
#define Key_DiameterSet "DiameterSet"
#define Key_InsulationRadius "InsulationRadius"

//Torus
#define Key_MainAngle "MainAngle"
#define Key_MainRadius "MainRadius"
#define Key_TubeRadius "TubeRadius"
#define Key_TorusCenter "TorusCenter"

//Piping
#define Key_InternId "InternId"
#define Key_PipingElems "PipingElems"

//MeshObject
#define Key_ObjectName "ObjectName"
#define Key_MeshId "MeshId"

//SimpleMeasure
#define Key_OriginPos "OriginPos"
#define Key_DestPos "DestPos"

//PolylineMeasure
#define Key_Points "Points"

//PointToPlaneMeasure
#define Key_PointToPlaneDist "PointToPlaneDist"
#define Key_Horizontal "Horizontal"
#define Key_Vertical "Vertical"
#define Key_PointCoord "PointCoord"
#define Key_PointOnPlane "PointOnPlane"
#define Key_NormalToPlane "NormalToPlane"
#define Key_ProjPoint "ProjPoint"

//PointToPipeMeasure & PipeToPlaneMeasure
#define Key_PointToAxeDist "PointToAxeDist"
#define Key_PointToAxeHorizontal "PointToAxeHorizontal"
#define Key_PointToAxeVertical "PointToAxeVertical"
#define Key_FreeDist "FreeDist"
#define Key_FreeDistHorizontal "FreeDistHorizontal"
#define Key_FreeDistVertical "FreeDistVertical"
#define Key_TotalFootprint "TotalFootprint"
#define Key_PipeDiameter "PipeDiameter"
#define Key_PipeCenter "PipeCenter"
#define Key_PointCoord "PointCoord"
#define Key_ProjPoint "ProjPoint"
#define Key_PipeCenterToProj "PipeCenterToProj"

//PipeToPlaneMeasure
#define Key_CenterToPlaneDist "CenterToPlaneDist"
#define Key_PlaneCenterHorizontal "PlaneCenterHorizontal"
#define Key_PlaneCenterVertical "PlaneCenterVertical"
#define Key_NormalOnPlane "NormalOnPlane"
#define Key_PointOnPlaneToProj "PointOnPlaneToProj"

//PipeToPipeMeasure
#define Key_CenterP1ToAxeP2 "CenterP1ToAxeP2"
#define Key_P1ToP2Horizontal "P1ToP2Horizontal"
#define Key_P1ToP2Vertical "P1ToP2Vertical"
#define Key_Pipe1Diameter "Pipe1Diameter"
#define Key_Pipe2Diameter "Pipe2Diameter"
#define Key_Pipe1Center "Pipe1Center"
#define Key_Pipe2Center "Pipe2Center"
#define Key_Pipe2CenterToProj "Pipe2CenterToProj"

//BeamBendingMeasure
#define Key_Point1 "Point1"
#define Key_Point2 "Point2"
#define Key_MaxBendingPos "MaxBendingPos"
#define Key_BendingValue "BendingValue"
#define Key_Length "Length"
#define Key_Ratio "Ratio"
#define Key_MaxRatio "MaxRatio"
#define Key_RatioSup "RatioSup"
#define Key_Result "Result"

//ColumnTiltMeasure
#define Key_BottomPoint "BottomPoint"
#define Key_TiltValue "TiltValue"
#define Key_TopPoint "TopPoint"
#define Key_Height "Height"

//UserOrientation
#define Key_AxisType "AxisTypeEnum"
#define Key_CustomAxis "CustomAxis"
#define Key_Order "Order"
#define Key_OldPoint "OldPoint"
#define Key_NewPoint "NewPoint"

//ProjectInfos
#define Key_Company "Company"
#define Key_Location "Location"
#define Key_BeamBendingTolerance "BeamBendingTolerance"
#define Key_ColumnTiltTolerance "ColumnTiltTolerance"
#define Key_DefaultClipMode "DefaultClipMode"
#define Key_DefaultClipDistances "DefaultClipDistances"
#define Key_DefaultRampDistances "DefaultRampDistances"
#define Key_DefaultRampSteps "DefaultRampSteps"
#define Key_ImportScanTranslation "ImportScanTranslation"
#define Key_Project_Id "ProjectId"
#define Key_CustomScanFolderPath "CustomScanFolderPath"

//Template
#define Key_OriginTemplate "OriginTemplate"
#define Key_Fields "Fields"
#define Key_Reference "Reference"
#define Key_DefaultValue "DefaultValue"
#define Key_Templates "Templates"

//Viewpoint
#define Key_Display_Parameters			"DisplayParameters" // For retro-compatibility
#define Key_ViewPoint					"ViewPoint"
#define Key_Projection_Mode				"ProjectionMode"
#define Key_Projection_Box				"ProjectionBox"
#define Key_Rendering_Mode				"RenderingMode"
#define Key_Background_Color			"BackgroundColor"
#define Key_Point_Size					"PointSize"
#define Key_Delta_Filling				"DeltaFilling"
#define Key_Gap_Filling_Texel_Threshold	"GapFillingTexelThreshold"

#define Key_Contrast					"Contrast"
#define Key_Brightness					"Brightness"
#define Key_Saturation					"Saturation"
#define Key_Luminance					"Luminance"
#define Key_Blending					"Blending"
#define Key_Flat_Color					"FlatColor"

#define Key_DistRamp					"DistanceRamp"
#define Key_DistRampSteps				"DistanceRampSteps"

#define Key_Blend_Mode					"BlendMode"
#define Key_NegativeEffect				"NegativeEffect"
#define Key_ReduceFlash					"ReduceFlash"
#define Key_Transparency				"Transparency"

#define Key_Post_Rendering_Normals		"PostRenderingNormals"
#define Key_Edge_Aware_Blur			"EdgeAwareBlur"
#define Key_Depth_Lining			"DepthLining"
#define Key_Billboard_Rendering		"BillboardRendering"
#define Key_Eye_Dome_Lighting			"EyeDomeLighting"
#define Key_Display_Guizmo				"DisplayGuizmo"
#define Key_Ramp_Scale_Options			"RampScaleOptions"

#define Key_Alpha_Object				"AlphaObject"
#define Key_Distance_Unit				"DistanceUnit"
#define Key_Diameter_Unit				"DiameterUnit"
#define Key_Volume_Unit					"VolumeUnit"
#define Key_Displayed_Digits			"DisplayedDigits"
#define Key_Measure_Show_Mask			"MeasureShowMask"
#define Key_Marker_Show_Mask			"MarkerShowMask"
#define Key_Marker_Rendering_Parameters	"MarkerRenderingParameters"
#define Key_Text_Display_Options		"MarkerTextDisplayParameters" // The struct name changed
#define Key_Display_All_Marker_Texts	"DisplayAllMarkerTexts"
#define Key_Display_All_Measures		"DisplayAllMeasures"

#define Key_Active_Scans				"ActiveScans"
#define Key_Active_Clippings			"ActiveClippings"
#define Key_Interior_Clippings			"InteriorClippings"
#define Key_Active_Ramps				"ActiveRamps"
#define Key_Visible_Objects				"VisibleObjects"
#define Key_Objects_Colors				"ObjectsColors"
#define Key_Ortho_Grid_Active			"OrthoGridActive"
#define Key_Ortho_Grid_Color			"OrthoGridColor"
#define Key_Ortho_Grid_Step				"OrthoGridStep"
#define Key_Ortho_Grid_Linewidth		"OrthoGridLinewidth"

/*** EXTRA KEY ***/

//ExportLists
#define Key_Origin "Origin"
#define Key_Elements "Elements"
#define Key_Lists "Lists"
#define Key_Standards "Standards"
#define Key_StandardType "StandardType"
#define Key_StandardLists "StandardLists"

#endif // !SERIALIZERKEYS_H_