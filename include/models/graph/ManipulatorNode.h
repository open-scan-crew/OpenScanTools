#ifndef MANIPULATOR_NODE_H
#define MANIPULATOR_NODE_H

#include "models/graph/AObjectNode.h"
#include <unordered_set>

class IDataDispatcher;
class MeshBuffer;
class CameraNode;

class ManipulatorNode : public AGraphNode
{
public:
	ManipulatorNode(IDataDispatcher& dataDispatcher);
	~ManipulatorNode();

	virtual bool isDisplayed() const;

	virtual Type getGraphType() const;

	std::shared_ptr<MeshBuffer> getActiveMeshBuffer() const;

	static bool isAcceptingObjectToManip(ElementType type);
	static double getManipSizeFactor(double factor);
	static void setScanManipulable(bool value);
	static std::unordered_set<ElementType> getManipulableTypes();

	bool setTarget(const std::unordered_set<SafePtr<AObjectNode>>& targets);

	void setManipulationMode(ManipulationMode mode);
	ManipulationMode getManipulationMode() const;

	std::unordered_set<Selection> getAcceptableSelections() const;

	// FIXME(robin) - We should add a 'CameraNode&' in the parameters to init correctly the manipulation.
	//  But we cannot because the 'RenderingEngine' do not possess the camera ref when calling the function.
	static bool setCurrentSelection(Selection selection, const glm::ivec2& mousePosition, const SafePtr<ManipulatorNode>& manip);
	Selection getCurrentSelection() const;
	bool isLocalManipulation() const;
	void setLocalManipulation(const bool& isLocal);

	static bool updateEvent(const SafePtr<ManipulatorNode>& manipNode, const glm::ivec2& mousePosition, const glm::ivec2& screenSize, const CameraNode& camera);

	const double& getDistanceToDisplay() const;

	void updateTransfo();
	void setTempManipPos(const glm::dvec3& tempPosition);

protected:
	void initTranslation(const CameraNode& camera);

	void updateTranslation(const glm::ivec2& mousePosition, ManipulateData& transfoMod, const CameraNode& camera);
	void updateRotation(const glm::ivec2& mousePosition, const glm::dmat4& inverseView, ManipulateData& transfoMod, const CameraNode& camera);
	void updateExtrusion(const glm::ivec2& mousePosition, ManipulateData& transfoMod, const CameraNode& camera);
	void updateScale(const glm::ivec2& _mousePosition, ManipulateData& transfoMod, const CameraNode& camera);

	double intersect_with_gizmo_axis(glm::dvec3 pos_ws, glm::dvec3 dir_ws, glm::ivec2 cursor, const CameraNode& camera);

	glm::dvec2 getAxisScreenProjection(const CameraNode& camera);
	glm::dvec3 getGizmoAncorage();
	glm::dvec3 getWorldAxis();
	glm::dvec3 getLocalAxis();
	glm::dmat4 computeTransformationMatrix();

protected:
	std::unordered_set<SafePtr<AObjectNode>>						m_targets;

	ManipulateData	m_cumulatedManipData;

	static std::unordered_set<ElementType> s_manipulableTypes;

	bool					m_isLocalManipulation;
	IDataDispatcher&		m_dataDispatcher;
	ManipulationMode		m_manipulationMode;
	Selection				m_currentSelection;
	glm::ivec2				m_startCursor;
	glm::ivec2				m_screenSize;
	double					m_distanceToDisplay;

	// Translation start params
	glm::dvec3 m_gizmo_pos;
	glm::dvec3 m_gizmo_dir;
	double m_cursor_on_axis;

	glm::dvec3 m_decalPosition = glm::dvec3(NAN);
};

#endif //! MANIPULATOR_NODE_H