#ifndef DATASERIALIZER_H_
#define DATASERIALIZER_H_

#include <nlohmannJson/json.hpp>
#include <magic_enum/magic_enum.hpp>
#include <unordered_set>

#include "io/SerializerKeys.h"

#include "models/application/List.h"
#include "models/OpenScanToolsModelEssentials.h"

class AGraphNode;

class TagNode;
class ScanObjectNode;
class ScanNode;
class ClusterNode;
class BoxNode;
class CylinderNode;
class TorusNode;
class SphereNode;
class PointNode;
class MeshObjectNode;
class SimpleMeasureNode;
class PolylineMeasureNode;
class PointToPlaneMeasureNode;
class PointToPipeMeasureNode;
class PipeToPlaneMeasureNode;
class PipeToPipeMeasureNode;
class BeamBendingMeasureNode;
class ColumnTiltMeasureNode;
class ViewPointNode;
class CameraNode;

class UserOrientation;
class Author;
class ProjectInfos;

namespace sma
{
	class TagTemplate;
}

namespace DataSerializer
{
	void Serialize(nlohmann::json& json, const SafePtr<TagNode>& data);
	void Serialize(nlohmann::json& json, const SafePtr<ScanObjectNode>& data);
	void Serialize(nlohmann::json& json, const SafePtr<ScanNode>& data);
	void Serialize(nlohmann::json& json, const SafePtr<ClusterNode>& data);
	void Serialize(nlohmann::json& json, const SafePtr<BoxNode>& data);
	void Serialize(nlohmann::json& json, const SafePtr<CylinderNode>& data);
	void Serialize(nlohmann::json& json, const SafePtr<TorusNode>& data);
	void Serialize(nlohmann::json& json, const SafePtr<SphereNode>& data);
	void Serialize(nlohmann::json& json, const SafePtr<PointNode>& data);
	void Serialize(nlohmann::json& json, const SafePtr<MeshObjectNode>& data);
	void Serialize(nlohmann::json& json, const SafePtr<SimpleMeasureNode>& data);
	void Serialize(nlohmann::json& json, const SafePtr<PolylineMeasureNode>& data);
	void Serialize(nlohmann::json& json, const SafePtr<PointToPlaneMeasureNode>& data);
	void Serialize(nlohmann::json& json, const SafePtr<PointToPipeMeasureNode>& data);
	void Serialize(nlohmann::json& json, const SafePtr<PipeToPlaneMeasureNode>& data);
	void Serialize(nlohmann::json& json, const SafePtr<PipeToPipeMeasureNode>& data);
	void Serialize(nlohmann::json& json, const SafePtr<BeamBendingMeasureNode>& data);
	void Serialize(nlohmann::json& json, const SafePtr<ColumnTiltMeasureNode>& data);
	void Serialize(nlohmann::json& json, const SafePtr<ViewPointNode>& data);
	void Serialize(nlohmann::json& json, const SafePtr<CameraNode>& data);


	nlohmann::json Serialize(const UserOrientation& data);
	nlohmann::json Serialize(const Author& auth);
	nlohmann::json Serialize(const ProjectInfos& data);
	nlohmann::json Serialize(const sma::TagTemplate& data);


	template<class ListType>
	nlohmann::json SerializeList(ListType data);

	nlohmann::json SerializeStandard(const std::unordered_set<StandardList>& data, StandardType type);

	//TODO MODELMANAGER SERIALIZE GRAPH

	//new
	//void ExportModification(nlohmann::json& modif_json, std::map<xg::Guid, std::pair <std::array<Data*, 2>, std::set<std::string>>> modification);
}

template<class ListType>
nlohmann::json DataSerializer::SerializeList(ListType data)
{
	nlohmann::json json;
	json[Key_Name] = Utils::to_utf8(data.getName());
	json[Key_Origin] = data.getOrigin();
	json[Key_Id] = data.getId().str();
	nlohmann::json childrenElem = nlohmann::json::array();
	for (auto itVec : data.list())
	{
		nlohmann::json item;
		item[Key_Name] = data.toJson(itVec);
		childrenElem.push_back(item);
	}
	json[Key_Elements] = childrenElem;
	return json;
}

#endif // !DATASERIALIZER_H_