#ifndef _MessageSplashScreen_H_
#define _MessageSplashScreen_H_

#include "ui_Message_Splashscreen.h"
#include <QtWidgets/qdialog.h>

class MessageSplashScreen : public QDialog
{

	Q_OBJECT

public:
	MessageSplashScreen(QWidget *parent = nullptr);
	~MessageSplashScreen();

	void setShowMessage(const QString& message);

private:
	Ui::messageSplashScreen m_ui;
};

#endif //_MessageSplashScreen_H_
