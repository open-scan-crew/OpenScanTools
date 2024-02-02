#ifndef QPLIST_FIELD_H_
#define QPLIST_FIELD_H_
#include <QtWidgets/QComboBox>

#include "gui/widgets/QPFields/AQPField.h"
#include "models/application/List.h"


class QPListField : public AQPField
{
public:
	QPListField(const sma::tField& field, QWidget* parent);
	~QPListField();

	void setValue(QString value) override;

	void setList(UserList list);
	QComboBox *getInfield() const;
private slots:

	void onComboEdit();

private:

	void buildQPField(const sma::tField& field) override;

private:
	QComboBox *m_listCombo;
	QLabel *m_nameLabel;
	QHBoxLayout *m_layout;
	UserList m_list;
};

#endif // !QPSTRING_FIELD_H_
