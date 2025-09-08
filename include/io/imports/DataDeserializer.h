#ifndef DATADESERIALIZER_H_
#define DATADESERIALIZER_H_

#include <nlohmannJson/json.hpp>

#include "utils/safe_ptr.h"
#include "models/application/List.h"


class AGraphNode;

class TagNode;
class PointCloudNode;
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

class Author;
class ProjectInfos;
class UserOrientation;

namespace sma
{
	class TagTemplate;
}

class Controller;
class GraphManager;

namespace DataDeserializer
{
	bool DeserializeTagNode(SafePtr<TagNode> node, const nlohmann::json& json, Controller& controller);
	bool DeserializePointCloudNode(SafePtr<PointCloudNode> node, const nlohmann::json& json, Controller& controller);
	bool DeserializeClusterNode(SafePtr<ClusterNode> node, const nlohmann::json& json, Controller& controller);
	bool DeserializeBoxNode(SafePtr<BoxNode> node, const nlohmann::json& json, Controller& controller);
	bool DeserializeCylinderNode(SafePtr<CylinderNode> node, const nlohmann::json& json, Controller& controller);
	bool DeserializeTorusNode(SafePtr<TorusNode> node, const nlohmann::json& json, Controller& controller);
	bool DeserializeSphereNode(SafePtr<SphereNode> node, const nlohmann::json& json, Controller& controller);
	bool DeserializePointNode(SafePtr<PointNode> node, const nlohmann::json& json, Controller& controller);
	bool DeserializeMeshObjectNode(SafePtr<MeshObjectNode> node, const nlohmann::json& json, Controller& controller);
	bool DeserializeSimpleMeasureNode(SafePtr<SimpleMeasureNode> node, const nlohmann::json& json, Controller& controller);
	bool DeserializePolylineMeasureNode(SafePtr<PolylineMeasureNode> node, const nlohmann::json& json, Controller& controller);
	bool DeserializePointToPlaneMeasureNode(SafePtr<PointToPlaneMeasureNode> node, const nlohmann::json& json, Controller& controller);
	bool DeserializePointToPipeMeasureNode(SafePtr<PointToPipeMeasureNode> node, const nlohmann::json& json, Controller& controller);
	bool DeserializePipeToPlaneMeasureNode(SafePtr<PipeToPlaneMeasureNode> node, const nlohmann::json& json, Controller& controller);
	bool DeserializePipeToPipeMeasureNode(SafePtr<PipeToPipeMeasureNode> node, const nlohmann::json& json, Controller& controller);
	bool DeserializeBeamBendingMeasureNode(SafePtr<BeamBendingMeasureNode> node, const nlohmann::json& json, Controller& controller);
	bool DeserializeColumnTiltMeasureNode(SafePtr<ColumnTiltMeasureNode> node, const nlohmann::json& json, Controller& controller);
	bool DeserializeViewPointNode(SafePtr<ViewPointNode> node, const nlohmann::json& json, Controller& controller);

	bool DeserializePiping(SafePtr<ClusterNode> node, const nlohmann::json& json, Controller& controller); //retro-compatibility

	bool DeserializeCameraNode(SafePtr<CameraNode> cameraNode, const nlohmann::json& json);

	void PostDeserializeNode(const nlohmann::json& json, const SafePtr<AGraphNode> node, const std::unordered_map<xg::Guid, SafePtr<AGraphNode>>& nodeById);

	template<class ListType>
	bool DeserializeList(const nlohmann::json& json, ListType& data);

	bool DeserializeStandards(const nlohmann::json& json, std::unordered_map<StandardType, std::vector<StandardList>>& data);

	bool DeserializeUserOrientation(const nlohmann::json& json, UserOrientation& data);
	bool DeserializeAuthor(const nlohmann::json& json, Author& auth);
	bool DeserializeProjectInfos(const nlohmann::json& json, const Controller& controller, ProjectInfos& data);
	bool DeserializeTagTemplate(const nlohmann::json& json, const Controller& controller, sma::TagTemplate& data);


	//new
	//bool ImportModification(const nlohmann::json& json, std::set<xg::Guid>& object_deleted, std::set<xg::Guid>& object_added, std::map<xg::Guid, std::set<std::string>>& object_updated );
}

#endif // !DATADESERIALIZER_H_