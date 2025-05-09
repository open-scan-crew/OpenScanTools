#ifndef MARKER_ICON_SELECTION_DIALOG_H_
#define MARKER_ICON_SELECTION_DIALOG_H_

#include "gui/IPanel.h"
#include "models/data/Marker.h"

#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/qdialog.h>

class MarkerIconSelectionDialog : public QDialog, public IPanel
{
    Q_OBJECT

public:
    MarkerIconSelectionDialog(QWidget *parent, float _guiScale = 1.0f);
    ~MarkerIconSelectionDialog();

    // from IPanel
    void informData(IGuiData *keyValue) override;

    void selectIconButton(scs::MarkerIcon icon);

signals:
    void markerSelected(scs::MarkerIcon icon);

private:

    QGridLayout* m_gridLayout;
    QHBoxLayout* m_hLayout;
    std::unordered_map<scs::MarkerIcon, QToolButton*> m_iconButton;
    QToolButton *m_okButton;
    QToolButton *m_cancelButton;

    scs::MarkerIcon m_iconChecked;
};

#endif