#include "gui/ShortcutSystem.h"
#include "controller/controls/ControlViewPoint.h"
#include "controller/controls/ControlIO.h"
#include "controller/controls/ControlFunction.h"
#include "controller/controls/ControlSpecial.h"
#include "controller/controls/ControlViewport.h"
#include "controller/controls/ControlScanEdition.h"
#include "controller/controls/ControlMeasure.h"
#include "controller/controls/ControlTest.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataHD.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "models/3d/ManipulationTypes.h"
#include "utils/Logger.h"

ShortcutSystem::ShortcutSystem(IDataDispatcher& dataDispacher, QWidget* parent)
	: m_dataDispatcher(dataDispacher)
	, QObject(parent)
	, m_isEditing(false)
	, m_notInContext(true)
	, m_activeContext(ContextType::none)
	, m_shortcutDefs({ { Qt::Key_Delete, &ShortcutSystem::slotDelete},
						{ Qt::Key_A, &ShortcutSystem::slotAlignView2PointsFunction},
						{ Qt::Key_B, &ShortcutSystem::slotBackground},
						{ Qt::Key_T, &ShortcutSystem::slotManipulatorTranslation},
						{ Qt::Key_R, &ShortcutSystem::slotManipulatorRotation},
						{ Qt::Key_E, &ShortcutSystem::slotManipulatorExtrusion},
						{ Qt::Key_S, &ShortcutSystem::slotManipulatorMode},
						{ Qt::Key_K, &ShortcutSystem::slotRandomScanColor},
						{ Qt::Key_V, &ShortcutSystem::slotCreateViewPoint},
						{ Qt::Key_F4, &ShortcutSystem::slotRecordPreformance},
						{ Qt::Key_D, &ShortcutSystem::slotActivateMeasure},
						{ Qt::Key_ScreenSaver, &ShortcutSystem::slotQuickScreenshot},
						{ Qt::Key_F12, &ShortcutSystem::slotCreateHdImage},
						{ Qt::Key_H, &ShortcutSystem::slotHideSelectedObjects},
						{ Qt::Key_Escape, &ShortcutSystem::slotAbort },
						{ Qt::Key_Return, &ShortcutSystem::slotValidateCurrentContext },
						{ Qt::Key_Enter, &ShortcutSystem::slotValidateCurrentContext }
		})
{

	// Deactivate batch test measurements shortcut
#ifdef _DEBUG_
	m_shortcutDefs.push_back({ Qt::Key_G, &ShortcutSystem::slotGenerateData });
#endif

	for (const ShortcutDef& iterator : m_shortcutDefs)
	{
		QShortcut* shortcut = new QShortcut(parent);
		shortcut->setKey(iterator.sequence);
		shortcut->setEnabled(false);
		shortcut->setAutoRepeat(false);
		QObject::connect(shortcut, &QShortcut::activated, this, iterator.slot);
		m_shortcuts.push_back(shortcut);
	}

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::activatedFunctions);
	GUI_LOG << "shortcut built" << LOGENDL;
}

ShortcutSystem::~ShortcutSystem()
{
	GUI_LOG << "destroy shortcut" << LOGENDL;
}

void ShortcutSystem::changeParent(QWidget* parent)
{
	for (QShortcut* shortcut : m_shortcuts)
		shortcut->setParent(parent);
}

void ShortcutSystem::addShortcut(QShortcut* shortcut)
{
	shortcut->setEnabled(false);
	m_shortcuts.push_back(shortcut);
}

void ShortcutSystem::informData(IGuiData *data)
{
	if (data->getType() == guiDType::projectLoaded)
		onProjectLoad(data);
	if (data->getType() == guiDType::activatedFunctions)
		onActivatedFunctions(data);
}

void ShortcutSystem::onProjectLoad(IGuiData * data)
{
	bool active(static_cast<GuiDataProjectLoaded*>(data)->m_isProjectLoad);
	for (QShortcut* shortcut : m_shortcuts)
		shortcut->setEnabled(active);
}

void ShortcutSystem::onActivatedFunctions(IGuiData* data)
{
	GuiDataActivatedFunctions* function = static_cast<GuiDataActivatedFunctions*>(data);
	m_activeContext = function->type;
	m_notInContext = function->type == ContextType::none;
}

void ShortcutSystem::slotDelete()
{
	GUI_LOG << "shortcut delete" << LOGENDL;
	m_dataDispatcher.sendControl(new control::special::DeleteSelectedElements(false));
}

void ShortcutSystem::slotAbort()
{
	GUI_LOG << "shortcut abort" << LOGENDL;
	if (m_activeContext == ContextType::polygonalSelector)
	{
		m_dataDispatcher.sendControl(new control::function::Validate());
		return;
	}

	if (m_notInContext)
		m_dataDispatcher.updateInformation(new GuiDataDisableFullScreen());
	m_dataDispatcher.sendControl(new control::function::Abort());
	m_dataDispatcher.sendControl(new control::function::AbortSelection());
}

void ShortcutSystem::slotValidateCurrentContext()
{
	m_dataDispatcher.sendControl(new control::function::Validate());
}

void ShortcutSystem::slotEdition(const bool& isEditing)
{
	GUI_LOG << "slotEdition " << isEditing << LOGENDL;
	for (QShortcut* shortcut : m_shortcuts)
		shortcut->setEnabled(!isEditing);
}

void ShortcutSystem::slotBackground()
{
	GUI_LOG << "slotBackground " << LOGENDL;
	m_dataDispatcher.sendControl(new control::viewport::ChangeBackgroundColor());
}

void ShortcutSystem::slotManipulatorTranslation()
{
	sendManipulatorMode(ManipulationMode::Translation);
}

void ShortcutSystem::slotManipulatorRotation()
{
	sendManipulatorMode(ManipulationMode::Rotation);
}

void ShortcutSystem::slotManipulatorExtrusion()
{
	sendManipulatorMode(ManipulationMode::Extrusion);
}

void ShortcutSystem::slotManipulatorScale()
{
	sendManipulatorMode(ManipulationMode::Scale);
}

void ShortcutSystem::sendManipulatorMode(const ManipulationMode& mode)
{
	m_dataDispatcher.updateInformation(new GuiDataManipulatorMode(mode));
}

void ShortcutSystem::slotManipulatorMode()
{
	m_dataDispatcher.updateInformation(new GuiDataManipulatorLocGlob(true));
}

void ShortcutSystem::slotRandomScanColor()
{
	m_dataDispatcher.sendControl(new control::scanEdition::RandomScansColors());
}

void ShortcutSystem::slotRecordPreformance()
{
	m_dataDispatcher.sendControl(new control::io::RecordPerformance());
}

void ShortcutSystem::slotActivateMeasure()
{
	m_dataDispatcher.sendControl(new control::measure::ActivateSimpleMeasure());
}

void ShortcutSystem::slotQuickScreenshot()
{
	m_dataDispatcher.sendControl(new control::io::QuickScreenshot(ImageFormat::MAX_ENUM));
}

void ShortcutSystem::slotCreateHdImage()
{
	m_dataDispatcher.updateInformation(new GuiDataCallImage(true, ""));
}

void ShortcutSystem::slotHideSelectedObjects()
{
	m_dataDispatcher.sendControl(new control::special::ShowHideCurrentObjects(false));
}

void ShortcutSystem::slotAlignView2PointsFunction()
{
	m_dataDispatcher.sendControl(new control::viewport::AlignView2PointsFunction());
}

void ShortcutSystem::slotCreateViewPoint()
{
	m_dataDispatcher.sendControl(new control::viewpoint::LaunchCreationContext());
}

void ShortcutSystem::slotGenerateData()
{
	m_dataDispatcher.sendControl(new control::test::AutoGenerateData());
}
