#ifndef CONTEXT_PROJECT_LOAD_H_
#define CONTEXT_PROJECT_LOAD_H_

#include "controller/functionSystem/AContext.h"
#include "models/project/ProjectInfos.h"
#include "models/project/ProjectTypes.h"

class CameraNode;

class ContextSaveCloseCreateProject : public AContext
{
public:
    ContextSaveCloseCreateProject(const ContextId& id);
    ~ContextSaveCloseCreateProject();
    ContextState start(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;
    ContextState launch(Controller& controller) override;
    bool canAutoRelaunch() const;

    ContextType getType() const override;

private:
    std::filesystem::path		m_templatePath;
    ProjectTemplate             m_projectTemplate;
    std::filesystem::path		m_folderPath;
    ProjectInfos                m_projectInfo;
    bool						m_saveLastProject;
	bool                        m_closeLastProject;
	bool						m_isWaitingModal;
    SafePtr<CameraNode>         m_cameraNode;
};

class ContextSaveCloseProject : public AContext
{
public:
    ContextSaveCloseProject(const ContextId& id);
    ~ContextSaveCloseProject();
    ContextState start(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;
    ContextState launch(Controller& controller) override;
    bool canAutoRelaunch() const;

    ContextType getType() const override;

private:
    bool						m_saveLastProject;
    SafePtr<CameraNode>         m_cameraNode;
};

class ContextSaveCloseLoadProject : public AContext
{
public:
	ContextSaveCloseLoadProject(const ContextId& id);
	~ContextSaveCloseLoadProject();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;

private:
    enum class WaitFor{Save, Restore, Central};
    void prepareRestoreModal(Controller& controller);

    ContextState processOpenCentral(const uint32_t& value, Controller& controller);
    ContextState processSaveReturn(const uint32_t& value);
    ContextState processRestoreReturn(const uint32_t& value);

private:
	std::filesystem::path		                m_projectToLoad; 
    std::vector<std::filesystem::path>          m_backups;
    WaitFor                                     m_modalsReturn;
    bool						                m_saveAndRestoreModals;
    bool						                m_restoreProject;
	bool						                m_saveLastProject;
    SafePtr<CameraNode>                         m_cameraNode;

    std::filesystem::path                       m_centralPath;
};

class ContextSaveQuitProject : public AContext
{
public:
    ContextSaveQuitProject(const ContextId& id);
    ~ContextSaveQuitProject();
    ContextState start(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;
    ContextState launch(Controller& controller) override;
    bool canAutoRelaunch() const;

    ContextType getType() const override;

private:
    bool						m_saveLastProject;
    bool                        m_closeLastProject;
    bool						m_disableSave;
    SafePtr<CameraNode>         m_cameraNode;
};


class ContextSaveProject : public AContext
{
public:
    ContextSaveProject(const ContextId& id);
    ~ContextSaveProject();
    ContextState start(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;
    ContextState launch(Controller& controller) override;
    bool canAutoRelaunch() const;

    ContextType getType() const override;

private:
    SafePtr<CameraNode>         m_cameraNode;
};

class ContextAutosaveProject : public AContext
{
public:
    ContextAutosaveProject(const ContextId& id);
    ~ContextAutosaveProject();
    ContextState start(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;
    ContextState launch(Controller& controller) override;
    bool canAutoRelaunch() const;

    ContextType getType() const override;

private:
    SafePtr<CameraNode>         m_cameraNode;
};

#endif // !CONTEXT_PROJECT_LOAD_H_