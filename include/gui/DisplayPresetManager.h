#ifndef DISPLAY_PRESET_MANAGER_H
#define DISPLAY_PRESET_MANAGER_H

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "models/3d/DisplayParameters.h"
#include "utils/safe_ptr.h"

#include <QtCore/qobject.h>

#include <unordered_map>
#include <vector>

class DialogDisplayPresets;
class MessageSplashScreen;
class ToolBarRenderSettings;
class ToolBarShowHideGroup;
class CameraNode;

class DisplayPresetManager : public QObject, public IPanel
{
	Q_OBJECT

public:
	struct ShowHideState
	{
		bool showTagMarkers = true;
		bool showViewpointMarkers = true;
		bool showClippings = true;
		bool showPoints = true;
		bool showPipes = true;
		bool showAll = true;
		bool showObjectTexts = true;
		bool showSelected = true;
		bool showUnselected = true;
		bool showMeasures = true;
	};

	struct DisplayPreset
	{
		QString name;
		DisplayParameters displayParameters;
		ShowHideState showHideState;
	};

	explicit DisplayPresetManager(IDataDispatcher& dataDispatcher, ToolBarShowHideGroup* showHideGroup, QWidget* parent = nullptr);
	~DisplayPresetManager();

	void registerRenderSettings(ToolBarRenderSettings* settings);

	void informData(IGuiData* data) override;

private slots:
	void handlePresetSelectionChanged(const QString& name);
	void handlePresetNewRequested();
	void handlePresetEditRequested(const QString& name);

	void handleDialogOk();
	void handleDialogUpdate();
	void handleDialogDelete();
	void handleDialogDefault();
	void handleDialogInitialDefault();
	void handleDialogCancel();

private:
	typedef void (DisplayPresetManager::* GuiDataFunction)(IGuiData*);
	inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
	{
		m_dataDispatcher.registerObserverOnKey(this, type);
		m_methods.insert({ type, fct });
	};

	void onProjectLoad(IGuiData* data);
	void onNewProject(IGuiData* data);
	void onOpenProject(IGuiData* data);
	void onFocusViewport(IGuiData* data);
	void onActiveCamera(IGuiData* data);

	void openDialog(const QString& presetName, bool editMode);
	void refreshRenderSettingsUi(const QString& selectedName);
	void applyPresetByName(const QString& name);
	void applyPreset(const DisplayPreset& preset);
	DisplayPreset buildPresetFromCurrent(const QString& name) const;
	ShowHideState getCurrentShowHideState() const;
	ShowHideState getDefaultShowHideState() const;
	DisplayPreset getInitialPreset() const;
	DisplayPreset getRawPreset() const;

	bool presetExists(const QString& name) const;
	DisplayPreset* findPreset(const QString& name);
	const DisplayPreset* findPreset(const QString& name) const;

	void loadPresets();
	void savePresets() const;
	void setDefaultPresetName(const QString& name);

	void showErrorMessage(const QString& message);
	bool validatePresetName(const QString& name, const QString& currentName = QString());

private:
	IDataDispatcher& m_dataDispatcher;
	ToolBarShowHideGroup* m_showHideGroup = nullptr;
	MessageSplashScreen* m_messageScreen = nullptr;
	DialogDisplayPresets* m_dialog = nullptr;

	std::vector<DisplayPreset> m_presets;
	QString m_defaultPresetName;
	QString m_editingPresetName;
	bool m_isProjectLoaded = false;
	bool m_applyDefaultPending = false;
	bool m_updatingSelection = false;

	SafePtr<CameraNode> m_focusCamera;
	std::unordered_map<guiDType, GuiDataFunction> m_methods;
	std::vector<ToolBarRenderSettings*> m_renderSettings;
};

#endif // DISPLAY_PRESET_MANAGER_H
