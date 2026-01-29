#ifndef DIALOG_COLOR_BALANCE_H
#define DIALOG_COLOR_BALANCE_H

#include "gui/Dialog/ADialog.h"
#include "ui_DialogColorBalance.h"
#include "pointCloudEngine/ColorBalanceSettings.h"
#include "io/FileUtils.h"

#include <unordered_map>
#include <QtCore/QString>

class DialogColorBalance : public ADialog
{
    Q_OBJECT
public:
    DialogColorBalance(IDataDispatcher& dataDispatcher, QWidget* parent);
    ~DialogColorBalance();

    void informData(IGuiData* data) override;

private:
    enum class StrengthPreset
    {
        Light,
        Medium,
        Strong
    };

    void translateUI();
    void refreshUI();
    void syncUiFromValues();
    void applyPreset(StrengthPreset preset);
    void startColorBalance();
    void cancelColorBalance();

    Ui::DialogColorBalance m_ui;
    std::wstring m_outputFolder;
    QString m_openPath;
    FileType m_outputFileType = FileType::TLS;
    ColorBalanceSettings m_settings;
    StrengthPreset m_preset = StrengthPreset::Medium;
    bool m_enableApplyRgbIntensity = false;
};

#endif
