#include "gui/widgets/QPFields/QPHyperLinkField.h"

#include <QtCore/qurl.h>

QPHyperLinkField::QPHyperLinkField(const sma::tField& field, QWidget* parent)
	: AQPField(field, parent)
{
	buildQPField(field);
}

QPHyperLinkField::~QPHyperLinkField()
{}

void QPHyperLinkField::buildQPField(const sma::tField& field)
{
	m_field = field;
	Firstlayout = new QHBoxLayout();
	VLayout = new QVBoxLayout();
	hLinkInfield = new QLineEdit();
	hLinkInfield->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	hLinkLabel = new QLabel();
	hLinkLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
	nameLabel = new QLabel(QString::fromStdWString(field.m_name));
	nameLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	Firstlayout->addWidget(nameLabel);
	Firstlayout->addWidget(hLinkInfield);
	VLayout->addLayout(Firstlayout);
	VLayout->addWidget(hLinkLabel);

	hLinkLabel->setTextFormat(Qt::RichText);
	hLinkLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	hLinkLabel->setOpenExternalLinks(true);

	setLayout(VLayout);

	QObject::connect(hLinkInfield, &QLineEdit::editingFinished, this, &QPHyperLinkField::onInfieldEdit);
}

QLineEdit *QPHyperLinkField::getInfield() const
{
	return (hLinkInfield);
}

QLabel *QPHyperLinkField::getHlinkLabel() const
{
	return (hLinkLabel);
}

void QPHyperLinkField::onInfieldEdit()
{
	QString str = QUrl::fromUserInput(hLinkInfield->text()).toString();
	hLinkLabel->setText("<a href=\"" + str + "\">" + str + "</a>");
	emit edited(m_field.m_id, str.toStdWString());
}

void QPHyperLinkField::setValue(QString value)
{
	hLinkInfield->blockSignals(true);
	hLinkLabel->blockSignals(true);
	hLinkInfield->setText(value);
	QString str = QUrl::fromUserInput(hLinkInfield->text()).toString();
	hLinkLabel->setText("<a href=\"" + str + "\">" + str + "</a>");
	hLinkInfield->blockSignals(false);
	hLinkLabel->blockSignals(false);
}