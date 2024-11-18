#include "gui/ribbon/ribbonbuttongroup.h"
#include "ui_ribbonbuttongroup.h"

RibbonButtonGroup::RibbonButtonGroup(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::RibbonButtonGroup)
  , m_title("")
{
  ui->setupUi(this);
}

RibbonButtonGroup::~RibbonButtonGroup()
{
  delete ui;
}

void RibbonButtonGroup::setTitle(const QString &title)
{
  m_title = title;
  ui->label->setText(m_title);
}

QString RibbonButtonGroup::title() const
{
  return m_title;
}

int RibbonButtonGroup::buttonCount() const
{
  return ui->horizontalLayout->count();
}

void RibbonButtonGroup::addWidget(QWidget *w)
{
	ui->horizontalLayout->addWidget(w);
}

