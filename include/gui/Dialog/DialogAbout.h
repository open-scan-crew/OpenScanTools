#ifndef DIALOG_ABOUT_H
#define DIALOG_ABOUT_H

#include "gui/Dialog/ADialog.h"
#include "gui/IDataDispatcher.h"
#include "ui_DialogAbout.h"

class DialogAbout : public ADialog
{
    Q_OBJECT

public:
    DialogAbout(IDataDispatcher& dataDispatcher, QWidget* parent = Q_NULLPTR);
    ~DialogAbout();

    void informData(IGuiData* keyValue) override;

    void setDialog(float guiScale);

private:
    Ui::DialogAbout      m_ui;
    QString              m_link;
};
#endif