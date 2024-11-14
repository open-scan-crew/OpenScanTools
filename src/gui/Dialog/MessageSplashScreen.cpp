#include "gui/Dialog/MessageSplashScreen.h"
#include "gui/texts/SplashScreenTexts.hpp"


MessageSplashScreen::MessageSplashScreen(QWidget *parent)
	: QDialog(parent)
{
	m_ui.setupUi(this);
	setWindowTitle(TEXT_MESSAGE_SPLASH_SCREEN_NAME);

	// ----- Important | Window Flags -----
	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::MSWindowsFixedSizeDialogHint;
	flags ^= Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);
	// ------------------------------------

	setModal(true);
}

MessageSplashScreen::~MessageSplashScreen()
{}

void MessageSplashScreen::setShowMessage(const QString& message)
{
	m_ui.label->setText(message);
	show();
}