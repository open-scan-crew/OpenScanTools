#include "gui/toolBars/ToolBarRenderSettings.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "controller/controls/ControlApplication.h"
#include "gui/texts/RenderingTexts.hpp"
#include "gui/UITransparencyConverter.h"

#include <cmath>
#include <qcolordialog.h>
#include <QSignalBlocker>
#include <QStringList>

#include "models/graph/CameraNode.h"

ToolBarRenderSettings::ToolBarRenderSettings(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale)
    : QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
    , m_currentRenderMode(UiRenderMode::Fake_Color)
    , m_brightness(0)
    , m_contrast(0)
    , m_lumiance(0)
    , m_saturation(0)
    , m_selectedColor(128,128,128)
	, m_intensityActive(true)
{
	m_ui.setupUi(this);
	setEnabled(false);

	std::unordered_map<UiRenderMode, std::string> tradUiRenderMode(getTradUiRenderMode());
    for (uint32_t iterator(0); iterator < (uint32_t)UiRenderMode::UiRenderMode_MaxEnum; iterator++)
    {
		//fixeme (Aurélien) to remove when I & RGB done
#ifndef _DEBUG
		if (iterator == 2)
			continue;
#endif // !_DEBUG
        m_ui.comboBox_renderMode->addItem(QString::fromStdString(tradUiRenderMode.at(UiRenderMode(iterator))),QVariant(iterator));
    }

	m_ui.brightnessLuminanceSlider->setMinimumWidth(100.f * guiScale);
	m_ui.contrastSaturationSlider->setMinimumWidth(100.f * guiScale);
	m_ui.transparencySlider->setMinimumWidth(100.f * guiScale);
    m_ui.transparencyCheckBox->setChecked(false);
    m_ui.transparencySpinBox->setEnabled(false);
    m_ui.transparencySlider->setEnabled(false);


	m_ui.lineEdit_rampMax->setType(NumericType::DISTANCE);
	m_ui.lineEdit_rampMin->setType(NumericType::DISTANCE);

	m_ui.comboBox_gapFillingStrength->addItem(tr("Off"), 9);
	m_ui.comboBox_gapFillingStrength->addItem(tr("Low"), 4);
	m_ui.comboBox_gapFillingStrength->addItem(tr("Mid"), 2);
	m_ui.comboBox_gapFillingStrength->addItem(tr("High"), 1);
	m_ui.comboBox_gapFillingStrength->setCurrentIndex(1);
	m_savedGapFillingIndex = m_ui.comboBox_gapFillingStrength->currentIndex();

	m_ui.comboBox_pointShape->addItem(tr("Squares"), QVariant(static_cast<int>(PointShape::Square)));
	m_ui.comboBox_pointShape->addItem(tr("Splats"), QVariant(static_cast<int>(PointShape::Splat)));
	m_ui.comboBox_pointShape->setCurrentIndex(0);
	m_ui.doubleSpinBox_splatRadius->setEnabled(false);

    // Init render options UI
    m_ui.pushButton_color->setPalette(QPalette(m_selectedColor));
    switchRenderMode((int)m_currentRenderMode);

	connect(m_ui.comboBox_renderMode,QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolBarRenderSettings::slotSetRenderMode);

	connect(m_ui.spinBox_pointSize, qOverload<int>(&QSpinBox::valueChanged), this, &ToolBarRenderSettings::slotSetPointSize);
	connect(m_ui.comboBox_pointShape, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolBarRenderSettings::slotSetPointShape);
	connect(m_ui.doubleSpinBox_splatRadius, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ToolBarRenderSettings::slotSetSplatRadius);
	connect(m_ui.comboBox_gapFillingStrength, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolBarRenderSettings::slotSetTexelThreshold);

	connect(m_ui.pushButton_color, &QPushButton::clicked, this, &ToolBarRenderSettings::slotColorPicking);

	// Slider - Spin box automation
	connect(m_ui.brightnessLuminanceSlider, &QSlider::valueChanged, m_ui.brightnessLuminanceSpinBox, &QSpinBox::setValue);
	connect(m_ui.brightnessLuminanceSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.brightnessLuminanceSlider, &QSlider::setValue);
	connect(m_ui.brightnessLuminanceSlider, &QSlider::valueChanged, this, &ToolBarRenderSettings::slotBrightnessLuminanceValueChanged);

	connect(m_ui.contrastSaturationSlider, &QSlider::valueChanged, m_ui.contrastSaturationSpinBox, &QSpinBox::setValue);
	connect(m_ui.contrastSaturationSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.contrastSaturationSlider, &QSlider::setValue);
	connect(m_ui.contrastSaturationSlider, &QSlider::valueChanged, this, &ToolBarRenderSettings::slotContrastSaturationValueChanged);

	connect(m_ui.falseColorSlider, &QSlider::valueChanged, m_ui.falseColorSpinBox, &QSpinBox::setValue);
	connect(m_ui.falseColorSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.falseColorSlider, &QSlider::setValue);
	connect(m_ui.falseColorSlider, &QSlider::valueChanged, this, &ToolBarRenderSettings::slotFakeColorValueChanged);

	connect(m_ui.alphaObjectsSlider, &QSlider::valueChanged, m_ui.alphaObjectsSpinBox, &QSpinBox::setValue);
	connect(m_ui.alphaObjectsSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.alphaObjectsSlider, &QSlider::setValue);
	connect(m_ui.alphaObjectsSlider, &QSlider::valueChanged, this, &ToolBarRenderSettings::slotAlphaBoxesValueChanged);

	connect(m_ui.transparencySlider, &QSlider::valueChanged, m_ui.transparencySpinBox, &QSpinBox::setValue);
	connect(m_ui.transparencySpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.transparencySlider, &QSlider::setValue);
	connect(m_ui.transparencySlider, &QSlider::valueChanged, this, &ToolBarRenderSettings::slotTransparencyValueChanged);

	connect(m_ui.transparencyCheckBox, &QCheckBox::stateChanged, this, &ToolBarRenderSettings::slotTranparencyActivationChanged);

	connect(m_ui.lineEdit_rampMin, &QLineEdit::editingFinished, this, &ToolBarRenderSettings::slotRampValues);
	connect(m_ui.lineEdit_rampMax, &QLineEdit::editingFinished, this, &ToolBarRenderSettings::slotRampValues);
	connect(m_ui.spinBox_rampStep, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ToolBarRenderSettings::slotRampValues);

	connect(m_ui.checkBox_normals, &QCheckBox::stateChanged, this, &ToolBarRenderSettings::slotNormalsChanged);
	connect(m_ui.slider_normals, &QSlider::valueChanged, m_ui.spinBox_normals, &QSpinBox::setValue);
	connect(m_ui.slider_normals, &QSlider::valueChanged, this, &ToolBarRenderSettings::slotNormalsChanged);
	connect(m_ui.spinBox_normals, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_normals, &QSlider::setValue);
	connect(m_ui.comboBox_displayPresets, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolBarRenderSettings::slotDisplayPresetSelectionChanged);
	connect(m_ui.pushButton_newDisplayPresets, &QPushButton::clicked, this, &ToolBarRenderSettings::slotDisplayPresetNew);
	connect(m_ui.pushButton_editDisplayPresets, &QPushButton::clicked, this, &ToolBarRenderSettings::slotDisplayPresetEdit);

	registerGuiDataFunction(guiDType::renderBrightness, &ToolBarRenderSettings::onRenderBrightness);
	registerGuiDataFunction(guiDType::renderContrast, &ToolBarRenderSettings::onRenderContrast);
	registerGuiDataFunction(guiDType::renderColorMode, &ToolBarRenderSettings::onRenderColorMode);
	registerGuiDataFunction(guiDType::renderLuminance, &ToolBarRenderSettings::onRenderLuminance);
	registerGuiDataFunction(guiDType::renderBlending, &ToolBarRenderSettings::onRenderBlending);
	registerGuiDataFunction(guiDType::renderPointSize, &ToolBarRenderSettings::onRenderPointSize);
	registerGuiDataFunction(guiDType::renderPointShape, &ToolBarRenderSettings::onRenderPointShape);
	registerGuiDataFunction(guiDType::renderSplatRadius, &ToolBarRenderSettings::onRenderSplatRadius);
	registerGuiDataFunction(guiDType::renderTexelThreshold, &ToolBarRenderSettings::onRenderTexelThreshold);
        registerGuiDataFunction(guiDType::renderSaturation, &ToolBarRenderSettings::onRenderSaturation);
        
        registerGuiDataFunction(guiDType::renderValueDisplay, &ToolBarRenderSettings::onRenderUnitUsage);
	registerGuiDataFunction(guiDType::renderActiveCamera, &ToolBarRenderSettings::onActiveCamera);
	registerGuiDataFunction(guiDType::focusViewport, &ToolBarRenderSettings::onFocusViewport);

	registerGuiDataFunction(guiDType::projectLoaded, &ToolBarRenderSettings::onProjectLoad);
}

ToolBarRenderSettings::~ToolBarRenderSettings()
{
	m_dataDispatcher.unregisterObserver(this);
}
//fixme POC dynamic langage
void ToolBarRenderSettings::changeEvent(QEvent* event)
{
	//switch (event->type()) 
	//{
	//	// this event is send if a translator is loaded
	//case QEvent::LanguageChange:
	//	m_ui.retranslateUi(this);
	//	break;
	//}
}

void ToolBarRenderSettings::informData(IGuiData *data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		GuiDataFunction method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarRenderSettings::onRenderBrightness(IGuiData* idata)
{
	GuiDataRenderBrightness* data = static_cast<GuiDataRenderBrightness*>(idata);
	m_brightness = data->m_brightness;
	if (!m_intensityActive)
		return;
	const QSignalBlocker spinBlocker(m_ui.brightnessLuminanceSpinBox);
	const QSignalBlocker sliderBlocker(m_ui.brightnessLuminanceSlider);
	m_ui.brightnessLuminanceSpinBox->setValue(data->m_brightness);
	m_ui.brightnessLuminanceSlider->setValue(data->m_brightness);
}

void ToolBarRenderSettings::onRenderContrast(IGuiData* idata)
{
	GuiDataRenderContrast* data = static_cast<GuiDataRenderContrast*>(idata);
	m_contrast = data->m_contrast;
	if (!m_intensityActive)
		return;
	const QSignalBlocker spinBlocker(m_ui.contrastSaturationSpinBox);
	const QSignalBlocker sliderBlocker(m_ui.contrastSaturationSlider);
	m_ui.contrastSaturationSpinBox->setValue(data->m_contrast);
	m_ui.contrastSaturationSlider->setValue(data->m_contrast);
}

void ToolBarRenderSettings::onRenderColorMode(IGuiData* idata)
{
	GuiDataRenderColorMode* data = static_cast<GuiDataRenderColorMode*>(idata);
	std::unordered_map<UiRenderMode, std::string> tradUiRenderMode(getTradUiRenderMode());
	m_ui.comboBox_renderMode->setCurrentText(QString::fromStdString(tradUiRenderMode.at(UiRenderMode(static_cast<int>(data->m_mode)))));
}

void ToolBarRenderSettings::onRenderLuminance(IGuiData* idata)
{
	GuiDataRenderLuminance* data = static_cast<GuiDataRenderLuminance*>(idata);
	m_lumiance = data->m_luminance;
	if (m_intensityActive)
		return;
	const QSignalBlocker spinBlocker(m_ui.brightnessLuminanceSpinBox);
	const QSignalBlocker sliderBlocker(m_ui.brightnessLuminanceSlider);
	m_ui.brightnessLuminanceSpinBox->setValue(data->m_luminance);
	m_ui.brightnessLuminanceSlider->setValue(data->m_luminance);
}

void ToolBarRenderSettings::onRenderBlending(IGuiData* idata)
{
	GuiDataRenderBlending* data = static_cast<GuiDataRenderBlending*>(idata);
	const QSignalBlocker spinBlocker(m_ui.falseColorSpinBox);
	const QSignalBlocker sliderBlocker(m_ui.falseColorSlider);
	m_ui.falseColorSpinBox->setValue(data->m_hue);
	m_ui.falseColorSlider->setValue(data->m_hue);
}

void ToolBarRenderSettings::onRenderPointSize(IGuiData* idata)
{
	GuiDataRenderPointSize* data = static_cast<GuiDataRenderPointSize*>(idata);
	if (data->m_pointSize != m_ui.spinBox_pointSize->value())
	{
		m_ui.spinBox_pointSize->blockSignals(true);
	    m_ui.spinBox_pointSize->setValue(data->m_pointSize);
		m_ui.spinBox_pointSize->blockSignals(false);
	}
}

void ToolBarRenderSettings::onRenderPointShape(IGuiData* idata)
{
	GuiDataRenderPointShape* data = static_cast<GuiDataRenderPointShape*>(idata);
	m_pointShape = data->m_shape;
	int index = m_ui.comboBox_pointShape->findData(static_cast<int>(m_pointShape));
	if (index != -1 && index != m_ui.comboBox_pointShape->currentIndex())
	{
		m_ui.comboBox_pointShape->blockSignals(true);
		m_ui.comboBox_pointShape->setCurrentIndex(index);
		m_ui.comboBox_pointShape->blockSignals(false);
	}

	bool splatEnabled = (m_pointShape == PointShape::Splat);
	m_ui.doubleSpinBox_splatRadius->setEnabled(splatEnabled);
	m_ui.comboBox_gapFillingStrength->setEnabled(!splatEnabled);
	m_ui.label_gapFilling->setEnabled(!splatEnabled);
	if (splatEnabled)
	{
		int offIndex = m_ui.comboBox_gapFillingStrength->findData(9);
		if (offIndex != -1)
		{
			m_ui.comboBox_gapFillingStrength->blockSignals(true);
			m_ui.comboBox_gapFillingStrength->setCurrentIndex(offIndex);
			m_ui.comboBox_gapFillingStrength->blockSignals(false);
		}
	}
}

void ToolBarRenderSettings::onRenderSplatRadius(IGuiData* idata)
{
	GuiDataRenderSplatRadius* data = static_cast<GuiDataRenderSplatRadius*>(idata);
	if (!qFuzzyCompare(data->m_radiusPx + 1.0, m_ui.doubleSpinBox_splatRadius->value() + 1.0))
	{
		m_ui.doubleSpinBox_splatRadius->blockSignals(true);
		m_ui.doubleSpinBox_splatRadius->setValue(data->m_radiusPx);
		m_ui.doubleSpinBox_splatRadius->blockSignals(false);
	}
}
void ToolBarRenderSettings::onRenderTexelThreshold(IGuiData* idata)
{
	GuiDataRenderTexelThreshold* data = static_cast<GuiDataRenderTexelThreshold*>(idata);
	int index = m_ui.comboBox_gapFillingStrength->findData(data->m_texelThreshold);
	if (index != -1 && index != m_ui.comboBox_gapFillingStrength->currentIndex())
	{
		m_ui.comboBox_gapFillingStrength->blockSignals(true);
		m_ui.comboBox_gapFillingStrength->setCurrentIndex(index);
		m_ui.comboBox_gapFillingStrength->blockSignals(false);
	}
}
void ToolBarRenderSettings::onRenderSaturation(IGuiData* idata) 
{
	GuiDataRenderSaturation* data = static_cast<GuiDataRenderSaturation*>(idata);
	m_saturation = data->m_saturation;
	if (m_intensityActive)
		return;
	const QSignalBlocker spinBlocker(m_ui.contrastSaturationSpinBox);
	const QSignalBlocker sliderBlocker(m_ui.contrastSaturationSlider);
	m_ui.contrastSaturationSpinBox->setValue(data->m_saturation);
	m_ui.contrastSaturationSlider->setValue(data->m_saturation);
}

void ToolBarRenderSettings::onRenderAlphaObjects(IGuiData* idata)
{
	GuiDataAlphaObjectsRendering* data = static_cast<GuiDataAlphaObjectsRendering*>(idata);
	int value((data->m_alpha * 100) + 1);
	m_ui.alphaObjectsSpinBox->setValue(value);
	m_ui.alphaObjectsSlider->setValue(value);
}

void ToolBarRenderSettings::onRenderUnitUsage(IGuiData * idata)
{
	auto castdata = static_cast<GuiDataRenderUnitUsage*>(idata);

	m_ui.lineEdit_rampMin->setUnit(castdata->m_valueParameters.distanceUnit);
	m_ui.lineEdit_rampMax->setUnit(castdata->m_valueParameters.distanceUnit);
}

void ToolBarRenderSettings::onProjectLoad(IGuiData* idata)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(idata);
	setEnabled(plData->m_isProjectLoad);
}

void ToolBarRenderSettings::onActiveCamera(IGuiData* idata)
{
	auto infos = static_cast<GuiDataCameraInfo*>(idata);
	if (infos->m_camera && m_focusCamera != infos->m_camera)
		return;

	ReadPtr<CameraNode> rCam = m_focusCamera.cget();
	if (!rCam)
		return;
	const DisplayParameters& displayParameters = rCam->getDisplayParameters();

	blockAllSignals(true);

	m_ui.transparencyCheckBox->setChecked(displayParameters.m_blendMode == BlendMode::Transparent);
	float uiTransparency = ui::transparency::trueValue_to_uiValue(displayParameters.m_transparency);
	m_ui.transparencySpinBox->setValue(uiTransparency);
	m_ui.transparencySpinBox->setEnabled(m_ui.transparencyCheckBox->isChecked());
	m_ui.transparencySlider->setValue(uiTransparency);
	m_ui.transparencySlider->setEnabled(m_ui.transparencyCheckBox->isChecked());

	std::unordered_map<UiRenderMode, std::string> tradUiRenderMode(getTradUiRenderMode());
	m_ui.comboBox_renderMode->setCurrentText(QString::fromStdString(tradUiRenderMode.at(displayParameters.m_mode)));
	m_selectedColor.setRgbF(displayParameters.m_flatColor.x, displayParameters.m_flatColor.y, displayParameters.m_flatColor.z);
	m_ui.pushButton_color->setPalette(QPalette(m_selectedColor));
	switchRenderMode(static_cast<int>(displayParameters.m_mode));

	m_contrast = displayParameters.m_contrast;
	m_brightness = displayParameters.m_brightness;

	m_lumiance = displayParameters.m_luminance;
	m_saturation = displayParameters.m_saturation;

	if (m_intensityActive)
	{
		m_ui.brightnessLuminanceSpinBox->setValue(displayParameters.m_brightness);
		m_ui.brightnessLuminanceSlider->setValue(displayParameters.m_brightness);

		m_ui.contrastSaturationSpinBox->setValue(displayParameters.m_contrast);
		m_ui.contrastSaturationSlider->setValue(displayParameters.m_contrast);
	}
	else
	{
		m_ui.brightnessLuminanceSpinBox->setValue(displayParameters.m_luminance);
		m_ui.brightnessLuminanceSlider->setValue(displayParameters.m_luminance);

		m_ui.contrastSaturationSpinBox->setValue(displayParameters.m_saturation);
		m_ui.contrastSaturationSlider->setValue(displayParameters.m_saturation);
	}

	m_ui.falseColorSpinBox->setValue(displayParameters.m_hue);
	m_ui.falseColorSlider->setValue(displayParameters.m_hue);

	if (displayParameters.m_pointSize != m_ui.spinBox_pointSize->value())
		m_ui.spinBox_pointSize->setValue(displayParameters.m_pointSize);

	int shapeIndex = m_ui.comboBox_pointShape->findData(static_cast<int>(displayParameters.m_pointShape));
	if (shapeIndex != -1)
		m_ui.comboBox_pointShape->setCurrentIndex(shapeIndex);
	m_ui.doubleSpinBox_splatRadius->setValue(displayParameters.m_splatRadiusPx);

	int texelIndex = m_ui.comboBox_gapFillingStrength->findData(displayParameters.m_texelThreshold);
	if (texelIndex != -1)
		m_ui.comboBox_gapFillingStrength->setCurrentIndex(texelIndex);
	m_savedGapFillingIndex = m_ui.comboBox_gapFillingStrength->currentIndex();

	const bool splatEnabled = (displayParameters.m_pointShape == PointShape::Splat);
	m_ui.comboBox_gapFillingStrength->setEnabled(!splatEnabled);
	m_ui.label_gapFilling->setEnabled(!splatEnabled);
	m_ui.doubleSpinBox_splatRadius->setEnabled(splatEnabled);

	int value(100 - (int)(displayParameters.m_alphaObject * 100));
	m_ui.alphaObjectsSpinBox->setValue(value);
	m_ui.alphaObjectsSlider->setValue(value);

	// Distance Ramp
	m_ui.lineEdit_rampMin->setValue(displayParameters.m_distRampMin);
	m_ui.lineEdit_rampMax->setValue(displayParameters.m_distRampMax);
	m_ui.spinBox_rampStep->setValue(displayParameters.m_distRampSteps);

	// Normals
	Qt::CheckState normalState(Qt::CheckState::Unchecked);
	if (displayParameters.m_postRenderingNormals.show)
	{
		if (displayParameters.m_postRenderingNormals.inverseTone)
			normalState = Qt::CheckState::PartiallyChecked;
		else
			normalState = Qt::CheckState::Checked;
	}
	m_ui.checkBox_normals->setCheckState(normalState);

	// ambiant value
	int newNormalValue = (int)std::round(displayParameters.m_postRenderingNormals.normalStrength * 100.f);

	m_ui.slider_normals->setEnabled(normalState != Qt::CheckState::Unchecked);
	m_ui.slider_normals->setValue(newNormalValue);
	m_ui.spinBox_normals->setEnabled(normalState != Qt::CheckState::Unchecked);
	m_ui.spinBox_normals->setValue(newNormalValue);

        blockAllSignals(false);
}

void ToolBarRenderSettings::onFocusViewport(IGuiData* data)
{
	GuiDataFocusViewport* castData = static_cast<GuiDataFocusViewport*>(data);
	if (castData->m_forceFocus && castData->m_camera)
		m_focusCamera = castData->m_camera;
}

void ToolBarRenderSettings::blockAllSignals(bool block)
{
	m_ui.transparencySpinBox->blockSignals(block);
	m_ui.transparencySlider->blockSignals(block);
	m_ui.transparencyCheckBox->blockSignals(block);
	m_ui.brightnessLuminanceSpinBox->blockSignals(block);
	m_ui.brightnessLuminanceSlider->blockSignals(block);
	m_ui.contrastSaturationSpinBox->blockSignals(block);
	m_ui.contrastSaturationSlider->blockSignals(block);
	m_ui.comboBox_renderMode->blockSignals(block);
	m_ui.brightnessLuminanceSpinBox->blockSignals(block);
	m_ui.brightnessLuminanceSlider->blockSignals(block);
	m_ui.falseColorSpinBox->blockSignals(block);
	m_ui.falseColorSlider->blockSignals(block);
	m_ui.spinBox_pointSize->blockSignals(block);
	m_ui.comboBox_pointShape->blockSignals(block);
	m_ui.doubleSpinBox_splatRadius->blockSignals(block);
	m_ui.comboBox_gapFillingStrength->blockSignals(block);
	m_ui.contrastSaturationSpinBox->blockSignals(block);
	m_ui.contrastSaturationSlider->blockSignals(block);
	m_ui.alphaObjectsSpinBox->blockSignals(block);
	m_ui.alphaObjectsSlider->blockSignals(block);
	m_ui.lineEdit_rampMax->blockSignals(block);
	m_ui.lineEdit_rampMin->blockSignals(block);
	m_ui.spinBox_rampStep->blockSignals(block);

	m_ui.checkBox_normals->blockSignals(block);
	m_ui.slider_normals->blockSignals(block);
	m_ui.spinBox_normals->blockSignals(block);
}

void ToolBarRenderSettings::switchRenderMode(const int& mode)
{
	m_currentRenderMode = UiRenderMode(mode);
	switch (m_currentRenderMode)
	{
        case UiRenderMode::Flat:
        {
            m_ui.adjust_options->hide();
            m_ui.ramp_options->setVisible(false);
            m_ui.pushButton_color->show();
			enableFalseColor(false);
        }
        break;
		case UiRenderMode::Flat_Distance_Ramp:
        case UiRenderMode::Distance_Ramp:
        {
            showContrastBrightness();
            m_ui.ramp_options->setVisible(true);
            m_ui.pushButton_color->hide();
			enableFalseColor(false);
        }
        break;
		case UiRenderMode::Grey_Colored:
        {
            showContrastBrightness();
            m_ui.ramp_options->setVisible(false);
            m_ui.pushButton_color->show();
			enableFalseColor(false);
        }
        break;
        case UiRenderMode::Scans_Color:
        case UiRenderMode::Clusters_Color:
        {
            showContrastBrightness();
            m_ui.ramp_options->setVisible(false);
            m_ui.pushButton_color->hide();
			enableFalseColor(false);
        }
        break;
		case UiRenderMode::Intensity:
		{
            showContrastBrightness();
            m_ui.ramp_options->setVisible(false);
            m_ui.pushButton_color->hide();
			enableFalseColor(false);
		}
		break;
		case UiRenderMode::RGB:
		{
            showSaturationLuminance();
            m_ui.ramp_options->setVisible(false);
			m_ui.pushButton_color->hide();
			enableFalseColor(false);
		}
		break;
		case UiRenderMode::IntensityRGB_Combined:
		case UiRenderMode::Fake_Color:
		{
			showSaturationLuminance();
			m_ui.ramp_options->setVisible(false);
			m_ui.pushButton_color->hide();
			enableFalseColor(true);
		}
		break;
	}
	//adjustSize();
}

void ToolBarRenderSettings::setDisplayPresetNames(const QStringList& names, const QString& selectedName)
{
	const QSignalBlocker blocker(m_ui.comboBox_displayPresets);
	m_ui.comboBox_displayPresets->clear();
	m_ui.comboBox_displayPresets->addItems(names);
	setDisplayPresetSelection(selectedName);
}

void ToolBarRenderSettings::setDisplayPresetSelection(const QString& name)
{
	const int index = m_ui.comboBox_displayPresets->findText(name);
	if (index >= 0)
		m_ui.comboBox_displayPresets->setCurrentIndex(index);
	m_ui.pushButton_editDisplayPresets->setEnabled(m_ui.comboBox_displayPresets->currentText() != "Initial");
}

QString ToolBarRenderSettings::currentDisplayPresetName() const
{
	return m_ui.comboBox_displayPresets->currentText();
}

void ToolBarRenderSettings::showContrastBrightness()
{
    if (!m_intensityActive)
    {
        m_saturation = m_ui.contrastSaturationSpinBox->value();
        m_lumiance = m_ui.brightnessLuminanceSpinBox->value();
        m_intensityActive = true;
        m_ui.contrastSaturationSpinBox->setValue(m_contrast);
        m_ui.brightnessLuminanceSpinBox->setValue(m_brightness);
    }

    m_ui.adjust_options->show();
    m_ui.contrastSaturationLabel->setText(TEXT_RENDER_CONTRAST);
    m_ui.brightnessLuminanceLabel->setText(TEXT_RENDER_BRIGHTNESS);
}

void ToolBarRenderSettings::showSaturationLuminance()
{
    if (m_intensityActive)
    {
        m_contrast = m_ui.contrastSaturationSpinBox->value();
        m_brightness = m_ui.brightnessLuminanceSpinBox->value();
        m_intensityActive = false;
        m_ui.contrastSaturationSpinBox->setValue(m_saturation);
        m_ui.brightnessLuminanceSpinBox->setValue(m_lumiance);
    }

    m_ui.adjust_options->show();
    m_ui.contrastSaturationLabel->setText(TEXT_RENDER_SATURATION);
    m_ui.brightnessLuminanceLabel->setText(TEXT_RENDER_LUMINANCE);
}

void ToolBarRenderSettings::enableFalseColor(bool enable)
{
	m_ui.falseColorSlider->setEnabled(enable);
	m_ui.falseColorSpinBox->setEnabled(enable);
}

void ToolBarRenderSettings::sendTransparency()
{
	int uiTransparency = m_ui.transparencySlider->value();
	float t = ui::transparency::uiValue_to_trueValue(uiTransparency);
	BlendMode blend = m_ui.transparencyCheckBox->isChecked() ? BlendMode::Transparent : BlendMode::Opaque;
	m_dataDispatcher.updateInformation(new GuiDataRenderTransparency(blend, t, m_focusCamera), this);
}

void ToolBarRenderSettings::slotBrightnessLuminanceValueChanged(int value)
{
	if(m_intensityActive)
		m_dataDispatcher.updateInformation(new GuiDataRenderBrightness(value, m_focusCamera), this);
	else
		m_dataDispatcher.updateInformation(new GuiDataRenderLuminance(value, m_focusCamera), this);
}

void ToolBarRenderSettings::slotContrastSaturationValueChanged(int value)
{
	if (m_intensityActive)
		m_dataDispatcher.updateInformation(new GuiDataRenderContrast(value, m_focusCamera), this);
	else
		m_dataDispatcher.updateInformation(new GuiDataRenderSaturation(value, m_focusCamera), this);
}

void ToolBarRenderSettings::slotFakeColorValueChanged(int value)
{
	m_dataDispatcher.updateInformation(new GuiDataRenderBlending(value, m_focusCamera), this);
}

void ToolBarRenderSettings::slotTranparencyActivationChanged(int value)
{
	m_ui.transparencySpinBox->setEnabled(value > 0);
	m_ui.transparencySlider->setEnabled(value > 0);
	sendTransparency();
}

void ToolBarRenderSettings::slotTransparencyValueChanged(int value)
{
	sendTransparency();
}

void ToolBarRenderSettings::slotSetPointSize(int pointSize)
{
    m_dataDispatcher.sendControl(new control::application::SetRenderPointSize(pointSize, m_focusCamera));
}

void ToolBarRenderSettings::slotSetPointShape(int index)
{
	m_pointShape = static_cast<PointShape>(m_ui.comboBox_pointShape->itemData(index).toInt());
	const bool splatEnabled = (m_pointShape == PointShape::Splat);

	if (splatEnabled)
	{
		m_savedGapFillingIndex = m_ui.comboBox_gapFillingStrength->currentIndex();
		int offIndex = m_ui.comboBox_gapFillingStrength->findData(9);
		if (offIndex != -1 && offIndex != m_ui.comboBox_gapFillingStrength->currentIndex())
		{
			m_ui.comboBox_gapFillingStrength->setCurrentIndex(offIndex);
		}
	}
	else
	{
		m_ui.comboBox_gapFillingStrength->setCurrentIndex(m_savedGapFillingIndex);
	}

	m_ui.comboBox_gapFillingStrength->setEnabled(!splatEnabled);
	m_ui.label_gapFilling->setEnabled(!splatEnabled);
	m_ui.doubleSpinBox_splatRadius->setEnabled(splatEnabled);

	m_dataDispatcher.updateInformation(new GuiDataRenderPointShape(m_pointShape, m_focusCamera), this);
}

void ToolBarRenderSettings::slotSetSplatRadius(double radius)
{
	m_dataDispatcher.updateInformation(new GuiDataRenderSplatRadius(static_cast<float>(radius), m_focusCamera), this);
}

void ToolBarRenderSettings::slotSetTexelThreshold(int index)
{
	int texelThreshold = m_ui.comboBox_gapFillingStrength->itemData(index).toInt();
	m_dataDispatcher.updateInformation(new GuiDataRenderTexelThreshold(texelThreshold, m_focusCamera), this);
}

void ToolBarRenderSettings::slotSetRenderMode(int mode)
{
	switchRenderMode(m_ui.comboBox_renderMode->currentData().toInt());

	m_dataDispatcher.sendControl(new control::application::RenderModeUpdate(UiRenderMode(m_ui.comboBox_renderMode->currentData().toInt()), m_focusCamera));	
}

void ToolBarRenderSettings::slotColorPicking()
{
	QColor newColor = QColorDialog::getColor(m_selectedColor, this);
	if (newColor.isValid())
	{
		m_selectedColor = newColor;
		m_ui.pushButton_color->setPalette(QPalette(m_selectedColor));
		m_dataDispatcher.updateInformation(new GuiDataRenderFlatColor(m_selectedColor.redF(), m_selectedColor.greenF(), m_selectedColor.blueF(), m_focusCamera), this);
	}
}

void ToolBarRenderSettings::hideTransparencyNormalsControls()
{
	m_ui.transparencyCheckBox->hide();
	m_ui.transparencySlider->hide();
	m_ui.transparencySpinBox->hide();
	m_ui.checkBox_normals->hide();
	m_ui.slider_normals->hide();
	m_ui.spinBox_normals->hide();
}

bool ToolBarRenderSettings::rampValidValue(float& min, float& max, int& step)
{
	min = m_ui.lineEdit_rampMin->getValue();
	max = m_ui.lineEdit_rampMax->getValue();

	step = m_ui.spinBox_rampStep->value();
	if (!step)
		step = 0xFFFF;
	return true;
}

void ToolBarRenderSettings::slotRampValues()
{
    float minV, maxV;
	int step;
	if (rampValidValue(minV, maxV, step))
	{
		m_dataDispatcher.updateInformation(new GuiDataRenderDistanceRampValues(minV, maxV, step, m_focusCamera), this);
	}
}

void ToolBarRenderSettings::slotNormalsChanged()
{
	PostRenderingNormals lighting = {};
	bool enable = m_ui.checkBox_normals->isChecked();
	m_ui.slider_normals->setEnabled(enable);
	m_ui.spinBox_normals->setEnabled(enable);

	lighting.show = enable;
	lighting.inverseTone = (m_ui.checkBox_normals->checkState() == Qt::CheckState::PartiallyChecked);
	lighting.normalStrength = m_ui.spinBox_normals->value() / 100.f;
	m_dataDispatcher.updateInformation(new GuiDataPostRenderingNormals(lighting, true, m_focusCamera), this);
}

void ToolBarRenderSettings::slotAlphaBoxesValueChanged(int value)
{
	m_dataDispatcher.updateInformation(new GuiDataAlphaObjectsRendering(1.0f - (value / 100.0f), m_focusCamera), this);
}

void ToolBarRenderSettings::slotDisplayPresetSelectionChanged(int index)
{
	Q_UNUSED(index)
	const QString selected = m_ui.comboBox_displayPresets->currentText();
	m_ui.pushButton_editDisplayPresets->setEnabled(selected != "Initial");
	emit displayPresetSelectionChanged(selected);
}

void ToolBarRenderSettings::slotDisplayPresetNew()
{
	emit displayPresetNewRequested();
}

void ToolBarRenderSettings::slotDisplayPresetEdit()
{
	emit displayPresetEditRequested(m_ui.comboBox_displayPresets->currentText());
}
