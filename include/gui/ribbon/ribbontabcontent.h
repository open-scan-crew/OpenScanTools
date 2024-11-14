#ifndef RIBBONTABCONTENT_H
#define RIBBONTABCONTENT_H

#include <QtWidgets/qwidget.h>
#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/QScrollArea>
#include "gui/ribbon/ribbonbuttongroup.h"
#include <QtWidgets/qboxlayout.h>

namespace Ui {
class RibbonTabContent;
}

class RibbonTabContent : public QWidget
{
  Q_OBJECT

public:
  explicit RibbonTabContent(QWidget *parent = 0);
  virtual ~RibbonTabContent();

  /// Add a group to the tab content.
  ///
  /// \param[in] groupName Name of the group
  void addGroup(const QString &groupName);

  /// Remove a group from the tab content.
  ///
  /// \param[in] groupName Name of the group
  void removeGroup(const QString &groupName);

  void disableGroup(const QString& groupName);

  /// Get the number of button groups in this tab content.
  ///
  /// \return The number of button groups
  int groupCount() const;

  void finish();

  void addWidget(const QString &groupName, QWidget *w);

private slots:
  void slotScrollLeft();
  void slotScrollRight();

protected:
	void resizeEvent(QResizeEvent *);
	bool event(QEvent *event);

private:
	Ui::RibbonTabContent *ui;

	QHBoxLayout *m_layout;
	QWidget *m_content;

	RibbonButtonGroup* getGroup(const QString &groupName);
	void updateScrollButtons();
};

#endif // RIBBONTABCONTENT_H
