#include "gui/toolBars/HomeFunctionTab.h"
#include "gui/IPanelManager.h"

#include "controller/controls/ControlProject.h"
#include "utils/Config.h"

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>

const int LAST_STRETCH = 10;

HomeFunctionTab::HomeFunctionTab(IPanelManager* panelManager, QWidget* parent) :
	APanel(panelManager),
	QWidget(parent)
{
	QGridLayout* layout = new QGridLayout(this);

	// Bottom labels
	layout->addWidget(new QLabel(tr("Project"), this), 1, 0);
	layout->addWidget(new QLabel(tr("Point Cloud Rendering"), this), 1, 1);
	layout->addWidget(new QLabel(tr("Tag"), this), 1, 2);
	layout->addWidget(new QLabel(tr("Measure"), this), 1, 3);
	layout->addWidget(new QLabel(tr("Create"), this), 1, 4);
	layout->addWidget(new QLabel(tr("Clipping"), this), 1, 5);

    // "Project" Content
    QGridLayout* homeLayout = new QGridLayout;

    QLabel* projectPropIcon = new QLabel(tr("Label"), this);
    projectPropIcon->setPixmap(QPixmap(COMPILEPATH("./resources/icons/IconProjectProperties.png")));
	projectPropertiesBtn = new QPushButton(tr("Project properties"));

    homeLayout->addWidget(projectPropIcon, 0, 0);
    homeLayout->addWidget(projectPropertiesBtn, 0, 1);
    homeLayout->setRowStretch(3, LAST_STRETCH);

	// "Point Cloud Rendering" Content
    m_pointSize = 2;
    m_colorMode = RenderMode::Intensity;
    m_projMode = ProjectionMode::Perspective;

	QGridLayout* renderLayout = new QGridLayout;

	m_colorIntensity = new QRadioButton(tr("Intensity"));
	m_colorRGB = new QRadioButton(tr("RGB"));
	m_colorIntensity->setChecked(true);

    m_projPerspective = new QPushButton(tr("Perspective"));
    m_projOrthographic = new QPushButton(tr("Orthographic"));

	QLabel* pointSizeLabel = new QLabel(tr("Point size"));
	m_pointSizeSpinBox = new QSpinBox();

	m_pointSizeSpinBox->setRange(m_minPS, m_maxPS);

	refreshUI();

	connect(m_colorIntensity, &QRadioButton::clicked, this, &HomeFunctionTab::setIntensityMode);
	connect(m_colorRGB, &QRadioButton::clicked, this, &HomeFunctionTab::setRGBMode);
	connect(projectPropertiesBtn, &QPushButton::pressed, this, &HomeFunctionTab::OpenProjectProperties);
	connect(m_pointSizeSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &HomeFunctionTab::setPS);
    connect(m_projPerspective, &QPushButton::clicked, this, &HomeFunctionTab::setPerspectiveMode);
    connect(m_projOrthographic, &QPushButton::clicked, this, &HomeFunctionTab::setOrthographicMode);

	renderLayout->addWidget(m_colorIntensity, 0, 0);
	renderLayout->addWidget(m_colorRGB, 1, 0);
	renderLayout->addWidget(pointSizeLabel, 0, 1);
	renderLayout->addWidget(m_pointSizeSpinBox, 0, 2);
    renderLayout->addWidget(m_projPerspective, 0, 3);
    renderLayout->addWidget(m_projOrthographic, 1, 3);
    renderLayout->setRowStretch(3, LAST_STRETCH);

    // Organize general layout
    layout->addLayout(homeLayout, 0, 0);
	layout->addLayout(renderLayout, 0, 1);
    layout->setColumnStretch(6, LAST_STRETCH);

	projectPropertiesBtn->setDisabled(true);

	//registerOnKey({ guiDType::renderSettings, 0 });
	registerOnKey(uiDataKey(guiDType::projectLoaded));
}

HomeFunctionTab::~HomeFunctionTab()
{

}

void HomeFunctionTab::informData(std::pair<uiDataKey, IGuiData*> keyValue)
{
    m_pointSizeSpinBox->blockSignals(true);

    /*
    if (keyValue.first.f == guiDType::renderSettings)
    {
        auto data = static_cast<GuiDataRenderSettings*>(keyValue.second);

        m_renderSettings = data->settings;
        refreshUI();
    }*/

	if (keyValue.first.f == guiDType::projectLoaded && keyValue.second != NULL)
	{
		projectPropertiesBtn->setEnabled(true);
	}

    m_pointSizeSpinBox->blockSignals(false);
}

std::string HomeFunctionTab::getName()
{
    return ("HomeFunctionTab");
}

void HomeFunctionTab::setPS(int _pointSize)
{
    m_pointSize = _pointSize;
    //refreshUI();
    updateData({ {guiDType::renderPointSize, 0}, new GuiDataRenderPointSize(m_pointSize) });
}

void HomeFunctionTab::setIntensityMode()
{
    m_colorMode = RenderMode::Intensity;
    updateData({ { guiDType::renderColorMode, 0 }, new GuiDataRenderColorMode(m_colorMode) });
}

void HomeFunctionTab::setRGBMode()
{
    m_colorMode = RenderMode::Color;
    updateData({ { guiDType::renderColorMode, 0 }, new GuiDataRenderColorMode(m_colorMode) });
}

void HomeFunctionTab::setPerspectiveMode()
{
    m_projMode = ProjectionMode::Perspective;
    updateData({ { guiDType::renderProjectionMode, 0 }, new GuiDataRenderProjectionMode(m_projMode) });
}

void HomeFunctionTab::setOrthographicMode()
{
    m_projMode = ProjectionMode::Orthographic;
    updateData({ { guiDType::renderProjectionMode, 0 }, new GuiDataRenderProjectionMode(m_projMode) });
}

void HomeFunctionTab::refreshUI()
{
    if (m_colorMode == RenderMode::Intensity)
        m_colorIntensity->setChecked(true);
    else
        m_colorRGB->setChecked(true);

    m_pointSizeSpinBox->setValue(m_pointSize);
}

void HomeFunctionTab::OpenProjectProperties()
{
	sendControl(new control::project::ShowProperties());
}