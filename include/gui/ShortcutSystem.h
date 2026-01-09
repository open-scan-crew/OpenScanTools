#ifndef SHORTCUT_SYSTEM_H_
#define SHORTCUT_SYSTEM_H_

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

#include <QtWidgets/qshortcut.h>
#include <deque>

enum class ManipulationMode;

class ShortcutSystem : public QObject, IPanel
{
public:
	ShortcutSystem(IDataDispatcher &dataDispatcher, QWidget *parent);
	~ShortcutSystem();

	void informData(IGuiData *data) override;

	void onProjectLoad(IGuiData *data);
	void onActivatedFunctions(IGuiData* data);
	void changeParent(QWidget* parent);
	void addShortcut(QShortcut* shortcut);

public slots :
	void slotDelete();
	void slotAbort();
	void slotBackground();
	void slotManipulatorTranslation();
	void slotManipulatorRotation();
	void slotManipulatorExtrusion(); 
	void slotManipulatorScale();
	void slotManipulatorMode();
	void slotRandomScanColor();
	void slotCreateViewPoint();
	void slotEdition(const bool& isEditing);
	void slotRecordPreformance();

	void slotActivateMeasure();
	void slotQuickScreenshot();
	void slotHideSelectedObjects();
	void slotCreateHdImage();

	void slotAlignView2PointsFunction();

	void slotGenerateData();

private:
	typedef void (ShortcutSystem::* SlotFunction)();
	struct ShortcutDef
	{
		QKeySequence	sequence;
		SlotFunction	slot;
	};
	void sendManipulatorMode(const ManipulationMode& mode);

private:
	bool m_isEditing;
	bool m_notInContext;
	IDataDispatcher& m_dataDispatcher;

	std::deque<ShortcutDef> m_shortcutDefs;
	std::deque<QShortcut*>	m_shortcuts;
};

#endif // !SHORTCUT_SYSTEM_H_
