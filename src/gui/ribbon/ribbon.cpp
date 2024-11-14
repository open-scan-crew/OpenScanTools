#include "gui/ribbon/ribbon.h"
#include "gui/ribbon/ribbontabcontent.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleOption>
#include <QtWidgets/qlabel.h>


Ribbon::Ribbon(QWidget *parent)
  : QTabWidget(parent)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
	// Note: the order in which the background/palette/stylesheet functions are
	// called does matter. Should be same as in Qt designer.
	setAutoFillBackground(true);
}

void Ribbon::addTab(const QString &tabName, RibbonTabContent *content)
{
	content->finish();
	QTabWidget::addTab(content, tabName);
}


void Ribbon::removeTab(const QString &tabName)
{

  // Find ribbon tab
  for (int i = 0; i < count(); i++)
  {
    if (tabText(i).toLower() == tabName.toLower())
    {
      // Remove tab
      QWidget *tab = QTabWidget::widget(i);
      QTabWidget::removeTab(i);
      delete tab;
      break;
    }
  }

}

void Ribbon::disableTab(const QString& tabName)
{

    // Find ribbon tab
    for (int i = 0; i < count(); i++)
    {
        if (tabText(i).toLower() == tabName.toLower())
        {
            // Remove tab
            QWidget* tab = QTabWidget::widget(i);
            tab->setEnabled(false);
            /*
            tab->setToolTip(TEXT_LICENCE_VERSION_FEATURE_NOT_AVAILABLE);
            for (QWidget* widg : tab->findChildren<QWidget*>())
                widg->setToolTip(TEXT_LICENCE_VERSION_FEATURE_NOT_AVAILABLE);
                */
            break;
        }
    }

}

RibbonTabContent* Ribbon::getTab(const QString& tabName)
{
    // Find ribbon tab
    for (int i = 0; i < count(); i++)
    {
        if (tabText(i).toLower() == tabName.toLower())
        {
            // Just to not crash as we don't care about perf here
            return dynamic_cast<RibbonTabContent*>(QTabWidget::widget(i));
        }
    }
    return nullptr;
}

/*
void Ribbon::resizeEvent(QResizeEvent *event)
{
    
}
*/
