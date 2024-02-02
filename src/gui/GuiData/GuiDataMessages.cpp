#include "gui/GuiData/GuiDataMessages.h"

// **** GuiDataWarning ****

GuiDataWarning::GuiDataWarning(const QString& message)
	: m_message(message)
{}

guiDType GuiDataWarning::getType()
{
	return (guiDType::popupWrnData);
}

// **** GuiDataModal ****

GuiDataModal::GuiDataModal(const uint32_t& flags, const QString& message)
	: m_message(message)
	, m_flags(flags)
{}

guiDType GuiDataModal::getType()
{
	return (guiDType::popupModalData);
}

// **** GuiDataInfo ****

GuiDataInfo::GuiDataInfo(const QString& message, bool isModal)
	: m_message(message)
	, m_isModal(isModal)
{}

guiDType GuiDataInfo::getType()
{
	return (guiDType::popupMsgData);
}

// **** GuiDataTmpMessage ****

GuiDataTmpMessage::GuiDataTmpMessage(const QString& message, const int& timeout)
	: m_message(message)
	, m_timeout(timeout)
{}

guiDType GuiDataTmpMessage::getType()
{
	return (guiDType::tmpMsgData);
}

// **** GuiDataProcessingSplashScreenStart ****

GuiDataProcessingSplashScreenStart::GuiDataProcessingSplashScreenStart(const uint64_t& maxStep, const QString& label, const QString& state)
	: m_label(label)
	, m_maxStep(maxStep)
	, m_state(state)
{}

guiDType GuiDataProcessingSplashScreenStart::getType()
{
	return (guiDType::processingSplashScreenStart);
}

// **** GuiDataProcessingSplashScreenProgressBarUpdate ****

GuiDataProcessingSplashScreenProgressBarUpdate::GuiDataProcessingSplashScreenProgressBarUpdate(const QString& state, const uint64_t& step)
	: m_step(step)
	, m_state(state)
{}

guiDType GuiDataProcessingSplashScreenProgressBarUpdate::getType()
{
	return (guiDType::processingSplashScreenProgressBarUpdate);
}

// **** GuiDataProcessingSplashScreenEnableCancelButton ****

GuiDataProcessingSplashScreenEnableCancelButton::GuiDataProcessingSplashScreenEnableCancelButton(bool enableCancel)
	: m_enableCancel(enableCancel)
{}

guiDType GuiDataProcessingSplashScreenEnableCancelButton::getType()
{
	return guiDType::processingSplashScreenEnableCancel;
}


// **** GuiDataProcessingSplashScreenLogUpdate ****

GuiDataProcessingSplashScreenLogUpdate::GuiDataProcessingSplashScreenLogUpdate(const QString& log)
	: m_log(log)
{}

guiDType GuiDataProcessingSplashScreenLogUpdate::getType()
{
	return (guiDType::processingSplashScreenLogUpdate);
}

// **** GuiDataProcessingSplashScreenEnd ****

GuiDataProcessingSplashScreenEnd::GuiDataProcessingSplashScreenEnd(const QString& state)
	: m_state(state)
{}

guiDType GuiDataProcessingSplashScreenEnd::getType()
{
	return (guiDType::processingSplashScreenEnd);
}

// **** GuiDataProcessingSplashSignalCancel ****

GuiDataProcessingSplashSignalCancel::GuiDataProcessingSplashSignalCancel()
{}

guiDType GuiDataProcessingSplashSignalCancel::getType()
{
	return guiDType::processingSplashScreenEnableCancel;
}

// **** GuiDataProcessingSplashScreenForceClose ****

GuiDataProcessingSplashScreenForceClose::GuiDataProcessingSplashScreenForceClose()
{}

guiDType GuiDataProcessingSplashScreenForceClose::getType()
{
	return guiDType::processingSplashScreenForceClose;
}


// **** GuiDataSplashScreenStart ****

GuiDataSplashScreenStart::GuiDataSplashScreenStart(const QString& text, const SplashScreenType& type)
	: m_text(text)
	, m_type(type)
{}

guiDType GuiDataSplashScreenStart::getType()
{
	return (guiDType::splashScreenStart);
}

// **** GuiDataSplashScreenEnd ****

GuiDataSplashScreenEnd::GuiDataSplashScreenEnd(const SplashScreenType& type)
	: m_type(type)
{}

guiDType GuiDataSplashScreenEnd::getType()
{
	return (guiDType::splashScreenEnd);
}

// **** GuiDataChangeCursor ****

GuiDataChangeCursor::GuiDataChangeCursor(Qt::CursorShape shape)
	: m_shape(shape)
{}

guiDType GuiDataChangeCursor::getType()
{
	return guiDType::cursorChange;
}