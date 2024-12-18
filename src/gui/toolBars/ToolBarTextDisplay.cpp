#include "gui/toolBars/ToolBarTextDisplay.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"

ToolBarTextDisplay::ToolBarTextDisplay(IDataDispatcher& dataDispatcher, QWidget *parent, const float& guiScale) :
	QWidget(parent),
	m_dataDispatcher(dataDispatcher),
	m_textFilter(13)
{
	m_ui.setupUi(this);
	setEnabled(false);


	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);

	m_methods.insert({ guiDType::projectLoaded, &ToolBarTextDisplay::onProjectLoad });

	connect(m_ui.indexBox, &QCheckBox::clicked, this, [this](bool checked) {this->toggleRenderParameter(checked, TEXT_SHOW_INDEX_BIT); });
	connect(m_ui.authorBox, &QCheckBox::clicked, this, [this](bool checked) {this->toggleRenderParameter(checked, TEXT_SHOW_AUTHOR_BIT); });
	connect(m_ui.identifierBox, &QCheckBox::clicked, this, [this](bool checked) {this->toggleRenderParameter(checked, TEXT_SHOW_IDENTIFIER_BIT); });
	connect(m_ui.nameBox, &QCheckBox::clicked, this, [this](bool checked) {this->toggleRenderParameter(checked, TEXT_SHOW_NAME_BIT); });
	connect(m_ui.disciplineBox, &QCheckBox::clicked, this, [this](bool checked) {this->toggleRenderParameter(checked, TEXT_SHOW_DISCIPLINE_BIT); });
	connect(m_ui.phaseBox, &QCheckBox::clicked, this, [this](bool checked) {this->toggleRenderParameter(checked, TEXT_SHOW_PHASE_BIT); });
	connect(m_ui.diameterBox, &QCheckBox::clicked, this, [this](bool checked) {this->toggleRenderParameter(checked, TEXT_SHOW_DIAMETER_BIT); });
	connect(m_ui.lengthBox, &QCheckBox::clicked, this, [this](bool checked) {this->toggleRenderParameter(checked, TEXT_SHOW_LENGTH_BIT); });
	connect(m_ui.coordinatesBox, &QCheckBox::clicked, this, [this](bool checked) {this->toggleRenderParameter(checked, TEXT_SHOW_COORD_BIT); });

	connect(m_ui.themeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolBarTextDisplay::changeTheme);

	connect(m_ui.fontSizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolBarTextDisplay::changeTextFontSize);

	m_ui.indexBox->setChecked(true);
	m_ui.identifierBox->setChecked(true);
	m_ui.nameBox->setChecked(true);
}

ToolBarTextDisplay::~ToolBarTextDisplay()
{
	m_dataDispatcher.unregisterObserver(this);
}

void ToolBarTextDisplay::informData(IGuiData * data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		TextDisplayMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarTextDisplay::onProjectLoad(IGuiData * data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
	m_ui.indexBox->setEnabled(true);
	m_ui.authorBox->setEnabled(true);
	m_ui.identifierBox->setEnabled(true);
	m_ui.nameBox->setEnabled(true);
	m_ui.disciplineBox->setEnabled(true);
	m_ui.phaseBox->setEnabled(true);
	m_ui.diameterBox->setEnabled(true);
	m_ui.lengthBox->setEnabled(true);
	m_ui.coordinatesBox->setEnabled(true);

	m_ui.themeComboBox->setEnabled(true);
}


inline void ToolBarTextDisplay::addRemoveFilter(TextFilter& textFilter, bool checked, int filter)
{
    if (checked)
        textFilter |= filter;
    else
        textFilter &= ~filter;
	//textFilter += (checked) ? filter : -filter;
	assert(((textFilter & filter) /*== filter (inutile)*/ && checked) || (!(textFilter & filter) && !checked)); //On regarde si le bouton est bien coché ou non selon le filtre appliqué
}

void ToolBarTextDisplay::toggleRenderParameter(bool checked, int parameter) {
	addRemoveFilter(m_textFilter, checked, parameter);
	m_dataDispatcher.updateInformation(new GuiDataRenderTextFilter(m_textFilter, SafePtr<CameraNode>()));
}

void ToolBarTextDisplay::changeTheme(int theme)
{
	//0 -> Dark | 1 -> Light
	m_dataDispatcher.updateInformation(new GuiDataRenderTextTheme(theme, SafePtr<CameraNode>()));
}

void ToolBarTextDisplay::changeTextFontSize(double font)
{
	m_dataDispatcher.updateInformation(new GuiDataRenderTextFontSize((float)font, SafePtr<CameraNode>()));
}
