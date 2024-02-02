#ifndef TOOLBAR_POINT_EDITION_H
#define TOOLBAR_POINT_EDITION_H

#include <QtWidgets/QWidget>
#include "ui_toolbar_pointEdition.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

class ToolBarPointEdition : public QWidget, public IPanel
{
    Q_OBJECT

public:
    explicit ToolBarPointEdition(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale);

    void informData(IGuiData *keyValue) override;

private:
    void onProjectLoad(IGuiData *data);
    ~ToolBarPointEdition();

public slots:
    void slotDeleteClippedPoints();

private:
    IDataDispatcher &m_dataDispatcher;

    typedef void (ToolBarPointEdition::* GuiDataFct)(IGuiData*);
    std::unordered_map<guiDType, GuiDataFct> m_guiDataFunctions;

    Ui::toolbar_pointEdition m_ui;
};

#endif
