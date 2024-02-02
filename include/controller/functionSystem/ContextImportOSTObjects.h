#ifndef CONTEXT_IMPORT_OST_H_
#define CONTEXT_IMPORT_OST_H_

#include "controller/functionSystem/AContext.h"
#include "models/OpenScanToolsModelEssentials.h"

#include <filesystem>
#include <glm/glm.hpp>

#include <unordered_set>
#include <unordered_map>

class AGraphNode;

class ContextImportOSTObjects : public AContext
{
public:
	ContextImportOSTObjects(const ContextId& id);
	~ContextImportOSTObjects();
	ContextState start(Controller& controller);
	ContextState launch(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller);

	bool canAutoRelaunch() const;
	ContextType getType() const override;

private:
	void addObjectsToProject(Controller& controller, const std::unordered_set<SafePtr<AGraphNode>>& objects);

private:
	std::filesystem::path										m_folder;
	std::vector<std::filesystem::path>							m_importFiles;
	std::unordered_set<SafePtr<AGraphNode>>						m_missingFileObjects;
	std::unordered_set<SafePtr<AGraphNode>>						m_importData;
};


class ContextLinkOSTObjects : public AContext
{
public:
	ContextLinkOSTObjects(const ContextId& id);
	~ContextLinkOSTObjects();
	ContextState start(Controller& controller);
	ContextState launch(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller);
	ContextState abort(Controller& controller) override;

	bool canAutoRelaunch() const;
	ContextType getType() const override;

private:
	void finish(Controller& controller);

private:
	std::filesystem::path									m_folder;
	std::unordered_set<SafePtr<AGraphNode>>					m_currentMissing;
	std::unordered_set<SafePtr<AGraphNode>>					m_startMissingFile;
};

#endif // !CONTEXT_EXPORT_CSV_H_
