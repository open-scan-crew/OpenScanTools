#ifndef TYPES_HPP
#define TYPES_HPP

#include <unordered_set>
/*!Type des éléments dans l'arbre

   ElementType est l'enum qui contient tous les différents types d'éléments que le projet peut contenir

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
	Grid,
	Point,
	Cylinder,
	Torus,
	Piping,
	Sphere,
	ViewPoint
};
/*! Type d'un TreeSystem 
	
	Notre arborescence est constituée de sous-arbres ayant chacun un unique type TreeType.
	(Scan,TagTree,PointTree....)

	*/
enum class TreeType
{
	RawData = 0,
	Scan,
	User,
	Hierarchy,
	Measures,
	Boxes,
	Tags,
	MeshObjects,
	Pco,
	Pipe,
	Point,
	Sphere,
	Piping,
	ViewPoint,
	MAXENUM
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

static const std::unordered_set<ElementType> s_allTypes = {
	ElementType::PolylineMeasure,
	ElementType::PointToPlaneMeasure,
	ElementType::PointToPipeMeasure,
	ElementType::PipeToPlaneMeasure,
	ElementType::PipeToPipeMeasure,
	ElementType::BeamBendingMeasure,
	ElementType::ColumnTiltMeasure,
	ElementType::MeshObject,
	ElementType::ViewPoint,
	ElementType::Scan,
	ElementType::PCO,
	ElementType::Box,
	ElementType::Grid,
	ElementType::Cylinder,
	ElementType::Torus,
	ElementType::Point,
	ElementType::Tag,
	ElementType::SimpleMeasure,
	ElementType::PolylineMeasure,
	ElementType::Sphere
};

static const std::unordered_set<ElementType> s_clippingTypes = {
    ElementType::Box,
    ElementType::Grid,
    ElementType::Cylinder,
    ElementType::Torus,
    ElementType::Point,
    ElementType::Tag ,
    ElementType::SimpleMeasure,
    ElementType::PolylineMeasure,
    ElementType::Sphere
};

static const std::unordered_set<ElementType> s_measuresTypes = {
	ElementType::PolylineMeasure,
	ElementType::PointToPlaneMeasure,
	ElementType::PointToPipeMeasure,
	ElementType::PipeToPlaneMeasure,
	ElementType::PipeToPipeMeasure,
	ElementType::SimpleMeasure
};

/*
static std::unordered_set<ElementType> s_manipulableTypes = {
	ElementType::Box,
	ElementType::Grid,
	ElementType::Cylinder,
	ElementType::Torus,
	ElementType::Sphere,
	ElementType::PCO,
	ElementType::MeshObject
};
*/
#endif