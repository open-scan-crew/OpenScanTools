#ifndef DIALOG_COLOR_BALANCE_FILTER_H
#define DIALOG_COLOR_BALANCE_FILTER_H

#include "gui/Dialog/ADialog.h"
#include "ui_DialogColorBalanceFilter.h"
#include "controller/messages/ColorBalanceFilterMessage.h"
#include "io/FileUtils.h"

#include <QtCore/QString>

class DialogColorBalanceFilter : public ADialog
{
    Q_OBJECT

public:
    DialogColorBalanceFilter(IDataDispatcher& dataDispatcher, QWidget* parent);
    ~DialogColorBalanceFilter();

    void informData(IGuiData* data) override;

    enum class BalancePreset
    {
        Light,
        Medium,
        Strong
    };

private:
    void startBalancing();
    void cancelBalancing();
    void translateUI();
    void refreshUI();
    void applyPreset(BalancePreset preset);
    void syncUiFromValues();
    void updateAvailability(bool rgbAvailable, bool intensityAvailable, bool rgbAndIntensityAvailable);

    Ui::DialogColorBalanceFilter m_ui;

    BalancePreset m_preset = BalancePreset::Medium;
    ColorBalanceMode m_mode = ColorBalanceMode::Separate;

    int m_kMin = 24;
    int m_kMax = 40;
    double m_trimPercent = 18.0;
    int m_sharpnessPercent = 20;
    bool m_applyOnIntensityAndRgb = true;
    bool m_rgbAvailable = false;
    bool m_intensityAvailable = false;
    bool m_rgbAndIntensityAvailable = false;
    FileType m_outputFileType = FileType::TLS;

    QString m_openPath;
    std::wstring m_outputFolder;
};

#endif
