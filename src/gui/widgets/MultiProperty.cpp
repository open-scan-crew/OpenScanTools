#include "gui/widgets/MultiProperty.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "controller/Controller.h"
#include "controller/controls/ControlDataEdition.h"
#include "controller/controls/ControlClippingEdition.h"
#include "controller/controls/ControlMetaControl.h"
#include <qmessagebox.h>
#include <QtWidgets/QMenu>
#include "gui/Texts.hpp"

#include "models/application/Author.h"
#include "models/graph/AClippingNode.h"

#include "gui/texts/DefaultUserLists.hpp"


std::unordered_set<SafePtr<AClippingNode>> getClippingsNodes(const std::unordered_set<SafePtr<AObjectNode>>& nodes)
{
	std::unordered_set<SafePtr<AClippingNode>> clipNodes;
	for (const SafePtr<AGraphNode>& node : nodes)
	{
		ElementType type;
		{
			ReadPtr<AGraphNode> rNode = node.cget();
			if (!rNode)
				continue;
			type = rNode->getType();
		}

		if (s_clippingTypes.find(type) != s_clippingTypes.end())
			clipNodes.insert(static_pointer_cast<AClippingNode>(node));
	}
	return clipNodes;
}

MultiProperty::MultiProperty(Controller& controller, QWidget* parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(controller.getDataDispatcher())
	, m_context(controller.cgetContext())
	, m_hyperLinkDial(new HyperlinkAddDialog(controller.getDataDispatcher(), this->parentWidget()))
{
	m_ui.setupUi(this);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::multiObjectProperties);

	connect(m_ui.addLinkButton, &QPushButton::released, this, [this]() { m_hyperLinkDial->show(); });
	connect(m_hyperLinkDial, &HyperlinkAddDialog::onCreatedLink, this, &MultiProperty::addHyperlink);

	connect(m_ui.colorPicker, &ColorPicker::pickedColor, this, [this](const Color32& color) { m_selectedColor = color; });

	connect(m_ui.colorToolButton, &QToolButton::clicked, this, [this]() {updateToolButton(m_ui.colorToolButton); });
	connect(m_ui.identifierToolButton, &QToolButton::clicked, this, [this]() {updateToolButton(m_ui.identifierToolButton); });
	connect(m_ui.nameToolButton, &QToolButton::clicked, this, [this]() {updateToolButton(m_ui.nameToolButton); });
	connect(m_ui.disciplineToolButton, &QToolButton::clicked, this, [this]() {updateToolButton(m_ui.disciplineToolButton); });
	connect(m_ui.phaseToolButton, &QToolButton::clicked, this, [this]() {updateToolButton(m_ui.phaseToolButton); });
	connect(m_ui.descriptionToolButton, &QToolButton::clicked, this, [this]() {updateToolButton(m_ui.descriptionToolButton); });
	connect(m_ui.linksToolButton, &QToolButton::clicked, this, [this]() {updateToolButton(m_ui.linksToolButton); });

	connect(m_ui.clippingActiveToolButton, &QToolButton::clicked, this, [this]() {updateToolButton(m_ui.clippingActiveToolButton); });
	connect(m_ui.clipMethodToolButton, &QToolButton::clicked, this, [this]() {updateToolButton(m_ui.clipMethodToolButton); });
	connect(m_ui.clipMaxToolButton, &QToolButton::clicked, this, [this]() {updateToolButton(m_ui.clipMaxToolButton); });
	connect(m_ui.clipMinToolButton, &QToolButton::clicked, this, [this]() {updateToolButton(m_ui.clipMinToolButton); });

	connect(m_ui.activeRampToolButton, &QToolButton::clicked, this, [this]() {updateToolButton(m_ui.activeRampToolButton); });
	connect(m_ui.rampMinToolButton, &QToolButton::clicked, this, [this]() {updateToolButton(m_ui.rampMinToolButton); });
	connect(m_ui.rampMaxToolButton, &QToolButton::clicked, this, [this]() {updateToolButton(m_ui.rampMaxToolButton); });
	connect(m_ui.rampStepsToolButton, &QToolButton::clicked, this, [this]() {updateToolButton(m_ui.rampStepsToolButton); });

	connect(m_ui.validateChangeButton, &QPushButton::released, this, &MultiProperty::onActivateChanges);
	connect(m_ui.cancelButton, &QPushButton::released, this, [this]() { close(); });

	m_ui.LinksTableWidget->setColumnCount(1);
	m_ui.LinksTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	m_ui.clipMaxLineEdit->setType(NumericType::DISTANCE);
	m_ui.clipMinLineEdit->setType(NumericType::DISTANCE);

	m_ui.lineEdit_rampMin->setType(NumericType::DISTANCE);
	m_ui.lineEdit_rampMax->setType(NumericType::DISTANCE);

	//General
	m_toolbuttonWidgets[m_ui.colorToolButton] = { m_ui.colorPicker, m_ui.colorLabel };
	m_toolbuttonWidgets[m_ui.nameToolButton] = { m_ui.NameInfield, m_ui.nameLabel };
	m_toolbuttonWidgets[m_ui.identifierToolButton] = { m_ui.IdentifierInfield, m_ui.identifierLabel };
	m_toolbuttonWidgets[m_ui.disciplineToolButton] = { m_ui.comboDiscipline, m_ui.discLabel };
	m_toolbuttonWidgets[m_ui.phaseToolButton] = { m_ui.comboPhase, m_ui.phaseLabel };
	m_toolbuttonWidgets[m_ui.descriptionToolButton] = { m_ui.DescInfield, m_ui.descLabel };
	m_toolbuttonWidgets[m_ui.linksToolButton] = { m_ui.addLinkButton, m_ui.LinksTableWidget, m_ui.linksLabel };

	//Clipping
	m_toolbuttonWidgets[m_ui.clippingActiveToolButton] = { m_ui.clipActiveCheckBox };
	m_toolbuttonWidgets[m_ui.clipMethodToolButton] = { m_ui.interiorClipRadioButton, m_ui.exteriorClipRadioButton, m_ui.clipMethodLabel };
	m_toolbuttonWidgets[m_ui.clipMaxToolButton] = { m_ui.clipMaxLineEdit, m_ui.clipMaxLabel };
	m_toolbuttonWidgets[m_ui.clipMinToolButton] = { m_ui.clipMinLineEdit, m_ui.clipMinLabel };

	//Ramp
	m_toolbuttonWidgets[m_ui.activeRampToolButton] = { m_ui.activeRampCheckBox };
	m_toolbuttonWidgets[m_ui.rampMaxToolButton] = { m_ui.lineEdit_rampMax, m_ui.label_max };
	m_toolbuttonWidgets[m_ui.rampMinToolButton] = { m_ui.lineEdit_rampMin, m_ui.label_min };
	m_toolbuttonWidgets[m_ui.rampStepsToolButton] = { m_ui.spinBox_rampSteps, m_ui.label_steps };

}

MultiProperty::~MultiProperty()
{
	m_dataDispatcher.unregisterObserver(this);
}

void MultiProperty::informData(IGuiData* data)
{
	if (data->getType() == guiDType::multiObjectProperties)
		onMultiData(data);
}

void MultiProperty::onMultiData(IGuiData* data)
{
	std::unordered_set<SafePtr<AObjectNode>> objects;
	GuiDataMultiObjectProperties* gData = static_cast<GuiDataMultiObjectProperties*>(data);
	for (SafePtr<AGraphNode> obj : gData->m_objects)
		objects.insert(static_pointer_cast<AObjectNode>(obj));

	m_objects = objects;

	clearFields();
	updatePhaseDiscipline();
}

void MultiProperty::updateToolButton(QToolButton* button)
{
	if (m_toolbuttonWidgets.find(button) == m_toolbuttonWidgets.end())
		return;

	for (QWidget* widg : m_toolbuttonWidgets.at(button))
		widg->setEnabled(button->isChecked());
	toggleToolButton(button);
}

void MultiProperty::updatePhaseDiscipline()
{
	SafePtr<UserList> disciplineList = m_context.getUserList(listId(DISCIPLINE_ID));
	SafePtr<UserList> phaseList = m_context.getUserList(listId(PHASE_ID));

	{
		m_ui.comboDiscipline->clear();
		ReadPtr<UserList> rDisci = disciplineList.cget();
		if (rDisci)
		{
			for (std::wstring disci : rDisci->clist())
				m_ui.comboDiscipline->addItem(QString::fromStdWString(disci));

			m_ui.comboDiscipline->setCurrentIndex(-1);
		}
	}

	{
		m_ui.comboPhase->clear();
		ReadPtr<UserList> rPhase = phaseList.cget();
		if (rPhase)
		{

			for (std::wstring phase : rPhase->clist())
				m_ui.comboPhase->addItem(QString::fromStdWString(phase));

			m_ui.comboPhase->setCurrentIndex(-1);
		}
	}
}

void MultiProperty::clearFields()
{
	bool containScan = false;
	bool containClip = false;

	for (SafePtr<AObjectNode> object : m_objects)
	{
		ReadPtr<AObjectNode> rObj = object.cget();
		if (!rObj)
			continue;

		if (rObj->getType() == ElementType::Scan)
			containScan = true;

		if (s_clippingTypes.find(rObj->getType()) != s_clippingTypes.end())
			containClip = true;

		if (containScan && containClip)
			break;
	}

	m_ui.nameWidget->setVisible(!containScan);
	m_ui.identifierWidget->setVisible(!containScan);
	m_ui.colorWidget->setVisible(!containScan);

	m_ui.clipWidget->setVisible(containClip);
	m_ui.rampWidget->setVisible(containClip);

	m_ui.NameInfield->clear();
	m_ui.IdentifierInfield->clear();
	m_ui.colorPicker->clearSelection();
	m_ui.comboDiscipline->setCurrentIndex(-1);
	m_ui.comboPhase->setCurrentIndex(-1);
	m_ui.DescInfield->clear();
	m_ui.LinksTableWidget->clear();

	m_ui.interiorClipRadioButton->setChecked(false);
	m_ui.exteriorClipRadioButton->setChecked(false);

	m_ui.clipActiveCheckBox->setChecked(false);

	m_ui.clipMaxLineEdit->setValue(0.);
	m_ui.clipMinLineEdit->setValue(0.);

	m_ui.lineEdit_rampMax->setValue(0.);
	m_ui.lineEdit_rampMin->setValue(0.);

	for (const auto& pair : m_toolbuttonWidgets)
	{
		pair.first->setChecked(false);
		updateToolButton(pair.first);
	}
}

void MultiProperty::updateLinkTable()
{
	m_ui.LinksTableWidget->clear();
	m_ui.LinksTableWidget->setRowCount((int)m_hyperLinks.size());
	int i = 0;
	for (const s_hyperlink& link : m_hyperLinks)
	{
		QLabel* hlinkLabel = new QLabel(this);
		hlinkLabel->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

		QObject::connect(hlinkLabel, &QLabel::customContextMenuRequested, [this, i]() { this->handleContextHyperlink(i); });

		hlinkLabel->setTextFormat(Qt::RichText);
		hlinkLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
		hlinkLabel->setOpenExternalLinks(true);

		hlinkLabel->setText("<a href=\"" + QString::fromStdWString(link.hyperlink) + "\">" + QString::fromStdWString(link.name) + "</a>");
		hlinkLabel->setToolTip(QString::fromStdWString(link.hyperlink));
		m_ui.LinksTableWidget->setCellWidget(i, 0, hlinkLabel);
		i++;
	}
}

void MultiProperty::toggleToolButton(QToolButton* button)
{
	QSize size = button->iconSize();
	button->setIcon(button->isChecked() ? QIcon(":icons/100x100/check.png") : QIcon());
	button->setIconSize(size);
}

void MultiProperty::addHyperlink(std::wstring hyperlink, std::wstring name)
{
	s_hyperlink link;
	link.hyperlink = hyperlink;
	link.name = name;
	m_hyperLinks.push_back(link);

	updateLinkTable();
}

void MultiProperty::handleContextHyperlink(int linkIndex)
{
	PANELLOG << "context custom" << LOGENDL;
	if (linkIndex < 0)
		return;

	QMenu* menu = new QMenu(this);

	QAction* delHLink = new QAction(TEXT_TAG_REMOVELINK, this);
	menu->addAction(delHLink);
	QObject::connect(delHLink, &QAction::triggered, [this, linkIndex]() { deleteHyperlink(linkIndex); });
	menu->popup(QCursor::pos());
}

void MultiProperty::deleteHyperlink(int linkIndex)
{
	if(linkIndex < m_hyperLinks.size())
		m_hyperLinks.erase(m_hyperLinks.begin() + linkIndex);
	updateLinkTable();
}

void MultiProperty::onActivateChanges()
{
	QMessageBox::StandardButton reply;

	reply = QMessageBox::question(this, TEXT_MULTI_ATTRIBUTES_CHANGE_TITLE, TEXT_MULTI_ATTRIBUTES_CHANGE_QUESTION, QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::No)
		return;

	m_dataDispatcher.sendControl(new control::meta::ControlStartMetaControl());

	if (m_ui.nameToolButton->isChecked())
		m_dataDispatcher.sendControl(new control::dataEdition::SetName(m_objects, m_ui.NameInfield->text().toStdWString()));
	if (m_ui.colorToolButton->isChecked())
		m_dataDispatcher.sendControl(new control::dataEdition::SetColor(m_objects, m_selectedColor));
	if (m_ui.descriptionToolButton->isChecked())
		m_dataDispatcher.sendControl(new control::dataEdition::SetDescription(m_objects, m_ui.DescInfield->toPlainText().toStdWString()));
	if (m_ui.identifierToolButton->isChecked())
		m_dataDispatcher.sendControl(new control::dataEdition::SetIdentifier(m_objects, m_ui.IdentifierInfield->text().toStdWString()));
	if (m_ui.phaseToolButton->isChecked())
		m_dataDispatcher.sendControl(new control::dataEdition::SetPhase(m_objects, m_ui.comboPhase->currentText().toStdWString()));
	if (m_ui.disciplineToolButton->isChecked())
		m_dataDispatcher.sendControl(new control::dataEdition::SetDiscipline(m_objects, m_ui.comboDiscipline->currentText().toStdWString()));
	if (m_ui.linksToolButton->isChecked())
		m_dataDispatcher.sendControl(new control::dataEdition::SetHyperLinks(m_objects, m_hyperLinks));

	std::unordered_set<SafePtr<AClippingNode>> clipNodes = getClippingsNodes(m_objects);
	if (m_ui.clippingActiveToolButton->isChecked())
		m_dataDispatcher.sendControl(new control::clippingEdition::SetClipActive(clipNodes, m_ui.clipActiveCheckBox->isChecked()));
	if (m_ui.clipMethodToolButton->isChecked())
		m_dataDispatcher.sendControl(new control::clippingEdition::SetMode(clipNodes, m_ui.interiorClipRadioButton->isChecked() ? ClippingMode::showInterior : ClippingMode::showExterior));
	if (m_ui.clipMinToolButton->isChecked())
		m_dataDispatcher.sendControl(new control::clippingEdition::SetMinClipDist(clipNodes, m_ui.clipMinLineEdit->getValue()));
	if (m_ui.clipMaxToolButton->isChecked())
		m_dataDispatcher.sendControl(new control::clippingEdition::SetMaxClipDist(clipNodes, m_ui.clipMaxLineEdit->getValue()));


	if (m_ui.activeRampToolButton->isChecked())
		m_dataDispatcher.sendControl(new control::clippingEdition::SetRampActive(clipNodes, m_ui.activeRampCheckBox->isChecked()));
	if (m_ui.rampMaxToolButton->isChecked())
		m_dataDispatcher.sendControl(new control::clippingEdition::SetMaxRamp(clipNodes, m_ui.lineEdit_rampMax->getValue()));
	if (m_ui.rampMinToolButton->isChecked())
		m_dataDispatcher.sendControl(new control::clippingEdition::SetMinRamp(clipNodes, m_ui.lineEdit_rampMin->getValue()));
	if (m_ui.rampStepsToolButton->isChecked())
		m_dataDispatcher.sendControl(new control::clippingEdition::SetRampSteps(clipNodes, m_ui.spinBox_rampSteps->value()));

	m_dataDispatcher.sendControl(new control::meta::ControlStopMetaControl());
}