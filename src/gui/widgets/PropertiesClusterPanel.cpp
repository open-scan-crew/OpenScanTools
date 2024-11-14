#include "gui/widgets/PropertiesClusterPanel.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "utils/QtLogStream.hpp"

#include "gui/widgets/FocusWatcher.h"
#include "controller/Controller.h"
#include "controller/controls/ControlDataEdition.h"

#include "models/graph/ClusterNode.h"
#include "models/3d/NodeFunctions.h"

#include <QtWidgets/qgridlayout.h>

PropertiesClusterPanel::PropertiesClusterPanel(const Controller& controller, QWidget* parent)
	: APropertyGeneral(controller.getDataDispatcher(), parent)
{
	m_ui.setupUi(this);

	m_ui.genericPropsHeadWidget->setControllerInfo(controller);
	m_ui.genericPropsFeetWidget->setDataDispatcher(controller.getDataDispatcher());

	// Link action
	QObject::connect(m_ui.colorPicker, SIGNAL(released()), this, SLOT(showColorPicker()));
	
	// Style
	QPalette palPicker = m_ui.colorPicker->palette();
	palPicker.setColor(QPalette::Button, QColor(0, 111, 190));
	palPicker.setColor(QPalette::ButtonText, !QColor(0, 111, 190));
	m_ui.colorPicker->setPalette(palPicker);

	m_ui.totalVolumeLineEdit->setType(NumericType::VOLUME);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendAuthorsList);
}

PropertiesClusterPanel::~PropertiesClusterPanel()
{
    m_dataDispatcher.unregisterObserver(this);
}

void PropertiesClusterPanel::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);
}

bool PropertiesClusterPanel::actualizeProperty(SafePtr<AGraphNode> object)
{
	if (object)
		m_storedFolder = static_pointer_cast<ClusterNode>(object);

	m_ui.genericPropsHeadWidget->setObject(m_storedFolder);
	m_ui.genericPropsFeetWidget->setObject(m_storedFolder);

	double totalVolume = 0.0;
	std::unordered_set<SafePtr<AGraphNode>> visitedNodes;
	AGraphNode::recOnAncestors(m_storedFolder, [](const SafePtr<AGraphNode>& node) { return AGraphNode::getOwningChildren(node); }, visitedNodes,
		[&totalVolume](const SafePtr<AGraphNode>& child)
		{
			totalVolume += nodeFunctions::calculateVolume(child);
			return true;
		});
	m_ui.totalVolumeLineEdit->setValue(totalVolume);

	ReadPtr<ClusterNode> cl = m_storedFolder.cget();
	if (!cl)
		return false;

	std::wstring name = L"NO_AUTHOR";
	ReadPtr<Author> rAuth = cl->getAuthor().cget();
	if(rAuth)
		name = rAuth->getName();

	QColor color = QColor(cl->getColor().r, cl->getColor().g, cl->getColor().b, cl->getColor().a);
	QPalette palPicker = m_ui.colorPicker->palette();
	palPicker.setColor(QPalette::Button, color);
	palPicker.setColor(QPalette::ButtonText, !color);
	m_ui.colorPicker->setPalette(palPicker);

	if (cl->getDefaultTreeType() != TreeType::Scan)
		m_ui.colorPicker->hide();
	else
		m_ui.colorPicker->show();

	if (cl->getDefaultTreeType() == TreeType::MeshObjects)
	{
		m_ui.totalVolumeLineEdit->hide();
		m_ui.totalVolumeLabel->hide();
	}
	else
	{
		m_ui.totalVolumeLineEdit->show();
		m_ui.totalVolumeLabel->show();
	}


	return true;
}

void PropertiesClusterPanel::informData(IGuiData *data)
{
	if (data->getType() == guiDType::projectLoaded)
		onProjectLoad(data);
}

void PropertiesClusterPanel::onProjectLoad(IGuiData * data)
{
	m_storedFolder.reset();
}

void PropertiesClusterPanel::showColorPicker()
{
	ReadPtr<ClusterNode> cl = m_storedFolder.cget();
	if (!cl)
		return;

	QColor color = QColorDialog::getColor(QColorFromColor32(cl->getColor()), this);

	if (color.isValid() == true)
	{
		if (color != cl->getColor())
		{
			m_dataDispatcher.sendControl(new control::dataEdition::SetColor(m_storedFolder, Color32FromQColor(color)));
			QPalette palPicker = m_ui.colorPicker->palette();
			palPicker.setColor(QPalette::Button, color);
			palPicker.setColor(QPalette::ButtonText, !color);
			m_ui.colorPicker->setPalette(palPicker);
		}
	}
}

void PropertiesClusterPanel::selectColor(QPushButton *color)
{
	ReadPtr<ClusterNode> cl = m_storedFolder.cget();
	if (!cl)
		return;

	if (color->palette().color(QPalette::Button) != cl->getColor())
		m_dataDispatcher.sendControl(new control::dataEdition::SetColor(m_storedFolder, Color32FromQColor(color->palette().color(QPalette::Button))));
}