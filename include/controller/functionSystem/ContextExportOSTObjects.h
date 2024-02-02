#ifndef CONTEXT_EXPORT_OST_OBJECTS_H_
#define CONTEXT_EXPORT_OST_OBJECTS_H_

#include "controller/functionSystem/AContext.h"
#include "models/3d/Graph/AGraphNode.h"

#include <filesystem>
#include <glm/glm.hpp>

#include <unordered_set>

struct Measure;

class AGraphNode;

class ContextExportOSTObjects : public AContext
{
public:
	ContextExportOSTObjects(const ContextId& id);
	~ContextExportOSTObjects();
	ContextState start(Controller& controller);
	ContextState launch(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller);
	bool canAutoRelaunch() const;
	ContextType getType() const override;

private:
	std::unordered_set<SafePtr<AGraphNode>>			m_objectsToExport;
	std::filesystem::path				m_output;
	bool								m_openExportFolder;
};

#endif // !CONTEXT_EXPORT_CSV_H_
