#ifndef DIALOG_SET_OF_POINTS_H
#define DIALOG_SET_OF_POINTS_H

#include "ui_DialogSetOfPoints.h"
#include "gui/Dialog/ADialog.h"

class DialogSetOfPoints : public ADialog
{
    Q_OBJECT

 public:
     DialogSetOfPoints(IDataDispatcher& dataDispatcher, QWidget* parent);
    ~DialogSetOfPoints();

public:
    void onCancel();
    void onOk();

    virtual void informData(IGuiData* keyValue) override;

private:
    Ui::DialogSetOfPoints m_ui;

};

#endif //!DIALOG_OPEN_PROJECT_CENTRAL_H