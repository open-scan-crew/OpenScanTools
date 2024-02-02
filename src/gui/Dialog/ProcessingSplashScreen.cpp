#include "gui/Dialog/ProcessingSplashScreen.h"
#include "gui/GuiData/GuiDataMessages.h"
#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/qregularexpression.h>
#include "controller/controls/ControlFunction.h"


#if defined(_WIN32)
#include <windows.h>
#include <synchapi.h>
#endif

ProcessingSplashScreen::ProcessingSplashScreen(IDataDispatcher& dataDispatcher, QWidget *parent)
	: m_dataDispatcher(dataDispatcher)
	, QDialog(nullptr)
	, ui(new Ui::processingSplashScreen())
	, m_forceStop(true)
{
	ui->setupUi(this);
	setModal(true);

	setWindowFlags(Qt::WindowType::MSWindowsFixedSizeDialogHint | 
					Qt::WindowType::FramelessWindowHint | 
					Qt::WindowType::WindowSystemMenuHint);

	connect(this, &ProcessingSplashScreen::onLabelValueChange, ui->label, &QLabel::setText);
	connect(this, &ProcessingSplashScreen::onProgressBarRangeChange, ui->progressBar, &QProgressBar::setRange);
	connect(this, &ProcessingSplashScreen::onProgressBarValueChange, ui->progressBar, &QProgressBar::setValue);
	connect(this, &ProcessingSplashScreen::onProgressBarTitleChange, ui->progressBar, &QProgressBar::setFormat);
	connect(this, &ProcessingSplashScreen::getProgressBarValue, ui->progressBar, &QProgressBar::format, Qt::BlockingQueuedConnection);
	connect(this, &ProcessingSplashScreen::onOkButtonShow, ui->okButton, &QPushButton::show);
	connect(this, &ProcessingSplashScreen::onOkButtonShow, ui->cancelButton, &QPushButton::hide);
	connect(this, &ProcessingSplashScreen::onOkButtonHide, ui->okButton, &QPushButton::hide);
	connect(this, &ProcessingSplashScreen::onOkButtonHide, ui->cancelButton, &QPushButton::show);
	connect(this, &ProcessingSplashScreen::onLogBrowserClear, ui->logBrowser, &QTextBrowser::clear);
	connect(this, &ProcessingSplashScreen::onLogBrowserAppend, ui->logBrowser, &QTextBrowser::append);
	connect(ui->okButton, &QPushButton::released, this, &ProcessingSplashScreen::hide);
	connect(ui->cancelButton, &QPushButton::released, this, &ProcessingSplashScreen::onCancel);


	m_dataDispatcher.registerObserverOnKey(this, { guiDType::processingSplashScreenStart });
	m_dataDispatcher.registerObserverOnKey(this, { guiDType::processingSplashScreenProgressBarUpdate });
	m_dataDispatcher.registerObserverOnKey(this, { guiDType::processingSplashScreenEnableCancel });
	m_dataDispatcher.registerObserverOnKey(this, { guiDType::processingSplashScreenLogUpdate });
	m_dataDispatcher.registerObserverOnKey(this, { guiDType::processingSplashScreenEnd });
	m_dataDispatcher.registerObserverOnKey(this, { guiDType::processingSplashScreenForceClose });
}

ProcessingSplashScreen::~ProcessingSplashScreen()
{}

void ProcessingSplashScreen::informData(IGuiData *keyValue)
{
	switch (keyValue->getType())
	{
		case  guiDType::processingSplashScreenStart:
		{
			GuiDataProcessingSplashScreenStart* info = static_cast<GuiDataProcessingSplashScreenStart*>(keyValue);
			show();
			emit onLabelValueChange(info->m_label);
			emit onProgressBarTitleChange(info->m_state);
			emit onProgressBarRangeChange(0, info->m_maxStep);
			emit onProgressBarValueChange(0);
			emit onLogBrowserClear();
			emit onOkButtonHide();
			m_forceStop = false;
			if (!m_progressbarWatcher.isRunning())
			{
				QFuture<void> futurePB = QtConcurrent::run(this, &ProcessingSplashScreen::updateProgressbarTitle);
				m_progressbarWatcher.setFuture(futurePB);
			}
		}
		break;
		case  guiDType::processingSplashScreenProgressBarUpdate:
		{
			GuiDataProcessingSplashScreenProgressBarUpdate* info = static_cast<GuiDataProcessingSplashScreenProgressBarUpdate*>(keyValue);
			emit onProgressBarValueChange(info->m_step);
			emit onProgressBarTitleChange(info->m_state);
			
		}
		break;
		case guiDType::processingSplashScreenEnableCancel:
		{
			GuiDataProcessingSplashScreenEnableCancelButton* info = static_cast<GuiDataProcessingSplashScreenEnableCancelButton*>(keyValue);
			ui->cancelButton->setEnabled(info->m_enableCancel);
		}
		break;
		case  guiDType::processingSplashScreenLogUpdate:
		{
			GuiDataProcessingSplashScreenLogUpdate* info = static_cast<GuiDataProcessingSplashScreenLogUpdate*>(keyValue);
			if (info->m_log.size())
				emit onLogBrowserAppend(info->m_log);
			else
				emit onLogBrowserClear();
		}
		break;
		case  guiDType::processingSplashScreenEnd:
		{
			GuiDataProcessingSplashScreenEnd* info = static_cast<GuiDataProcessingSplashScreenEnd*>(keyValue);
			emit onOkButtonShow();
			emit onProgressBarTitleChange(info->m_state);
			m_forceStop = true;
		}
		break;
		case guiDType::processingSplashScreenForceClose:
		{
			m_forceStop = true;
			hide();
		}
		break;
	}
}

void ProcessingSplashScreen::onCancel()
{
	m_dataDispatcher.sendControl(new control::function::Abort());
	m_dataDispatcher.updateInformation(new GuiDataProcessingSplashSignalCancel());
}

void ProcessingSplashScreen::closeEvent(QCloseEvent *event)
{
	onCancel();
}

void ProcessingSplashScreen::updateProgressbarTitle()
{
	while (!m_forceStop)
	{
		Sleep(500);
		static uint8_t counter(0);
		QString format(emit getProgressBarValue());
		format.remove(QRegularExpression("~+"));
		switch (counter)
		{
		case 0:
			break;
		case 1:
			format.append("~");
			format.insert(0, "~");
			break;
		case 2:
			format.append("~~");
			format.insert(0, "~~");
			break;
		case 3:
			format.append("~~~");
			format.insert(0, "~~~");
			break;
		case 4:
			counter = 0;
		}
		counter++;
		emit onProgressBarTitleChange(format);
	}
}