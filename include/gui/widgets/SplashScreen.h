#ifndef SPLASH_SCREEN_H
#define SPLASH_SCREEN_H

#include <QtWidgets/qsplashscreen.h>
#include <QtGui/qpainter.h>

class SplashScreen : public QSplashScreen
{
public:
    SplashScreen();
    ~SplashScreen();
    virtual void drawContents(QPainter *painter);
    void showStatusMessage(const QString &message, const QColor &color = Qt::white);
    void setMessageRect(QRect rect, int alignment = Qt::AlignLeft);
private:
    QString m_message;
    int		m_alignement;
    QColor	m_color;
    QRect	m_rect;
};

#endif // SPLASH_SCREEN_H
