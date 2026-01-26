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
    void startFiltering();
    void cancelFiltering();

private:
    void translateUI();
    void refreshUI();
    void syncUiFromValues();

    Ui::DialogColorBalance m_ui;
    ColorBalanceMode m_mode = ColorBalanceMode::Separate;
    ColorBalanceStrength m_strength = ColorBalanceStrength::Mid;
    bool m_applyOnRGBAndIntensity = false;
    bool m_hasBothComponents = false;
    std::wstring m_outputFolder;
    QString m_openPath;
};

#endif
