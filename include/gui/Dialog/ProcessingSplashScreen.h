#ifndef _PROCESSINGSPLASHSCREEN_H_
#define _PROCESSINGSPLASHSCREEN_H_

#include "ui_processing_splashscreen.h"

#include "gui/IDataDispatcher.h"
#include "gui/IPanel.h"

#include <QtCore/qfuturewatcher.h>
#include <QtWidgets/qdialog.h>

class ProcessingSplashScreen : public QDialog, public IPanel
{

	Q_OBJECT

public:
	ProcessingSplashScreen(IDataDispatcher& dataDispatcher, QWidget *parent = nullptr);
	~ProcessingSplashScreen();
	void informData(IGuiData *keyValue);
	void closeEvent(QCloseEvent *event);

signals:
	void onProgressBarValueChange(int step);
	void onProgressBarTitleChange(QString format);
	void onLabelValueChange(QString format);
	void onProgressBarRangeChange(int min, int max);
	QString getProgressBarValue();

	void onLogBrowserClear();
	void onLogBrowserAppend(QString log);
	void onOkButtonShow();
	void onOkButtonHide();

private:
	void updateProgressbarTitle();
	void onCancel();

private:
	Ui::processingSplashScreen *ui;
	IDataDispatcher &m_dataDispatcher;
	bool m_forceStop;
	QFutureWatcher<void> m_progressbarWatcher;
};

#endif //_PROCESSINGSPLASHSCREEN_H_
