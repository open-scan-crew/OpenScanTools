#include "gui/Dialog/MessageSplashScreen.h"
#include "gui/texts/SplashScreenTexts.hpp"


MessageSplashScreen::MessageSplashScreen(QWidget *parent)
	: QDialog(parent)
{
	m_ui.setupUi(this);
	setWindowTitle(TEXT_MESSAGE_SPLASH_SCREEN_NAME);
	m_ui.buttonBox->setVisible(false);
	connect(m_ui.buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(m_ui.buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

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
	m_ui.buttonBox->setVisible(false);
	m_ui.label->setText(message);
	show();
}

bool MessageSplashScreen::confirmMessage(const QString& message)
{
	m_ui.label->setText(message);
	m_ui.buttonBox->setVisible(true);
	const int result = exec();
	m_ui.buttonBox->setVisible(false);
	return result == QDialog::Accepted;
}
