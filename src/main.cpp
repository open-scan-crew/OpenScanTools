#include <QtWidgets/QApplication>
#include "gui/Translator.h"
#include <qevent.h>
#include <thread>

#include "utils/Config.h"
#include "utils/Logger.h"
#include "utils/OpenScanToolsVersion.h"
#include "controller/Controller.h"
#include "controller/ControlListener.h"
#include "gui/Gui.h"
#include "gui/DataDispatcher.h"
#include "gui/widgets/SplashScreen.h"
#include "gui/widgets/FocusWatcher.h"

#include "core/SignalHandler.h"
#include "impl/PCE_impl.h"
#include "controller/controls/ControlProject.h"
#include "controller/controls/ControlApplication.h"

#include "models/3d/Graph/OpenScanToolsGraphManager.h"

#include "test/SafeDataTests.h"

#ifdef _DEBUG_
#define TL_BUILD "Debug  "
constexpr bool VK_VALIDATION_ENABLED = true;
#else
#define TL_BUILD "Release"
constexpr bool VK_VALIDATION_ENABLED = false;
#endif
 
#define HEADER(build, vers, date) "\
*******************************************************************************\n\
** OpenScanTools - OST                                                                   **\n\
** "##build" v"##vers"  "##date"                                             **\n\
** (C)OpenScanTools 2024                                                           **\n\
*******************************************************************************\n"
#ifdef _WIN32
#include <windows.h>
#endif

#ifndef _DEBUG_
#ifdef _WIN32
HHOOK hHook = NULL;
QApplication* myApp = nullptr;
Gui* myTarget = nullptr;

LRESULT CALLBACK MyLowLevelKeyBoardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (wParam == WM_KEYDOWN)
	{
		//Get the key information
		KBDLLHOOKSTRUCT cKey = *((KBDLLHOOKSTRUCT*)lParam);
		if (cKey.vkCode == 44) //PrintScreen key code
		{
			QKeyEvent* printScreenEvent = new QKeyEvent(QEvent::KeyRelease, Qt::Key_ScreenSaver, Qt::NoModifier);
			myApp->postEvent(myTarget, printScreenEvent);
		}
	}
	return CallNextHookEx(hHook, nCode, wParam, lParam);
}
#endif
#endif

#include <QGuiApplication>

int main(int argc, char** argv)
{
	std::setlocale(LC_ALL, "");
	// /!\ Ne pas enlever, c'est important pour l'import/export de fichiers 
	std::setlocale(LC_NUMERIC, "C");

	std::filesystem::path programPath(argv[0]);

	Logger::init(programPath);
	Logger::log() << "\n" << HEADER(TL_BUILD, OPENSCANTOOLS_VERSION, "25/04/2024") << LOGENDL;

	// Disable the high dpi scaling because it causes rendering problems with Vulkan
	QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
	QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	/*
	{
		using namespace tl::tests;
		logSizeof();

		data::upcastTest();
		data::integrityTest(3, 42);
		for (int j = 0; j < 1000; j++)
		{
			integrityTest(rand());
		}

		massTest_1(1000);
		massTest_1(10000);
		massTest_1(100000);
		massTest_1(1000000);
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));

		massTest_2(1000);
		massTest_2(10000);
		massTest_2(100000);
		massTest_2(1000000);
		std::this_thread::sleep_for(std::chrono::milliseconds(10000));
	}
	*/
	
	QApplication app(argc, argv);

#ifdef _DEBUG_
	Translator appTranslator(&app);
#else
	std::filesystem::path dir(QCoreApplication::applicationDirPath().toStdString());
	Translator appTranslator(&app, dir / "translations");
#endif // DEBUG

	appTranslator.setActiveLangage(Config::getLangage());

	app.setWindowIcon(QIcon(":/resources/images/OpenScanTools-cmjn-ye-bl.ico"));

	QIcon::setThemeName("OpenScanTools");

	// Parse the args
	bool vk_validation_enabled = VK_VALIDATION_ENABLED;
	bool select_device = false;

	for (int i = 0; i < argc; i++)
	{
		Logger::log() << "arg[" << i << "] : [" << argv[i] << "]" << LOGENDL;
		if (strcmp(argv[i], "--vulkan-validation") == 0)
			vk_validation_enabled = true;
		if (strcmp(argv[i], "--select-device") == 0)
			select_device = true;
	}

	if (!tl_pce_init(vk_validation_enabled, select_device))
		return 0;

#ifndef _DEBUG_
	SplashScreen splash;

	splash.show();
	splash.showStatusMessage("Loading controller");
#endif

    // Set the Config before the initializing the other modules
    Config::setApplicationDirPath(QCoreApplication::applicationDirPath().toStdWString());
	DataDispatcher dataDispatcher;
	OpenScanToolsGraphManager graphManager(dataDispatcher);
	Controller controller(dataDispatcher, graphManager);
	SignalHandler signalHandler(&dataDispatcher);

	Gui gui(controller, &appTranslator);

	FocusInOutWatcher focus(&app);
	QObject::connect(&focus, &FocusInOutWatcher::focusIn, [&gui]() {gui.onEditing(true); });
	QObject::connect(&focus, &FocusInOutWatcher::focusOut, [&gui]() {gui.onEditing(false); });

#ifndef _DEBUG_
#ifdef _WIN32
	myApp = &app;
	myTarget = &gui;
	hHook = SetWindowsHookEx(WH_KEYBOARD_LL, MyLowLevelKeyBoardProc, NULL, 0);
	if (hHook == NULL)
	{
		Logger::log() << "\n" << "Hook Failed" << LOGENDL;
	}
#endif
#endif
	// Launch the controller in a separate thread with a fixed framerate
	std::thread controllerThread(&Controller::run, &controller, 60);

	if (argc >= 2)
	{
		std::filesystem::path path(argv[1]);
		if (path.extension() == ".tlp" || path.extension() == ".TLP")
		{
#ifndef _DEBUG_
			splash.showMessage("Loading Project");
#endif
			controller.getControlListener()->notifyUIControl(new control::project::DropLoad(path));
		}
	}

#ifndef _DEBUG_
	splash.showStatusMessage("Loading GUI");
#endif

	//Logger::log() << "Gui start" << LOGENDL;
	gui.launch();

#ifndef _DEBUG_
	splash.finish(&gui);
	gui.showAuthorManager();
#endif
	app.exec();

#ifndef _DEBUG_
	splash.show();

	splash.showStatusMessage("Stop controller");
#endif

	dataDispatcher.setActive(false);

    // Close the application
	//th.join();
    controller.stop();
	controllerThread.join();

#ifndef _DEBUG_
	splash.showStatusMessage("Unloading GPU Resources");
#endif

    tl_pce_shutdown();

#ifndef _DEBUG_
    splash.showStatusMessage("Destroy all main objects");
#endif
	return (0);
}