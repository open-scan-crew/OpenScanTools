#ifndef GUI_DATA_MESSAGES_H
#define GUI_DATA_MESSAGES_H

#include "gui/GuiData/IGuiData.h"

#include <QtCore/qstring.h>

class GuiDataWarning : public IGuiData
{
public:
	GuiDataWarning(const QString& message);
	guiDType getType() override;
public:
	const QString m_message;
};

class GuiDataModal : public IGuiData
{
public:
	GuiDataModal(const uint32_t& flags, const QString& message);
	guiDType getType() override;
public:
	const QString m_message;
	const uint32_t m_flags;
};

class GuiDataInfo : public IGuiData
{
public:
	GuiDataInfo(const QString& message, bool isModal);
	guiDType getType() override;
public:
	const QString m_message;
	bool m_isModal;
};

class GuiDataTmpMessage : public IGuiData
{
public:
	GuiDataTmpMessage(const QString& message="", const int& timeout = 0);
	guiDType getType() override;
public:
	const QString m_message;
	const int m_timeout;
};

class GuiDataProcessingSplashScreenStart : public IGuiData
{
public:
	GuiDataProcessingSplashScreenStart(const uint64_t& maxStep, const QString& label, const QString& state);
	guiDType getType() override;
public:
	const QString m_label;
	const QString m_state;
	const uint64_t m_maxStep;
};

class GuiDataProcessingSplashScreenProgressBarUpdate : public IGuiData
{
public:
	GuiDataProcessingSplashScreenProgressBarUpdate(const QString& state, const uint64_t& step);
	guiDType getType() override;
public:
	const uint64_t m_step;
	const QString m_state;
};

class GuiDataProcessingSplashScreenEnableCancelButton : public IGuiData
{
public:
	GuiDataProcessingSplashScreenEnableCancelButton(bool enableCancel);
	guiDType getType() override;
public:
	const bool m_enableCancel;
};

class GuiDataProcessingSplashScreenLogUpdate : public IGuiData
{
public:
	GuiDataProcessingSplashScreenLogUpdate(const QString& log);
	guiDType getType() override;
public:
	const QString m_log;
};


class GuiDataProcessingSplashScreenEnd : public IGuiData
{
public:
	GuiDataProcessingSplashScreenEnd(const QString& state);
	guiDType getType() override;
public:
	const QString m_state;
};

class GuiDataProcessingSplashSignalCancel : public IGuiData
{
public:
	GuiDataProcessingSplashSignalCancel();
	guiDType getType() override;
};

class GuiDataProcessingSplashScreenForceClose : public IGuiData
{
public:
	GuiDataProcessingSplashScreenForceClose();
	guiDType getType() override;
};

class GuiDataSplashScreenStart : public IGuiData
{
public:
	enum class SplashScreenType {Display, Message};
public:
	GuiDataSplashScreenStart(const QString& text, const SplashScreenType& type);
	guiDType getType() override;
public:
	const QString m_text;
	const SplashScreenType m_type;
};

class GuiDataSplashScreenEnd : public IGuiData
{
public:
	enum class SplashScreenType { Display, Message };
public:
	GuiDataSplashScreenEnd(const SplashScreenType& type);
	guiDType getType() override;
public:
	const SplashScreenType m_type;
};

class GuiDataChangeCursor : public IGuiData
{
public:
	GuiDataChangeCursor(Qt::CursorShape shape = Qt::CursorShape::ArrowCursor);
	guiDType getType() override;
public:
	Qt::CursorShape m_shape;
};

#endif //! GUI_DATA_MESSAGES_H
