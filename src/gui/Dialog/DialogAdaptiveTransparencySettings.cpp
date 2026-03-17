#include "gui/Dialog/DialogAdaptiveTransparencySettings.h"

#include "gui/widgets/CustomWidgets/qdoubleedit.h"

DialogAdaptiveTransparencySettings::DialogAdaptiveTransparencySettings(const AdaptiveTransparencySettings& settings, UnitType distanceUnit, QWidget* parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);

    m_ui.lineEdit_transparencyNear->setRange(1, 100);
    m_ui.lineEdit_transparencyFar->setRange(1, 100);

    m_ui.lineEdit_transparencyDistNear->setType(NumericType::DISTANCE);
    m_ui.lineEdit_transparencyDistFar->setType(NumericType::DISTANCE);
    m_ui.lineEdit_transparencyDistNear->setUnit(distanceUnit);
    m_ui.lineEdit_transparencyDistFar->setUnit(distanceUnit);
    m_ui.lineEdit_transparencyDistNear->setRules(ANumericLineEdit::LineEditRules::PositiveStrict);
    m_ui.lineEdit_transparencyDistFar->setRules(ANumericLineEdit::LineEditRules::PositiveStrict);

    connect(m_ui.lineEdit_transparencyDistNear, &QLineEdit::editingFinished, this, &DialogAdaptiveTransparencySettings::onDistanceNearEdited);
    connect(m_ui.lineEdit_transparencyDistFar, &QLineEdit::editingFinished, this, &DialogAdaptiveTransparencySettings::onDistanceFarEdited);

    connect(m_ui.okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_ui.cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    applySettingsToUi(settings);
}

AdaptiveTransparencySettings DialogAdaptiveTransparencySettings::getSettings() const
{
    AdaptiveTransparencySettings settings;
    settings.transparencyNear = static_cast<float>(m_ui.lineEdit_transparencyNear->value());
    settings.transparencyFar = static_cast<float>(m_ui.lineEdit_transparencyFar->value());
    settings.distanceNear = static_cast<float>(m_ui.lineEdit_transparencyDistNear->getValue());
    settings.distanceFar = static_cast<float>(m_ui.lineEdit_transparencyDistFar->getValue());
    return settings;
}

void DialogAdaptiveTransparencySettings::onDistanceNearEdited()
{
    enforceDistanceConstraint(true);
}

void DialogAdaptiveTransparencySettings::onDistanceFarEdited()
{
    enforceDistanceConstraint(false);
}

void DialogAdaptiveTransparencySettings::applySettingsToUi(const AdaptiveTransparencySettings& settings)
{
    m_ui.lineEdit_transparencyNear->setValue(static_cast<int>(settings.transparencyNear));
    m_ui.lineEdit_transparencyFar->setValue(static_cast<int>(settings.transparencyFar));
    m_ui.lineEdit_transparencyDistNear->setValue(settings.distanceNear);
    m_ui.lineEdit_transparencyDistFar->setValue(settings.distanceFar);
    enforceDistanceConstraint(true);
}

void DialogAdaptiveTransparencySettings::enforceDistanceConstraint(bool forceFarUpdate)
{
    const double nearDist = m_ui.lineEdit_transparencyDistNear->getValue();
    const double farDist = m_ui.lineEdit_transparencyDistFar->getValue();
    if (forceFarUpdate || farDist <= nearDist)
        m_ui.lineEdit_transparencyDistFar->setValue(nearDist + 1.0);
}
