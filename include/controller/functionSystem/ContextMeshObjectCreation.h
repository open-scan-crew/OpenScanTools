#ifndef CONTEXT_WAVEFRONT_CREATION_H_
#define CONTEXT_WAVEFRONT_CREATION_H_

#include "controller/functionSystem/ARayTracingContext.h"
#include "controller/messages/IMessage.h"
#include "io/FileUtils.h"
#include "io/MeshObjectTypes.h"
#include "io/imports/ImportTypes.h"
#include <glm/glm.hpp>
#include <filesystem>

class ClusterNode;

enum class Selection;

class ContextMeshObjectCreation : public ARayTracingContext
{
public:
	ContextMeshObjectCreation(const ContextId& id);
	~ContextMeshObjectCreation();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	ContextState abort(Controller& controller) override;

	bool canAutoRelaunch() const;
	ContextType getType() const override;

private:
	glm::dquat getRotation() const;

private:
	std::filesystem::path	m_file;
	FileType				m_extension;
	float					m_inputScale;
	bool					m_isMerge;
	bool					m_truncateCoor;
	PositionOptions			m_posOption;
	glm::vec3				m_infoPosition;
	SafePtr<ClusterNode>	m_rootCluster;
	Selection				m_up, m_forward;
	uint32_t				m_count;
	LODValue				m_lod;
};


#endif // !CONTEXT_WAVEFRONT_CREATION_H_
