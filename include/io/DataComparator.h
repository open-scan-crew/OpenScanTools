#ifndef DATACOMPARATOR_H
#define DATACOMPARATOR_H


#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>

#include "models/project/ProjectInfos.h"
#include "models/application/UserOrientation.h"
#include "models/data/clipping/ClippingGeometry.h"
#include "models/Types.hpp"
#include "models/project/ProjectTypes.h"
#include "utils/IdGiver.hpp"
#include "tls_def.h"
#include "models/application/TagTemplate.h"
#include <nlohmannJson/json.hpp>
#include <magic_enum/magic_enum.hpp>
#include "io/SerializerKeys.h"
#include "models/application/UserOrientation.h"
#include "models/project/ProjectInfos.h"

//new
namespace DataComparator
{
	//std::set<std::string> Compare(const UITag& data1, const UITag& data2);
	/*std::set<std::string> Compare(const UICluster& data1, const UICluster& data2);
	std::set<std::string> Compare(const UIMasterCluster& data1, const UIMasterCluster& data2);
	std::set<std::string> Compare(const UIScan& data1, const UIScan& data2);
	std::set<std::string> Compare(const UIBox& data1, const UIBox& data2);
	std::set<std::string> Compare(const UIGrid& data1, const UIGrid& data2);
	std::set<std::string> Compare(const UICylinder& data1, const UICylinder& data2);
	std::set<std::string> Compare(const UITorus& data1, const UITorus& data2);
	std::set<std::string> Compare(const UIPiping& data1, const UIPiping& data2);
	std::set<std::string> Compare(const UISphere& data1, const UISphere& data2);
	std::set<std::string> Compare(const UIPoint& data1, const UIPoint& data2);
	std::set<std::string> Compare(const UIPCObject& data1, const UIPCObject& data2);
	std::set<std::string> Compare(const UIMeshObject& data1, const UIMeshObject& data2);
	std::set<std::string> Compare(const UISimpleMeasure& data1, const UISimpleMeasure& data2);
	std::set<std::string> Compare(const UIPolylineMeasure& data1, const UIPolylineMeasure& data2);
	std::set<std::string> Compare(const UIPointToPlaneMeasure& data1, const UIPointToPlaneMeasure& data2);
	std::set<std::string> Compare(const UIPointToPipeMeasure& data1, const UIPointToPipeMeasure& data2);
	std::set<std::string> Compare(const UIPipeToPlaneMeasure& data1, const UIPipeToPlaneMeasure& data2);
	std::set<std::string> Compare(const UIPipeToPipeMeasure& data1, const UIPipeToPipeMeasure& data2);
	std::set<std::string> Compare(const UIBeamBendingMeasure& data1, const UIBeamBendingMeasure& data2);
	std::set<std::string> Compare(const UIColumnTiltMeasure& data1, const UIColumnTiltMeasure& data2);
	std::set<std::string> Compare(const UserOrientation& data1, const UserOrientation& data2);
	std::set<std::string> Compare(const UIViewPoint& data1, const UIViewPoint& data2);
	std::set<std::string> Compare(const ProjectInfos& data1, const ProjectInfos& data2);
	std::set<std::string> Compare(const sma::TagTemplate& data1, const sma::TagTemplate& data2);
	std::set<std::string> CompareGenericProperties(const Data& d1, const Data& d2);
	std::set<std::string> CompareClippingProperties(const UIClipping& d1, const UIClipping& d2);
	std::set<std::string> CompareObject3DProperties(const UIObject3D& d1, const UIObject3D& d2);*/
}
#endif // !DATASERIALIZER_H_ 
