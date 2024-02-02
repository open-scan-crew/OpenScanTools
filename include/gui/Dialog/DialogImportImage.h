#ifndef DIALOG_IMPORT_IMAGE_H
#define DIALOG_IMPORT_IMAGE_H

#include "gui/Dialog/ADialog.h"
#include "ui_DialogImportImage.h"

class DialogImportImage : public ADialog
{
    Q_OBJECT

public:
	DialogImportImage(IDataDispatcher& dataDispatcher, QWidget *parent);
    ~DialogImportImage();

    void informData(IGuiData *data) override;

private:
    void onCancel();
	void onConvert();

	void onInputBrowser();
	void onOutputBrowser();


private:
    Ui::DialogImportImage m_ui;
	QString							   m_openPath;

};

#endif
