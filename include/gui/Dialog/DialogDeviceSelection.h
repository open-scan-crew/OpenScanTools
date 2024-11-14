#ifndef DIALOG_DEVICE_SELECTION_H
#define DIALOG_DEVICE_SELECTION_H

#include "ui_DialogDeviceSelection.h"
#include <QtWidgets/qdialog.h>

class DialogDeviceSelection : public QDialog
{
    Q_OBJECT

public:
    DialogDeviceSelection(const std::unordered_map<uint32_t, std::string>& devices, QWidget *parent);
    ~DialogDeviceSelection();

    int exec() override;
    
    void onOk();

private:
    Ui::DialogDeviceSelection m_ui;
};

#endif