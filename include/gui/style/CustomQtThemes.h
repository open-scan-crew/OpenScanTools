#ifndef CUSTOM_QT_THEMES_H_
#define CUSTOM_QT_THEMES_H_

#include <QProxyStyle>

namespace scs
{

    class ProxyStyle : public QProxyStyle
    {
        Q_OBJECT
    public:
        ProxyStyle(QStyle* style)
            : QProxyStyle(style)
        {}
        int styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const override;
    };


    void ApplyDarkTheme(float scale);
}
#endif