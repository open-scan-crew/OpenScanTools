#ifndef QPHLINK_FIELD_H_
#define QPHLINK_FIELD_H_

#include <QtWidgets/qlineedit.h>

#include "gui/widgets/QPFields/AQPField.h"


class QPHyperLinkField : public AQPField
{
public:
	QPHyperLinkField(const sma::tField& field, QWidget* parent);
	~QPHyperLinkField();

	void setValue(QString value) override;

	QLineEdit *getInfield() const;
	QLabel *getHlinkLabel() const;

private slots:
	void onInfieldEdit();

private:
	void buildQPField(const sma::tField& field) override;


private:
	QLineEdit *hLinkInfield;
	QLabel *hLinkLabel;
	QLabel *nameLabel;
	QVBoxLayout *VLayout;
	QHBoxLayout *Firstlayout;
};

#endif // !QPSTRING_FIELD_H_
