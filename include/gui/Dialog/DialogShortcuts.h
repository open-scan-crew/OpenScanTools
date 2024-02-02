#ifndef DIALOG_SHORTCUTS_H
#define DIALOG_SHORTCUTS_H

#include "ui_DialogShortcuts.h"

class DialogShortcuts : public QDialog
{
    Q_OBJECT

public:
    DialogShortcuts(QWidget* parent = Q_NULLPTR);
    ~DialogShortcuts();

private:
    Ui::DialogShortcuts  m_ui;
};
#endif