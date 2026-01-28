#ifndef DIALOG_COLOR_DENOISE_FILTER_H
#define DIALOG_COLOR_DENOISE_FILTER_H

#include "controller/messages/ColorDenoiseFilterMessage.h"
#include "gui/Dialog/ADialog.h"
#include "ui_DialogColorDenoiseFilter.h"

#include <string>

class DialogColorDenoiseFilter : public ADialog
{
    Q_OBJECT

public:
    DialogColorDenoiseFilter(IDataDispatcher& dataDispatcher, QWidget* parent);
    ~DialogColorDenoiseFilter();

    void informData(IGuiData* data) override;

private:
    void startFiltering();
    void cancelFiltering();
    void translateUI();
    void refreshUI();

    enum class DenoisePreset
    {
        Low,
        Mid,
        High
    };

    void applyPreset(DenoisePreset preset);
    void syncUiFromValues();

private:
    Ui::DialogColorDenoiseFilter m_ui;
    std::wstring m_outputFolder;
    QString m_openPath;

    int m_kNeighbors = 12;
    int m_strength = 40;
    int m_luminanceStrength = 50;
    double m_radiusFactor = 4.0;
    bool m_preserveLuminance = true;
    DenoiseFilterMode m_mode = DenoiseFilterMode::Separate;

    DenoisePreset m_preset = DenoisePreset::Mid;
};

#endif
