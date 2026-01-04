#include "gui/Dialog/DialogSettings.h"
#include "controller/controls/ControlApplication.h"
#include "controller/controls/ControlProjectTemplate.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/IDataDispatcher.h"
#include "gui/UnitConverter.h"
#include "gui/Translator.h"
#include "models/project/ProjectTypes.h"
#include "models/3d/OpticalFunctions.h"
#include "utils/Config.h"
#include "utils/QtLogStream.hpp"
#include "gui/texts/SettingsTexts.hpp"
#include "gui/texts/FileSystemTexts.hpp"

#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qcolordialog.h>

static std::vector<UnitType> s_settingUnits = { UnitType::M, UnitType::CM, UnitType::MM, UnitType::YD, UnitType::FT, UnitType::INC };
static std::vector<UnitType> s_settingVolumeUnits = { UnitType::M3, UnitType::LITRE};

DialogSettings::DialogSettings(IDataDispatcher& dataDispatcher, QWidget* parent)
	: ADialog(dataDispatcher, parent)
	, m_guizmoParams(dataDispatcher, this)
{
	m_guizmoParams.hide();
	m_ui.setupUi(this);

	for (const auto& lang : Translator::getAvailableLanguage())
		m_ui.langageComboBox->addItem(Translator::getLanguageQStr(lang), QVariant((uint32_t)lang));

	m_ui.langageComboBox->setCurrentIndex((int)Translator::getActiveLanguage());

	for (UnitType unit : s_settingUnits) {
		QString text = UnitConverter::getUnitText(unit);
		m_ui.distanceUnitBox->addItem(text, QVariant((uint32_t)unit));
		m_ui.diameterUnitBox->addItem(text, QVariant((uint32_t)unit));
	}

	for(UnitType vUnit : s_settingVolumeUnits)
		m_ui.volumeUnitBox->addItem(UnitConverter::getUnitText(vUnit), QVariant((uint32_t)vUnit));


	m_ui.autosaveTimingComboBox->addItem(QString(""), QVariant(0));
	for (const uint8_t& time : Config::Timing)
		m_ui.autosaveTimingComboBox->addItem(TEXT_SETTINGS_MINUTES.arg(time), QVariant(time));

	m_ui.lineEdit_temporary->setVisible(false);
	m_ui.tempLabel->setVisible(false);
	m_ui.toolButton_temporary->setVisible(false);

	m_ui.lineEdit_minPersp->setType(NumericType::DISTANCE);
	m_ui.lineEdit_maxPersp->setType(NumericType::DISTANCE);
	m_ui.examineMinimumDistanceLineEdit->setType(NumericType::DISTANCE);
	m_ui.lineEdit_limitOrtho->setType(NumericType::DISTANCE);

	connect(m_ui.okButton, &QPushButton::released, this, &DialogSettings::onOk);
	connect(m_ui.langageComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DialogSettings::onLanguageChanged);
	connect(m_ui.spinBox_digits, QOverload<int>::of(&QSpinBox::valueChanged), this, &DialogSettings::onUnitUsageChanged);
	connect(m_ui.distanceUnitBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DialogSettings::onUnitUsageChanged);
	connect(m_ui.diameterUnitBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DialogSettings::onUnitUsageChanged);
	connect(m_ui.volumeUnitBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DialogSettings::onUnitUsageChanged);
	connect(m_ui.toolButton_folder, &QToolButton::clicked, this, &DialogSettings::onSelectProjsFolder);
	connect(m_ui.toolButton_temporary, &QToolButton::clicked, this, &DialogSettings::onSelectTempFolder);
	connect(m_ui.toolButton_ffmpegFolder, &QToolButton::clicked, this, &DialogSettings::onSelectFfmpegFolder);
	connect(m_ui.lineEdit_project, &QLineEdit::editingFinished, this, &DialogSettings::onProjsFolder);
	connect(m_ui.lineEdit_temporary, &QLineEdit::editingFinished, this, &DialogSettings::onTempFolder);
	connect(m_ui.lineEdit_ffmpeg, &QLineEdit::editingFinished, this, &DialogSettings::onFfmpegFolder);
	connect(m_ui.pushButton_color, &QPushButton::clicked, this, &DialogSettings::onColorPicking);
	connect(m_ui.templateProjectBtn, &QToolButton::clicked, [this]() 
		{
			m_dataDispatcher.updateInformation(new GuiDataProjectTemplateManagerDisplay());
			m_dataDispatcher.sendControl(new control::projectTemplate::SendList()); 
			hide(); 
		});
	connect(m_ui.highestIndexRadioButton, &QRadioButton::clicked, this, &DialogSettings::onIndexationMethodChoice);
	connect(m_ui.fillMissingRadioButton, &QRadioButton::clicked , this, &DialogSettings::onIndexationMethodChoice);
	connect(m_ui.noDecimationRB, &QRadioButton::clicked, this, &DialogSettings::onDecimationChanged);
	connect(m_ui.constantDecimationRB, &QRadioButton::clicked, this, &DialogSettings::onDecimationChanged);
	connect(m_ui.dynamicDecimationRB, &QRadioButton::clicked, this, &DialogSettings::onDecimationChanged);
	connect(m_ui.constantDecimationSlider, &QSlider::valueChanged, this, &DialogSettings::onDecimationChanged);
	connect(m_ui.minDecimationValue, &QLineEdit::editingFinished, this, &DialogSettings::onDecimationChanged);
	connect(m_ui.centeringTargetExamineCheckBox, &QCheckBox::stateChanged, this, &DialogSettings::onExamineOptions);
	connect(m_ui.keepExamineOnPanBox, &QCheckBox::stateChanged, this, &DialogSettings::onExamineOptions);
	connect(m_ui.framelessCheckBox, &QCheckBox::stateChanged, this, &DialogSettings::onFramelessChanged);
	connect(m_ui.examineDisplayCheckBox, &QCheckBox::stateChanged, this, &DialogSettings::onExamineDisplayChanged);
	connect(m_ui.autosaveActivateCheckBox, &QCheckBox::stateChanged, this, &DialogSettings::onAutosaveCheckboxChanged); 
	connect(m_ui.autosaveTimingComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DialogSettings::onAutosaveComboxBoxChanged);
	connect(m_ui.manipulatorSizeSlider, &QSlider::valueChanged, this, &DialogSettings::onManipulatorSizeChanged);
	connect(m_ui.manipulatorSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &DialogSettings::onManipulatorSizeChanged);
	connect(m_ui.toolButtonGuizmo, &QToolButton::clicked, [this]()
		{
			m_guizmoParams.exec();
		});
	connect(m_ui.examineMinimumDistanceLineEdit, &QDoubleEdit::editingFinished, this, &DialogSettings::onNavigationParametersChanged);
	connect(m_ui.examineRotationSlider, &QSlider::valueChanged, this, [this](double value) { m_ui.examineRotationSpinBox->setValue(value / 100.); onNavigationParametersChanged(); });
	connect(m_ui.translationSpeedSlider, &QSlider::valueChanged, this, [this](double value) { m_ui.translationSpeedSpinBox->setValue(value / 100.); onNavigationParametersChanged(); });
	connect(m_ui.examineRotationSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) { m_ui.examineRotationSlider->setValue(value * 100); onNavigationParametersChanged(); });
	connect(m_ui.translationSpeedSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) { m_ui.translationSpeedSlider->setValue(value * 100); onNavigationParametersChanged(); });
	connect(m_ui.mouseCheckBox, &QCheckBox::stateChanged, this, &DialogSettings::onNavigationParametersChanged);
	connect(m_ui.invertCameraRotationCheckBox, &QCheckBox::stateChanged, this, &DialogSettings::onNavigationParametersChanged);

	connect(m_ui.slider_perspDistances, &QSlider::valueChanged, this, &DialogSettings::onRenderPerspDistanceChanged);
	connect(m_ui.slider_orthoDistances, &QSlider::valueChanged, this, &DialogSettings::onRenderOrthoDistanceChanged);

	connect(m_ui.unlockScansCheckBox, &QCheckBox::stateChanged, this, &DialogSettings::onUnlockScansManipulation);

	m_ui.okButton->setFocus();

    m_dataDispatcher.registerObserverOnKey(this, guiDType::renderDecimationOptions);

    initConfigValues();
	adjustSize();
}

DialogSettings::~DialogSettings()
{}

void DialogSettings::informData(IGuiData* idata)
{
    switch (idata->getType())
    {
    case guiDType::renderDecimationOptions:
        m_savedDecimationOptions = static_cast<GuiDataRenderDecimationOptions*>(idata)->m_options;
        showDecimationOptions();
        break;
    default:
        break;
    }
}

void DialogSettings::initComboBox(QComboBox* comboBox, int currentData)
{
	for (int iterator(0); iterator < comboBox->count(); iterator++)
	{
		if (comboBox->itemData(iterator).toInt() == currentData)
		{
			comboBox->setCurrentIndex(iterator);
			break;
		}
	}
}

void DialogSettings::initConfigValues()
{
	blockSignal(true);

	Color32 color = Config::getUserColor();
	m_ui.pushButton_color->setPalette(QPalette(QColor(color.r, color.g, color.b)));
	m_ui.centeringTargetExamineCheckBox->setChecked(Config::getCenteringConfiguration());
	UnitUsage valueDisplayParameters = Config::getUnitUsageConfiguration();

	m_ui.spinBox_digits->setValue(valueDisplayParameters.displayedDigits);

	initComboBox(m_ui.diameterUnitBox, (int)valueDisplayParameters.diameterUnit);
	initComboBox(m_ui.distanceUnitBox, (int)valueDisplayParameters.distanceUnit);
	initComboBox(m_ui.volumeUnitBox, (int)valueDisplayParameters.volumeUnit);

    m_ui.lineEdit_project->setText(QString::fromStdWString(Config::getProjectsPath().wstring()));
    m_ui.lineEdit_temporary->setText(QString::fromStdWString(Config::getTemporaryPath().wstring()));
    m_ui.lineEdit_ffmpeg->setText(QString::fromStdWString(Config::getFFmpegPath().wstring()));
    m_ui.framelessCheckBox->setChecked(Config::getMaximizedFrameless());
	m_ui.keepExamineOnPanBox->setChecked(Config::getKeepingExamineConfiguration());
    m_ui.examineDisplayCheckBox->setChecked(Config::getExamineDisplayMode());
	m_ui.autosaveTimingComboBox->setEnabled(Config::getIsAutoSaveActive());
	m_ui.autosaveActivateCheckBox->setChecked(Config::getIsAutoSaveActive());
	m_ui.fillMissingRadioButton->setChecked(Config::getIndexationMethod() == IndexationMethod::FillMissingIndex);
	m_ui.unlockScansCheckBox->setChecked(Config::isUnlockScanManipulation());

	int manipulatorSize = (int)Config::getManipulatorSize();
	m_ui.manipulatorSizeSlider->setValue(manipulatorSize);
	m_ui.manipulatorSizeSpinBox->setValue(manipulatorSize);

	uint8_t activeTime(Config::getAutoSaveTiming());
	if (activeTime)
		initComboBox(m_ui.autosaveTimingComboBox, activeTime);

	updateUnitValues();

	blockSignal(false);
}

void DialogSettings::updateUnitValues()
{
	PerspectiveZBounds zBounds = Config::getPerspectiveZBounds();
	OrthographicZBounds orthoBounds = Config::getOrthographicZBounds();

	m_ui.lineEdit_minPersp->setValue(tls::getNearValue(zBounds));
	m_ui.lineEdit_maxPersp->setValue(tls::getFarValue(zBounds));

	m_ui.slider_perspDistances->setMinimum(c_min_near_plan_log2);
	m_ui.slider_perspDistances->setMaximum(c_max_near_plan_log2);
	m_ui.slider_perspDistances->setValue(zBounds.near_plan_log2);
	m_ui.slider_orthoDistances->setMinimum(c_min_ortho_range_log2);
	m_ui.slider_orthoDistances->setMaximum(c_max_ortho_range_log2);
	m_ui.slider_orthoDistances->setValue(orthoBounds);

	NavigationParameters navParams = Config::getNavigationParameters();
	m_ui.examineMinimumDistanceLineEdit->setValue(navParams.examineMinimumRadius);
	m_ui.translationSpeedSlider->setValue(navParams.cameraTranslationSpeedFactor);
	m_ui.translationSpeedSpinBox->setValue(navParams.cameraTranslationSpeedFactor / 100.);
	m_ui.examineRotationSlider->setValue(navParams.cameraRotationExamineFactor);
	m_ui.examineRotationSpinBox->setValue(navParams.cameraRotationExamineFactor / 100.);
	m_ui.mouseCheckBox->setChecked(navParams.wheelInverted);
	m_ui.invertCameraRotationCheckBox->setChecked(navParams.mouseDragInverted);
}

void DialogSettings::blockSignal(bool active)
{
	m_ui.fillMissingRadioButton->blockSignals(active);
	m_ui.autosaveTimingComboBox->blockSignals(active);
	m_ui.autosaveActivateCheckBox->blockSignals(active);
	m_ui.keepExamineOnPanBox->blockSignals(active);
	m_ui.centeringTargetExamineCheckBox->blockSignals(active);
	m_ui.mouseCheckBox->blockSignals(active);
	m_ui.spinBox_digits->blockSignals(active);
	m_ui.lineEdit_project->blockSignals(active);
	m_ui.lineEdit_temporary->blockSignals(active);
	m_ui.lineEdit_ffmpeg->blockSignals(active);
	m_ui.manipulatorSizeSlider->blockSignals(active);
	m_ui.manipulatorSizeSpinBox->blockSignals(active);
	m_ui.pushButton_color->blockSignals(active);
	m_ui.framelessCheckBox->blockSignals(active);

	m_ui.examineMinimumDistanceLineEdit->blockSignals(active);
	m_ui.translationSpeedSlider->blockSignals(active);
	m_ui.examineRotationSlider->blockSignals(active);
}

void DialogSettings::showEvent(QShowEvent* event)
{
	m_ui.langageComboBox->blockSignals(true);
	m_ui.langageComboBox->setCurrentIndex((int)Translator::getActiveLanguage());
	m_ui.langageComboBox->blockSignals(false);
}

QCheckBox* DialogSettings::getFramelessCheckBox()
{
    return m_ui.framelessCheckBox;
}

void DialogSettings::onLanguageChanged(const uint32_t& index)
{
	//fixme (Aurélien) for dynamic lang modification
	//m_translator->setActiveLangage((Translator::LangageType)m_ui.langageComboBox->currentData().toInt());
}

void DialogSettings::onUnitUsageChanged()
{
	UnitUsage valueDisplayParameters = { (UnitType)m_ui.distanceUnitBox->currentData().toInt(), (UnitType)m_ui.diameterUnitBox->currentData().toInt(), (UnitType)m_ui.volumeUnitBox->currentData().toInt(), (uint32_t)m_ui.spinBox_digits->value() };
	m_dataDispatcher.sendControl(new control::application::SetValueSettingsDisplay(valueDisplayParameters));
}

void DialogSettings::onSelectTempFolder()
{
	QString folderPath = QFileDialog::getExistingDirectory(this, TEXT_SELECT_DIRECTORY, m_ui.lineEdit_temporary->text()
		, QFileDialog::ShowDirsOnly);

	if (folderPath != "")
	{
		m_ui.lineEdit_temporary->setText(folderPath);
		m_dataDispatcher.sendControl(new control::application::SetTemporaryFolder(std::filesystem::path(folderPath.toStdWString())));
	}
}

void DialogSettings::onSelectProjsFolder()
{
	QString folderPath = QFileDialog::getExistingDirectory(this, TEXT_SELECT_DIRECTORY, m_ui.lineEdit_project->text()
		, QFileDialog::ShowDirsOnly);

	if (folderPath != "")
	{
		m_ui.lineEdit_project->setText(folderPath);
		m_dataDispatcher.sendControl(new control::application::SetProjectsFolder(std::filesystem::path(folderPath.toStdWString())));
	}
}

void DialogSettings::onTempFolder()
{
	m_dataDispatcher.sendControl(new control::application::SetTemporaryFolder(std::filesystem::path(m_ui.lineEdit_temporary->text().toStdWString())));
}

void DialogSettings::onProjsFolder()
{
	m_dataDispatcher.sendControl(new control::application::SetProjectsFolder(std::filesystem::path(m_ui.lineEdit_project->text().toStdWString())));
}

void DialogSettings::onSelectFfmpegFolder()
{
	QString folderPath = QFileDialog::getExistingDirectory(this, TEXT_SELECT_DIRECTORY, m_ui.lineEdit_ffmpeg->text()
		, QFileDialog::ShowDirsOnly);

	if (folderPath != "")
	{
		m_ui.lineEdit_ffmpeg->setText(folderPath);
		m_dataDispatcher.sendControl(new control::application::SetFFmpegFolder(std::filesystem::path(folderPath.toStdWString())));
	}
}

void DialogSettings::onFfmpegFolder()
{
	m_dataDispatcher.sendControl(new control::application::SetFFmpegFolder(std::filesystem::path(m_ui.lineEdit_ffmpeg->text().toStdWString())));
}

void DialogSettings::onColorPicking()
{
	QColor selectedColor = QColorDialog::getColor(QColorFromColor32(Config::getUserColor()));
	if (selectedColor.isValid())
	{
		m_ui.pushButton_color->setPalette(QPalette(selectedColor));
		m_dataDispatcher.sendControl(new control::application::SetUserColor(Color32(selectedColor.red(), selectedColor.green(), selectedColor.blue())));
	}
}

void DialogSettings::onOk()
{
    LanguageType type((LanguageType)m_ui.langageComboBox->currentData().toInt());
    if (type != Translator::getActiveLanguage())
    {
        m_dataDispatcher.sendControl(new control::application::SetLanguage(type));
    }
    // Actualize the decimation options if they have been changed
    sendDecimationOptions();

    hide();
}

void DialogSettings::onCancel()
{
    // reset the values
    showDecimationOptions();
    hide();
}

void DialogSettings::onDecimationChanged()
{
    m_ui.constantDecimationSlider->setEnabled(m_ui.constantDecimationRB->isChecked());
    m_ui.minDecimationValue->setEnabled(m_ui.dynamicDecimationRB->isChecked());
    m_ui.constantDecimationLabel->setText(QString("%1%").arg(m_ui.constantDecimationSlider->value()));
}

void DialogSettings::onExamineOptions()
{
	m_dataDispatcher.sendControl(new control::application::SetExamineOptions(m_ui.centeringTargetExamineCheckBox->isChecked(), m_ui.keepExamineOnPanBox->isChecked()));
}

void DialogSettings::showDecimationOptions()
{
    m_ui.noDecimationRB->setChecked(m_savedDecimationOptions.mode == DecimationMode::None);
    m_ui.constantDecimationRB->setChecked(m_savedDecimationOptions.mode == DecimationMode::Constant);
    m_ui.dynamicDecimationRB->setChecked(m_savedDecimationOptions.mode == DecimationMode::Adaptive);

    m_ui.constantDecimationSlider->setValue(m_savedDecimationOptions.constantValue * 100);
    m_ui.minDecimationValue->setText(QString::number(m_savedDecimationOptions.dynamicMin * 100));

    m_ui.constantDecimationSlider->setEnabled(m_ui.constantDecimationRB->isChecked());
    m_ui.minDecimationValue->setEnabled(m_ui.dynamicDecimationRB->isChecked());
    m_ui.constantDecimationLabel->setText(QString("%1%").arg(m_ui.constantDecimationSlider->value()));
}

void DialogSettings::sendDecimationOptions()
{
    DecimationOptions options;
    if (m_ui.noDecimationRB->isChecked())
    {
        options.mode = DecimationMode::None;
    }
    else if (m_ui.constantDecimationRB->isChecked())
    {
        options.mode = DecimationMode::Constant;
    }
    else if (m_ui.dynamicDecimationRB->isChecked())
    {
        options.mode = DecimationMode::Adaptive;
    }

    // Get all the values
    options.constantValue = m_ui.constantDecimationSlider->value() / 100.f;
    bool valid = false;
    options.dynamicMin = m_ui.minDecimationValue->text().toFloat(&valid) / 100.f;
    if (!valid)
        options.dynamicMin = m_savedDecimationOptions.dynamicMin;

    // Send a control only when the options have changed
    bool optionsChanged = false;
    optionsChanged |= options.mode != m_savedDecimationOptions.mode;
    optionsChanged |= options.constantValue != m_savedDecimationOptions.constantValue;
    optionsChanged |= options.dynamicMin != m_savedDecimationOptions.dynamicMin;

    if (optionsChanged)
    {
        m_savedDecimationOptions = options;
        m_dataDispatcher.sendControl(new control::application::SetDecimationOptions(options, true, true));
    }
}

void DialogSettings::onFramelessChanged()
{
    m_dataDispatcher.sendControl(new control::application::SetFramelessMode(m_ui.framelessCheckBox->isChecked(), true));
}

void DialogSettings::onExamineDisplayChanged()
{
    m_dataDispatcher.sendControl(new control::application::SetExamineDisplayMode(m_ui.examineDisplayCheckBox->isChecked(), true));
}

void DialogSettings::onAutosaveComboxBoxChanged()
{
	m_dataDispatcher.sendControl(new control::application::SetAutoSaveParameters(true, m_ui.autosaveTimingComboBox->currentData().toInt(), true));
}

void DialogSettings::onIndexationMethodChoice()
{
	m_dataDispatcher.sendControl(new control::application::SetIndexationMethod(m_ui.fillMissingRadioButton->isChecked() ? IndexationMethod::FillMissingIndex : IndexationMethod::HighestIndex, true));
}

void DialogSettings::onManipulatorSizeChanged(int manipulatorSize)
{
	m_ui.manipulatorSizeSlider->setValue(manipulatorSize);
	m_ui.manipulatorSizeSpinBox->setValue(manipulatorSize);

	m_dataDispatcher.sendControl(new control::application::SetManipulatorSize(manipulatorSize, true));
}

void DialogSettings::onNavigationParametersChanged()
{
	NavigationParameters navParams;
	navParams.cameraRotationExamineFactor = m_ui.examineRotationSlider->value();
	navParams.cameraTranslationSpeedFactor = m_ui.translationSpeedSlider->value();
	navParams.examineMinimumRadius = m_ui.examineMinimumDistanceLineEdit->getValue();
	navParams.mouseDragInverted = m_ui.invertCameraRotationCheckBox->isChecked();
	navParams.wheelInverted = m_ui.mouseCheckBox->isChecked();

	m_dataDispatcher.sendControl(new control::application::SetNavigationParameters(navParams, true));
}

void DialogSettings::onRenderPerspDistanceChanged()
{
	PerspectiveZBounds zBounds;
	zBounds.near_plan_log2 = m_ui.slider_perspDistances->value();
	zBounds.near_far_ratio_log2 = c_max_near_far_ratio_log2;

	m_ui.lineEdit_minPersp->setValue(tls::getNearValue(zBounds));
	m_ui.lineEdit_maxPersp->setValue(tls::getFarValue(zBounds));

	m_dataDispatcher.sendControl(new control::application::SetPerspectiveZBounds(zBounds, true));
}

void DialogSettings::onRenderOrthoDistanceChanged()
{
	OrthographicZBounds orthoBound;
	orthoBound = m_ui.slider_orthoDistances->value();

	m_ui.lineEdit_limitOrtho->setValue(tls::getOrthographicZBoundsValue(orthoBound));

	m_dataDispatcher.sendControl(new control::application::SetOrthographicZBounds(orthoBound, true));
}

void DialogSettings::onUnlockScansManipulation()
{
	m_dataDispatcher.sendControl(new control::application::UnlockScanManipulation(m_ui.unlockScansCheckBox->isChecked()));
}

void DialogSettings::onAutosaveCheckboxChanged()
{
	m_ui.autosaveTimingComboBox->setEnabled(m_ui.autosaveActivateCheckBox->isChecked());
	if(!m_ui.autosaveActivateCheckBox->isChecked())
		m_dataDispatcher.sendControl(new control::application::SetAutoSaveParameters(false, 0, true));
}
