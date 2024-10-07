#ifndef AOBJECT_NODE_H_
#define AOBJECT_NODE_H_

#include "models/graph/AGraphNode.h"
#include "models/OpenScanToolsModelEssentials.h"
#include "models/3d/MeshDrawData.h"

#include <unordered_set>

#define SGLog Logger::log(LoggerMode::SceneGraphLog)

enum class ManipulationMode;
enum class Selection;

class AObjectNode : public AGraphNode
{
public:

    // Work in progress - Try to define each object type as an aggregation of types
    typedef enum TypeFlagBits {
        POINT_CLOUD_BIT = 0x00000001,
        MARKER_BIT = 0x00000002,
        CYLINDER_BIT,
        BOX_BIT,
        SPHERE_BIT, // for example
        WAVEFRONT_BIT,
        CLIPPING_BIT,
        MEASURE_BIT,
        TYPE_MAX_ENUM = 0x7FFFFFFF,
        SCAN_BIT = POINT_CLOUD_BIT | MARKER_BIT,
        SCAN_OBJECT_BIT = POINT_CLOUD_BIT,
        CLUSTER_BIT, // ??
        PIPE_BIT = CYLINDER_BIT | CLIPPING_BIT,
        BEAM_BENDING_MEASURE = MARKER_BIT | MEASURE_BIT,
    } TypeFlagBits;
    typedef uint32_t TypeFlags;

public:
	AObjectNode();
	AObjectNode(const AObjectNode& node);
	
	~AObjectNode();

	virtual Type getGraphType() const;

	bool isHovered() const;
	void setHovered(bool hovered);

	bool isAcceptingManipulatorMode(const ManipulationMode& mode) const;
	virtual std::unordered_set<Selection> getAcceptableSelections(const ManipulationMode& mode) const;
	virtual std::unordered_set<ManipulationMode> getAcceptableManipulationModes() const;

	virtual void setDefaultData(const Controller& controller) override;

	virtual MeshDrawData getMeshDrawData(const glm::dmat4& gTransfo) const;

protected:
	bool			m_isHovered = false;
};

#endif //! OBJECT_NODE_H_