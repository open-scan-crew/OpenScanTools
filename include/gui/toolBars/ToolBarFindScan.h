#ifndef TOOLBAR_FIND_SCAN_H
#define TOOLBAR_FIND_SCAN_H

#include <QtWidgets/qwidget.h>
#include "ui_toolbar_findscan.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

class ToolBarFindScan : public QWidget, public IPanel
{
    Q_OBJECT

public:
    explicit ToolBarFindScan(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale);

    void informData(IGuiData *keyValue) override;

private:
    void onProjectLoad(IGuiData *data);
    ~ToolBarFindScan();

public slots:
    void slotFindScan();

private:
    IDataDispatcher &m_dataDispatcher;

    typedef void (ToolBarFindScan::* GuiDataFct)(IGuiData*);
    std::unordered_map<guiDType, GuiDataFct> m_guiDataFunctions;

    Ui::toolbar_findScan m_ui;
};

#endif
