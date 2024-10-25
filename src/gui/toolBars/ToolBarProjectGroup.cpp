#include "gui/toolBars/ToolBarProjectGroup.h"
#include "controller/controls/ControlProject.h"
#include "gui/guiData/GuiDataGeneralProject.h"
#include "controller/controls/ControlApplication.h"
#include "gui/Dialog/DialogRecentProjects.h"
#include "gui/Dialog/AuthorListDialog.h"
#include "utils/Logger.h"

#define PANEL_LOG Logger::log(LoggerMode::GuiLog)


ToolBarProjectGroup::ToolBarProjectGroup(IDataDispatcher &dataDispatcher, QWidget *parent, float scale)
	: QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
    , m_projectSaved(true)
    , m_saveable(true)
    , m_guiScale(scale)
{
	m_ui.setupUi(this);
    m_ui.saveButton->setEnabled(false);
	//m_ui.blankProjectBtn->setEnabled(false);
    m_ui.closeButton->setEnabled(false);
    m_ui.importButton->setEnabled(false);
    m_ui.propertiesButton->setEnabled(false);

    // Set the right size for the icons
    m_ui.newButton->setIconSize(QSize(20, 20) * m_guiScale);
    m_ui.openButton->setIconSize(QSize(20, 20) * m_guiScale);
    m_ui.userBtn->setIconSize(QSize(20, 20) * m_guiScale);
    m_ui.saveButton->setIconSize(QSize(20, 20) * m_guiScale);
    m_ui.closeButton->setIconSize(QSize(20, 20) * m_guiScale);
    m_ui.exitButton->setIconSize(QSize(20, 20) * m_guiScale);
    m_ui.importButton->setIconSize(QSize(20, 20) * m_guiScale);
    m_ui.propertiesButton->setIconSize(QSize(20, 20) * m_guiScale);
	m_ui.recentProjects->setIconSize(QSize(20, 20) * m_guiScale);

    connect(m_ui.newButton, &QToolButton::clicked, [this]() { m_dataDispatcher.sendControl(new control::project::SaveCreate()); });
    connect(m_ui.openButton, &QToolButton::clicked, [this]() { m_dataDispatcher.sendControl(new control::project::SaveCloseLoad()); });
    connect(m_ui.saveButton, &QToolButton::clicked, [this]() { m_dataDispatcher.sendControl(new control::project::StartSave()); });
    connect(m_ui.closeButton, &QToolButton::clicked, [this]() { m_dataDispatcher.sendControl(new control::project::SaveClose()); });
    connect(m_ui.exitButton, &QToolButton::clicked, [this]() { m_dataDispatcher.sendControl(new control::project::SaveQuit()); });
	connect(m_ui.propertiesButton, &QToolButton::clicked, [this]() { m_dataDispatcher.sendControl(new control::project::ShowProperties()); });
    connect(m_ui.userBtn, &QToolButton::clicked, this, [this]() { manageAuthors(true); });
    connect(m_ui.importButton, &QToolButton::clicked, [this]() {  m_dataDispatcher.sendControl(new control::project::FunctionImportScan()); });
	connect(m_ui.recentProjects, &QToolButton::clicked, this, &ToolBarProjectGroup::openRecentSave);

    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);

    m_methods.insert({ guiDType::projectLoaded, &ToolBarProjectGroup::onProjectLoad });

    this->QWidget::update();
}

ToolBarProjectGroup::~ToolBarProjectGroup()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarProjectGroup::manageAuthors(bool displayCloseButton)
{
	GUI_LOG << "manage authors" << LOGENDL;
	AuthorListDialog *dialog = new AuthorListDialog(m_dataDispatcher, this->parentWidget());
	m_dataDispatcher.sendControl(new control::application::author::SendAuthorList());
    dialog->setWindowFlag(Qt::WindowCloseButtonHint, displayCloseButton);
	dialog->show();
}

void ToolBarProjectGroup::openRecentSave()
{
	GUI_LOG << "open recent projects" << LOGENDL;
	DialogRecentProjects *dialog = new DialogRecentProjects(m_dataDispatcher, this->parentWidget());
	m_dataDispatcher.sendControl(new control::application::SendRecentProjects());
	dialog->show();
}

void ToolBarProjectGroup::informData(IGuiData *data)
{
    if (m_methods.find(data->getType()) != m_methods.end())
    {
        ProjectGroupMethod method = m_methods.at(data->getType());
        (this->*method)(data);
    }
}

void ToolBarProjectGroup::onProjectLoad(IGuiData * data)
{
	GuiDataProjectLoaded *PLdata = static_cast<GuiDataProjectLoaded*>(data);
	m_ui.saveButton->setEnabled(PLdata->m_isProjectLoad && m_saveable);
    m_ui.closeButton->setEnabled(PLdata->m_isProjectLoad);
	m_ui.importButton->setEnabled(PLdata->m_isProjectLoad && m_saveable);
    m_ui.propertiesButton->setEnabled(PLdata->m_isProjectLoad);
}

void ToolBarProjectGroup::slotProperties()
{
	m_dataDispatcher.sendControl(new control::project::ShowProperties());
}