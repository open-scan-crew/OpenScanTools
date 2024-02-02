#ifndef RIBBONTABWIDGET_H
#define RIBBONTABWIDGET_H

#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QScrollArea>
#include "gui/ribbon/ribbontabcontent.h"

class Ribbon : public QTabWidget
{
  Q_OBJECT
public:
  explicit Ribbon(QWidget *parent = 0);


  void addTab(const QString &tabName, RibbonTabContent *content);

  /// Remove a tab from the ribbon.
  ///
  /// \param[in] tabName Name of the tab
  void removeTab(const QString& tabName);

  void disableTab(const QString& tabName);

  RibbonTabContent* getTab(const QString& tabName);

private:

    //void resizeEvent(QResizeEvent *event) override;
};

#endif // RIBBONTABWIDGET_H
