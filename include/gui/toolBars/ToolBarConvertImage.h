#ifndef TOOLBAR_CONVERTIMAGE_H
#define TOOLBAR_CONVERTIMAGE_H

#include <QtWidgets/QWidget>
#include "ui_toolbar_convertImage.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

#include "gui/Dialog/DialogImportImage.h"

class ToolBarConvertImage : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarConvertImage(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale);

	void informData(IGuiData *data) override;

private:
    ~ToolBarConvertImage();
	void onProjectLoad(IGuiData* data);

public slots:
	void slotConvertImage();

private:
	Ui::ToolBarConvertImage m_ui;
    IDataDispatcher &m_dataDispatcher;
	DialogImportImage* m_dialog;
};

#endif // TOOLBAR_IMPORTOBJECTS_H

