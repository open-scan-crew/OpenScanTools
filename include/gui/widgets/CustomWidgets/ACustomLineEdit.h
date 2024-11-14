#ifndef CUSTOM_LINE_EDIT_H_
#define CUSTOM_LINE_EDIT_H_

#include <QtWidgets/qlineedit.h>
#include <qfuturewatcher.h>
#include <qthreadpool.h>

enum class LineEditType {REGEXP, INTEGER, DOUBLE};

class ACustomLineEdit : public QLineEdit
{
	Q_OBJECT

public:
	enum class LineEditRules {Nothing, NotZero, NotNegative, PositiveStrict};

public:
	ACustomLineEdit(QWidget* parent = nullptr);
	ACustomLineEdit(const QString&, QWidget* parent = nullptr);
	~ACustomLineEdit();

	void activatePlaceholderText(const bool& activate);
	void setText(const QString& text);
	void blockInputReject(bool block);

	virtual LineEditType getType() = 0;

public slots:
	void inputRejected();

signals:
	void onPaletteChange(QPalette palette);

protected:
	void initialise();
	void toggleBackgroundColor();
	virtual void initialiseValidator() = 0;

protected:
	bool m_setPlaceholder;
	QFutureWatcher<void> m_invalidToggleWatcher;
	QPalette m_basePalette, m_errorPalette;
	QThreadPool m_pool;

	bool m_blockInputReject;
};
#endif // !CUSTOM_LINE_EDIT_H_
