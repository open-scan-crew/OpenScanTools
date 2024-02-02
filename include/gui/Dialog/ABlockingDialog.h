#ifndef ABLOCKINGDIALOG_H_
#define ABLOCKINGDIALOG_H_

#include "ADialog.h"

class ABlockingDialog : public ADialog
{
public:
	ABlockingDialog(IDataDispatcher& dataDispacher, QWidget* parent = 0);
	~ABlockingDialog();

	// from IPanel
	virtual void informData(IGuiData* keyValue) = 0;
	virtual QString getName() const = 0;
	void keyPressEvent(QKeyEvent* e);
	void closeEvent(QCloseEvent* e);
};

#endif // !ADIALOG_H_