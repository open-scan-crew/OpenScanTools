#ifndef DIALOG_COLOR_BALANCE_H
#define DIALOG_COLOR_BALANCE_H

#include "gui/Dialog/ADialog.h"
#include "ui_DialogColorBalance.h"

#include "controller/messages/ColorBalanceMessage.h"

class DialogColorBalance : public ADialog
{
    Q_OBJECT
public:
    DialogColorBalance(IDataDispatcher& dataDispatcher, QWidget* parent);
    ~DialogColorBalance();

    void informData(IGuiData* data) override;

private slots:
    void startColorBalance();
    void cancelColorBalance();

private:
    enum class ColorBalancePreset
    {
        Low,
        Mid,
        High
    };

    void translateUI();
    void refreshUI();
    void applyPreset(ColorBalancePreset preset);
    void syncUiFromValues();

    Ui::DialogColorBalance m_ui;
    ColorBalancePreset m_preset = ColorBalancePreset::Mid;
    int m_photometricDepth = 7;
    int m_nMin = 150;
    int m_nGood = 1000;
    double m_beta = 0.5;
    int m_smoothingLevels = 4;
    bool m_applyIntensityWhenAvailable = true;
    std::wstring m_outputFolder;
    QString m_openPath;
};

#endif
