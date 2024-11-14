#ifndef ADIALOG_H_
#define ADIALOG_H_

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

#include <QtWidgets/qdialog.h>

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