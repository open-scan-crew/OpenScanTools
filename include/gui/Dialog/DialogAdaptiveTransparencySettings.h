#ifndef DIALOG_ADAPTIVE_TRANSPARENCY_SETTINGS_H
#define DIALOG_ADAPTIVE_TRANSPARENCY_SETTINGS_H

#include "ui_DialogAdaptiveTransparencySettings.h"
#include "gui/UnitUsage.h"

#include <QtWidgets/QDialog>

struct AdaptiveTransparencySettings
{
    float transparencyNear = 5.f;
    float transparencyFar = 40.f;
    float distanceNear = 2.f;
    float distanceFar = 10.f;
};

class DialogAdaptiveTransparencySettings : public QDialog
{
    Q_OBJECT

public:
    DialogAdaptiveTransparencySettings(const AdaptiveTransparencySettings& settings, UnitType distanceUnit, QWidget* parent = nullptr);
    ~DialogAdaptiveTransparencySettings() override = default;

    AdaptiveTransparencySettings getSettings() const;

private slots:
    void onDistanceNearEdited();
    void onDistanceFarEdited();

private:
    void applySettingsToUi(const AdaptiveTransparencySettings& settings);
    void enforceDistanceConstraint();

private:
    Ui::DialogAdaptiveTransparencySettings m_ui;
};

#endif
