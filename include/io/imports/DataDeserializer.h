#ifndef DATADESERIALIZER_H_
#define DATADESERIALIZER_H_

#include <nlohmannJson/json.hpp>

#include "utils/safe_ptr.h"
#include "models/application/List.h"


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
	void DeserializeTagNode(WritePtr<TagNode>& nodeToEdit, const nlohmann::json& json, Controller& controller);
	void DeserializeScanObjectNode(WritePtr<ScanObjectNode>& nodeToEdit, const nlohmann::json& json, Controller& controller);
	void DeserializeScanNode(WritePtr<ScanNode>& nodeToEdit, const nlohmann::json& json, Controller& controller);
	void DeserializeClusterNode(WritePtr<ClusterNode>& nodeToEdit, const nlohmann::json& json, Controller& controller);
	void DeserializeBoxNode(WritePtr<BoxNode>& nodeToEdit, const nlohmann::json& json, Controller& controller);
	void DeserializeCylinderNode(WritePtr<CylinderNode>& nodeToEdit, const nlohmann::json& json, Controller& controller);
	void DeserializeTorusNode(WritePtr<TorusNode>& nodeToEdit, const nlohmann::json& json, Controller& controller);
	void DeserializeSphereNode(WritePtr<SphereNode>& nodeToEdit, const nlohmann::json& json, Controller& controller);
	void DeserializePointNode(WritePtr<PointNode>& nodeToEdit, const nlohmann::json& json, Controller& controller);
	void DeserializeMeshObjectNode(WritePtr<MeshObjectNode>& nodeToEdit, const nlohmann::json& json, Controller& controller);
	void DeserializeSimpleMeasureNode(WritePtr<SimpleMeasureNode>& nodeToEdit, const nlohmann::json& json, Controller& controller);
	void DeserializePolylineMeasureNode(WritePtr<PolylineMeasureNode>& nodeToEdit, const nlohmann::json& json, Controller& controller);
	void DeserializePointToPlaneMeasureNode(WritePtr<PointToPlaneMeasureNode>& nodeToEdit, const nlohmann::json& json, Controller& controller);
	void DeserializePointToPipeMeasureNode(WritePtr<PointToPipeMeasureNode>& nodeToEdit, const nlohmann::json& json, Controller& controller);
	void DeserializePipeToPlaneMeasureNode(WritePtr<PipeToPlaneMeasureNode>& nodeToEdit, const nlohmann::json& json, Controller& controller);
	void DeserializePipeToPipeMeasureNode(WritePtr<PipeToPipeMeasureNode>& nodeToEdit, const nlohmann::json& json, Controller& controller);
	void DeserializeBeamBendingMeasureNode(WritePtr<BeamBendingMeasureNode>& nodeToEdit, const nlohmann::json& json, Controller& controller);
	void DeserializeColumnTiltMeasureNode(WritePtr<ColumnTiltMeasureNode>& nodeToEdit, const nlohmann::json& json, Controller& controller);
	void DeserializeViewPointNode(WritePtr<ViewPointNode>& nodeToEdit, const nlohmann::json& json, Controller& controller);

	void DeserializePiping(WritePtr<ClusterNode>& nodeToEdit, const nlohmann::json& json, Controller& controller); //retro-compatibility

	void DeserializeCameraNode(WritePtr<CameraNode>& cameraNode, const nlohmann::json& json);

	void PostDeserializeNode(const nlohmann::json& json, const SafePtr<AGraphNode>& node, const std::unordered_map<xg::Guid, SafePtr<AGraphNode>>& nodeById);

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