#ifndef FILTER_TYPES_HPP
#define FILTER_TYPES_HPP

#include "models/ElementType.h"

#include <unordered_set>

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
	ElementType::Cylinder,
	ElementType::Torus,
	ElementType::Point,
	ElementType::Tag ,
	ElementType::SimpleMeasure,
	ElementType::PolylineMeasure,
	ElementType::Sphere
};

#endif