#ifndef QDOUBLE_EDIT_H_
#define QDOUBLE_EDIT_H_

#include "gui/IPanel.h"
#include "ANumericLineEdit.h"
#include <QDoubleValidator>
#include "gui/UnitUsage.h"
#include "gui/IDataDispatcher.h"

enum class UnitType;

enum class NumericType { NONE, DISTANCE, DIAMETER, VOLUME};

class QDoubleEdit : public ANumericLineEdit, public IPanel
{
	public:
		QDoubleEdit(QWidget* parent = nullptr);
		QDoubleEdit(const QString&, QWidget* parent = nullptr);
		~QDoubleEdit();

		void registerDataDispatcher(IDataDispatcher* dataDispatcher);
		void informData(IGuiData* keyValue) override;

		void setUnit(UnitType unit);
		void setType(NumericType type);
		void setText(const QString& text);
		void setPower(int power);
		void resetUnit();

		const double& getValue();
        void setValue(double value);

		LineEditType getType() override;

	private:
		void initialiseValidator();
		bool checkValue(const double& value);

		void actualize(bool useCurrentText);

		void focusInEvent(QFocusEvent* event) override;
		void focusOutEvent(QFocusEvent* event) override;

	private:
		QDoubleValidator* m_validator;
		UnitType m_unit;
		NumericType m_type;

		UnitUsage m_valueParameters;
		IDataDispatcher* m_dataDispatcher;

		double m_meterValue;
		int m_power = 1;
};

#endif // !QDOUBLE_EDIT_H_