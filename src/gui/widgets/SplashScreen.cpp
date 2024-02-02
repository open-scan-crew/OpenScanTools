#include "gui/widgets/SplashScreen.h"
#include "gui/texts/AboutTexts.hpp"
#include <QtGui/QPixMap>

SplashScreen::SplashScreen() 
	: QSplashScreen(QPixmap(":/resources/images/Ouverture_OpenScanTools_800-399.jpg"), Qt::Tool | Qt::SplashScreen)
{

	//QPixmap splashMask(":images/splashmask.png");

	setMessageRect(QRect::QRect(15, 10, 400, 20), Qt::AlignLeft); // Setting the message position.

	QFont splashFont;
	splashFont.setFamily("Arial");
	splashFont.setBold(true);
	splashFont.setPixelSize(16);

	setFont(splashFont);
//	splash->setMask(splashMask);
	//setWindowFlags(Qt::WindowStaysOnTopHint );
	
	showStatusMessage("");
	
};

SplashScreen::~SplashScreen()
{
};

void SplashScreen::drawContents(QPainter *painter)
{
	//QPixmap textPix = QSplashScreen::pixmap();
    painter->setPen(m_color);
    painter->drawText(m_rect, m_alignement, TEXT_ABOUT_VERSION + " - " + m_message);
};

void SplashScreen::showStatusMessage(const QString &message, const QColor &color)
{
    m_message = message;
    m_color = color;
    showMessage(m_message, m_alignement, m_color);
};

void SplashScreen::setMessageRect(QRect rect, int alignement)
{
    m_rect = rect;
    m_alignement = alignement;
};
