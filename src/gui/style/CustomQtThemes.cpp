#include "gui/style/CustomQtThemes.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>
#include <QtWidgets/QStyleOption>

void scs::ApplyDarkTheme(float scale)
{
    QStyle* style = QStyleFactory::create("Fusion");
    ProxyStyle* proxyStyle = new ProxyStyle(style);
    qApp->setStyle(proxyStyle);

    // modify palette to dark
    QPalette palette;

    QColor disabledColor = QColor(127, 127, 127);
    QColor darkColor = QColor(53, 53, 53);
    QColor otherdarkColor = QColor(66, 66, 66);

    palette.setColor(QPalette::Window, darkColor);
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, disabledColor);
    palette.setColor(QPalette::Base, otherdarkColor);
    palette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, darkColor);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
    palette.setColor(QPalette::Dark, QColor(35, 35, 35));
    palette.setColor(QPalette::Shadow, QColor(20, 20, 20));
    palette.setColor(QPalette::Button, darkColor);
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);

    qApp->setPalette(palette);

    // Fine tuning some elements


    // Determine default colors
    QColor bg = qApp->palette().color(QPalette::Window);
    QColor mid = qApp->palette().color(QPalette::Mid);

    int onePx = 1 * scale + 0.5f;
    int twoPx = 2 * scale + 0.5f;
    int tenPx = 10 * scale;
    int iconSize = 20 * scale;

    QString toolButtonStyleSheet = QString(
        "QToolButton:hover {"
        "	border: %1px solid #148CD2;"// bleu
        "}"
        ""
        "QToolButton:checked {"
        "   border: %1px solid #8C6BE4;"// violet
        "}"
        ""
        "QToolButton:disabled {"
        "	background: rgb(44, 44, 44);"
        "}"
        ""
    ).arg(onePx);   

    QString toolBarStyleSheet = QString(
        "QToolBar {"
        "   spacing: 5px;"
        "   border-bottom: %1px solid gray;"
        "   padding: %1px;"
        "   icon-size: %2px %2px;"
        "}"
        ""
    ).arg(twoPx).arg(iconSize);

    // Set stylesheet
    QString tabWidgetStyleSheet = QString(
        "QTabWidget::pane {"
        "  border-top: %10px solid rgb(%4, %5, %6);"
        "  position: relative;"
        "  top: -%10px;"
        "}"
        ""
        "QTabBar::tab {"
        "  position: absolute;"
        "  top: -%13px;"
        "  margin-top: %10px;"
        "  padding-top: %10px;"
        "  padding-bottom: %11px;"
        "  padding-left: %12px;"
        "  padding-right: %12px;"
        "}"
        ""
        "QTabBar::tab::!selected {"
        //"  border-top: %10px solid rgb(%1, %2, %3);"
        //"  border-right: %10px solid rgb(%1, %2, %3);"
        //"  border-left: %10px solid rgb(%1, %2, %3);"
        "  margin-top: %10px;"
        "  margin-right: %10px;"
        "  margin-left: %10px;"
        "  border-bottom: %10px solid rgb(%4, %5, %6);"
        "}"
        ""
        "QTabBar::tab:selected {"
        "  background-color: rgb(%1, %2, %3);"
        "  border-top: %10px solid rgb(%4, %5, %6);"
        "  border-right: %10px solid rgb(%4, %5, %6);"
        "  border-left: %10px solid rgb(%4, %5, %6);"
        "  border-bottom: %10px solid rgb(%1, %2, %3);"
        "}"
        ""
        "QTabBar::tab:hover"
        "{"
        "  background-color: rgb(%7, %8, %9);"
        "}"
        ""
        "QTreeView"
        "{"
        "	color:rgb(255, 255, 255);"
        "	background-color:rgba(0, 0, 0, 0);"
        "	selection-background-color:transparent;"
        "   show-decoration-selected: 1;"
        "}"
        "QTreeView::item:hover, QTreeView::item:hover:selected"
        "{"
        "	border: 1px solid #666666;"
        "	border-radius:1px;"
        "}"
        ""
        "QTreeView::item:selected, QTreeView::item:selected:active, QTreeView::item:selected:!active"
        "{"
        "	border-radius:1px;"
        "	background-color:rgba(255,255,255,50);"
        "   color:rgb(0,0,0);"
        "}"
        "QTreeView::item:hover {"
        "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #e7effd, stop: 1 #cbdaf1);"
        "    color:rgb(50, 50, 50); "
        "    border: 1px solid #bfcde4;"
        "}"
        ""
        "QTreeView::branch:has-siblings:!adjoins-item {"
        "    border-image: url(:/icons/arbo/stylesheet-vline.png) 0;"
        "}"
        ""
        "QTreeView::branch:has-siblings:adjoins-item {"
        "    border-image: url(:/icons/arbo/stylesheet-branch-more.png) 0;"
        "}"
        ""
        "QTreeView::branch:!has-children:!has-siblings:adjoins-item {"
        "    border-image: url(:/icons/arbo/stylesheet-branch-end.png) 0;"
        "}"
        ""
        "QTreeView::branch:has-children:!has-siblings:closed,"
        "QTreeView::branch:closed:has-children:has-siblings {"
        "        border-image: none;"
        "        image: url(:/icons/arbo/stylesheet-branch-closed.png);"
        "}"
        ""
        "QTreeView::branch:open:has-children:!has-siblings,"
        "QTreeView::branch:open:has-children:has-siblings  {"
        "        border-image: none;"
        "        image: url(:/icons/arbo/stylesheet-branch-open.png);"
        "}"
        ""
        "QTreeView::branch:open:has-children:has-siblings  {"
        "        border-image: none;"
        "        image: url(:/icons/arbo/stylesheet-branch-open.png);"
        "}"
        ""
        "QTreeView::indicator:unchecked {image: url(:/icons/arbo/arbo-hide.png);}"
        "QTreeView::indicator:checked {image: url(:/icons/arbo/arbo-show.png);}"
        "QTreeView::indicator:indeterminate {image: url(:/icons/arbo/arbo-half.png);}"
    ).arg(bg.red()).arg(bg.green()).arg(bg.blue())
        .arg(mid.red()).arg(mid.green()).arg(mid.blue())
        .arg(102).arg(102).arg(102)
        .arg(onePx).arg(twoPx).arg(tenPx).arg(onePx - 1);

    QString AllStyleSheet = toolButtonStyleSheet + toolBarStyleSheet + tabWidgetStyleSheet;
    qApp->setStyleSheet(AllStyleSheet);
}

int scs::ProxyStyle::styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const
{
    if (hint == QStyle::SH_ToolTip_WakeUpDelay)
        return 200; //millisecond

    return QProxyStyle::styleHint(hint, option, widget, returnData);
}