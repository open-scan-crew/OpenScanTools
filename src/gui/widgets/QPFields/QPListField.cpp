#include "gui/widgets/QPFields/QPListField.h"
#include "gui/IPanel.h"
#include "utils/Logger.h"

QPListField::QPListField(const sma::tField& field, QWidget* parent)
	: AQPField(field, parent)
{
	buildQPField(field);
}

QPListField::~QPListField()
{}


void QPListField::buildQPField(const sma::tField& field)
{
	m_field = field;
	m_layout = new QHBoxLayout();
	m_listCombo = new QComboBox();
	m_listCombo->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	m_nameLabel = new QLabel(QString::fromStdWString(field.m_name));
	m_nameLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	m_layout->addWidget(m_nameLabel);
	m_layout->addWidget(m_listCombo);
	setLayout(m_layout);

	QObject::connect(m_listCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QPListField::onComboEdit);
}

void QPListField::setValue(QString value)
{
	int i = 0;

	GUI_LOG << "list received value : " << value.toStdString() << LOGENDL;
	m_listCombo->blockSignals(true);
	m_listCombo->setCurrentIndex(0);
	for (auto it = m_list.list().begin(); it != m_list.list().end(); it++)
	{
		if (value.toStdWString() == (*it))
		{
			GUI_LOG << "list change to " << i << " : " << m_listCombo->itemText(i).toStdString() << LOGENDL;
			m_listCombo->setCurrentIndex(i);
			m_listCombo->blockSignals(false);
			return;
		}
		++i;
	}
	m_listCombo->blockSignals(false);
}

void QPListField::setList(UserList list)
{
	m_listCombo->blockSignals(true);
	m_list = list;
	for (auto it = m_list.list().begin(); it != m_list.list().end(); it++)
		m_listCombo->addItem(QString::fromStdWString(*it));
	m_listCombo->blockSignals(false);
}

QComboBox * QPListField::getInfield() const
{
	return (m_listCombo);
}

void QPListField::onComboEdit()
{
	emit edited(m_field.m_id, m_listCombo->currentText().toStdWString());
}