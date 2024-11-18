#include "gui/ribbon/ribbontabcontent.h"
#include "ui_ribbontabcontent.h"
#include "QtWidgets/qscrollbar.h"
#include <QtWidgets/qtoolbutton.h>
#include <QtGui/qevent.h>


RibbonTabContent::RibbonTabContent(QWidget *parent)
  : QWidget(parent)
  ,  ui(new Ui::RibbonTabContent)
{
	ui->setupUi(this);
	
	m_content = new QWidget();
	m_layout = new QHBoxLayout(m_content);
	m_layout->setSpacing(0);
	m_layout->setContentsMargins(0, 0, 0, 0);

	ui->leftButton->setAutoRepeat(true);
	ui->rightButton->setAutoRepeat(true);

	QObject::connect(ui->leftButton, &QToolButton::released, this, &RibbonTabContent::slotScrollLeft);
	QObject::connect(ui->rightButton, &QToolButton::released, this, &RibbonTabContent::slotScrollRight);
}

void RibbonTabContent::updateScrollButtons()
{
	// Show toolbar icons only if there is not enough room to fit the toolbar 
	if (width() < ui->scrollArea->widget()->width())
	{
		ui->leftButton->setVisible(true);
		ui->rightButton->setVisible(true);
	}
	else
	{
		ui->leftButton->setVisible(false);
		ui->rightButton->setVisible(false);
	}
}

void RibbonTabContent::resizeEvent(QResizeEvent *event)
{
	updateScrollButtons();
	event->accept();
}

bool RibbonTabContent::event(QEvent *event)
{
	if (event->type() == QEvent::Enter)
	{
		updateScrollButtons();
	}
	return (QWidget::event(event));
}

void RibbonTabContent::slotScrollLeft()
{
	ui->scrollArea->horizontalScrollBar()->setValue(ui->scrollArea->horizontalScrollBar()->value() - 50);
}

void RibbonTabContent::slotScrollRight()
{
	ui->scrollArea->horizontalScrollBar()->setValue(ui->scrollArea->horizontalScrollBar()->value() + 50);
}


void RibbonTabContent::finish()
{
	m_content->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
	ui->scrollArea->setWidget(m_content);
	ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->scrollArea->setWidgetResizable(true);
	updateScrollButtons();
}

RibbonTabContent::~RibbonTabContent()
{
  delete ui;
}

void RibbonTabContent::addGroup(const QString &groupName)
{
  RibbonButtonGroup *ribbonButtonGroup = new RibbonButtonGroup;
  ribbonButtonGroup->setTitle(groupName);

  m_layout->addWidget(ribbonButtonGroup);

}

void RibbonTabContent::removeGroup(const QString &groupName)
{
  // Find ribbon group
  for (int i = 0; i < m_layout->count(); i++)
  {
    RibbonButtonGroup *group = static_cast<RibbonButtonGroup*>(m_layout->itemAt(i)->widget());
    if (group->title().toLower() == groupName.toLower())
    {
      m_layout->removeWidget(group); /// \todo :( No effect
      delete group;
      break;
    }
  }

  /// \todo  What if the group still contains buttons? Delete manually?
  // Or automaticly deleted by Qt parent() system.
}

void RibbonTabContent::disableGroup(const QString& groupName)
{
	for (int i = 0; i < m_layout->count(); i++)
	{
		RibbonButtonGroup* group = static_cast<RibbonButtonGroup*>(m_layout->itemAt(i)->widget());
		if (group->title().toLower() == groupName.toLower())
		{
			group->setEnabled(false);
			/*
			group->setToolTip(TEXT_LICENCE_VERSION_FEATURE_NOT_AVAILABLE);
			for (QWidget* widg : group->findChildren<QWidget*>())
				widg->setToolTip(TEXT_LICENCE_VERSION_FEATURE_NOT_AVAILABLE);
			*/
			break;
		}
	}
}

int RibbonTabContent::groupCount() const
{
  return m_layout->count();
}

RibbonButtonGroup* RibbonTabContent::getGroup(const QString &groupName)
{
	// Find ribbon group
	RibbonButtonGroup *ribbonButtonGroup = nullptr;
	for (int i = 0; i < m_layout->count(); i++)
	{
		RibbonButtonGroup *group = static_cast<RibbonButtonGroup*>(m_layout->itemAt(i)->widget());
		if (group->title().toLower() == groupName.toLower())
		{
			ribbonButtonGroup = group;
			break;
		}
	}

	return ribbonButtonGroup;
}

void RibbonTabContent::addWidget(const QString &groupName, QWidget *w)
{
	RibbonButtonGroup *ribbonButtonGroup = getGroup(groupName);

	if (ribbonButtonGroup != nullptr)
	{
		// Group found
		// Add ribbon button
		ribbonButtonGroup->addWidget(w);
	
	}
	else
	{
		// Group not found
		// Add ribbon group
		addGroup(groupName);
		// Add ribbon button
		addWidget(groupName, w);
	}
}
