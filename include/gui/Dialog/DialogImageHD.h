#ifndef DIALOG_IMAGE_HD_H
#define DIALOG_IMAGE_HD_H

#include "ui_DialogImageHD.h"
#include "gui/Dialog/ADialog.h"

class UIClipping;

class DialogImageHD : public ADialog
{
    Q_OBJECT

public:
    DialogImageHD(IDataDispatcher& dataDispatcher, QWidget* parent);
    ~DialogImageHD();

    void informData(IGuiData* data) override;

public:
    void generateImage();
    void cancel();

private:
    void translateUI();


private:
    Ui::DialogImageHD m_ui;

};

#endif
