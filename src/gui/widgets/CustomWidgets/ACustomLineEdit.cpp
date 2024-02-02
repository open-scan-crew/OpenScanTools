#include "gui/widgets/CustomWidgets/ACustomLineEdit.h"
#include <QtConcurrent/qtconcurrentrun.h>
#include "gui/Texts.hpp"

ACustomLineEdit::ACustomLineEdit(QWidget* parent)
	: QLineEdit(parent)
	, m_pool(this)
	, m_setPlaceholder(true)
	, m_blockInputReject(false)
{
	initialise();
	setPlaceholderText(TEXT_ENTER_VALUE_PLACEHOLDER);
}

ACustomLineEdit::ACustomLineEdit(const QString& string, QWidget* parent)
	: QLineEdit(string, parent)
	, m_pool(this)
	, m_setPlaceholder(true)
	, m_blockInputReject(false)
{
	initialise();
}

ACustomLineEdit::~ACustomLineEdit()
{}

void ACustomLineEdit::setText(const QString& text)
{
	if (m_setPlaceholder)
		setPlaceholderText(text);
	QLineEdit::setText(text);
}

void ACustomLineEdit::blockInputReject(bool block)
{
	m_blockInputReject = block;
}

void ACustomLineEdit::activatePlaceholderText(const bool& activate)
{
	m_setPlaceholder = activate;
}

void ACustomLineEdit::initialise()
{
	m_basePalette = this->palette();
	m_errorPalette.setColor(QPalette::Base, Qt::red);
	m_errorPalette.setColor(QPalette::Window, Qt::red);

	QObject::connect(this, &QLineEdit::inputRejected, this, &ACustomLineEdit::inputRejected);
	QObject::connect(this, &ACustomLineEdit::onPaletteChange, this, &QLineEdit::setPalette);
}

void ACustomLineEdit::inputRejected()
{
	if(!m_blockInputReject)
		m_invalidToggleWatcher.setFuture(QtConcurrent::run(this, &ACustomLineEdit::toggleBackgroundColor));
}

void ACustomLineEdit::toggleBackgroundColor()
{
	emit onPaletteChange(m_errorPalette);
	QThread::msleep(500);
	emit onPaletteChange(m_basePalette);
}