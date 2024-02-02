#ifndef ADIALOG_H_
#define ADIALOG_H_

#include <QtWidgets/QDialog>
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

class ADialog : public QDialog, public IPanel
{
	Q_OBJECT

public:
	ADialog(IDataDispatcher& dataDispacher, QWidget* parent = 0);
	~ADialog();

	// from IPanel
	virtual void informData(IGuiData* keyValue) = 0;

protected:
	IDataDispatcher& m_dataDispatcher;
};

#endif // !ADIALOG_H_