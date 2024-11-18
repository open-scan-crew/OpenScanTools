#ifndef RIBBONBUTTONGROUP_H
#define RIBBONBUTTONGROUP_H

#include <QtWidgets/qwidget.h>

namespace Ui {
class RibbonButtonGroup;
}

class RibbonButtonGroup : public QWidget
{
  Q_OBJECT

public:
  explicit RibbonButtonGroup(QWidget *parent = 0);
  virtual ~RibbonButtonGroup();

  /// Set the title of the button group.
  /// The title is shown underneath the buttons.
  ///
  /// \param[in] title The title
  void setTitle(const QString &title);

  /// Get the title of the button group.
  ///
  /// \return The title
  QString title() const;

  /// Get the number of buttons in the button group.
  ///
  /// \return The number of buttons
  int buttonCount() const;

  void addWidget(QWidget * w);

private:
  Ui::RibbonButtonGroup *ui;
  QString m_title; ///< Title of the button group
};

#endif // RIBBONBUTTONGROUP_H
