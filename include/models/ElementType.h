#ifndef ELEMENT_TYPE_H
#define ELEMENT_TYPE_H

/*!Type des éléments dans l'arbre

   ElementType est l'enum qui contient tous les diffÃ©rents types d'Ã©lÃ©ments que le projet peut contenir

 */
enum class ElementType
{
	None = 0,
	Data,
	Tag,
	Cluster,
	HierarchyCluster,
	Scan,
	PCO,
	MasterCluster,
	SimpleMeasure,
	PolylineMeasure,
	BeamBendingMeasure,
	ColumnTiltMeasure,
	PointToPlaneMeasure,
	PointToPipeMeasure,
	PipeToPlaneMeasure,
	PipeToPipeMeasure,
	MeshObject,
	Box,
	Point,
	Cylinder,
	Torus,
	Piping,
	Sphere,
	ViewPoint,
	Target
};

/*
enum class ComponentType
{
	GeneralData,
	ClippingData,
	TransformationModuleData,
	GeneralMeasureData,

	ScanData,
	MeshObjectData,
	BeamBendingMeasureData,
	ColumnTiltData,
	ClusterData,
	StandardRadiusData,
	TorusData,
	SphereData,

	PipeToPipeMeasureData,
	PipeToPlanMeasureData,
	PointToPipeMeasureData,

	PointToPlaneMeasureData,

	PolylineMeasureData,
	SimpleMeasureData,

	TagData,
	ViewPointData
};
*/

#endif