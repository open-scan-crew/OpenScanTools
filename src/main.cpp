#include "utils/Config.h"
#include "utils/Logger.h"
#include "controller/Controller.h"
#include "controller/ControlListener.h"
#include "gui/Gui.h"
#include "gui/Translator.h"
#include "gui/DataDispatcher.h"
#include "gui/widgets/SplashScreen.h"
#include "gui/widgets/FocusWatcher.h"

#include "core/SignalHandler.h"
#include "impl/PCE_impl.h"
#include "controller/controls/ControlProject.h"

#include "models/3d/Graph/OpenScanToolsGraphManager.h"

#include <thread>
#include <qevent.h>
#include <QGuiApplication>

#ifdef _DEBUG_
constexpr bool VK_VALIDATION_ENABLED = false;
#else
constexpr bool VK_VALIDATION_ENABLED = false;
#endif


#if !defined(_DEBUG_) && defined(_WIN32)
#include <windows.h>
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

int main(int argc, char** argv)
{
    std::setlocale(LC_ALL, "");
    // /!\ Ne pas enlever, c'est important pour l'import/export de fichiers 
    std::setlocale(LC_NUMERIC, "C");

    std::filesystem::path programPath(argv[0]);

    Logger::init(programPath);

    // Disable the high dpi scaling because it causes rendering problems with Vulkan
    QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

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
        Logger::log(LoggerMode::LogConfig) << "arg[" << i << "] : [" << argv[i] << "]" << LOGENDL;
        if (strcmp(argv[i], "--vulkan-validation") == 0)
            vk_validation_enabled = true;
        if (strcmp(argv[i], "--select-device") == 0)
            select_device = true;
    }

    if (!tl_pce_init(vk_validation_enabled, select_device))
        return 0;

    SplashScreen splash;
    splash.show();
    splash.showStatusMessage("Loading controller");

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

#if !defined(_DEBUG_) && defined(_WIN32)
    myApp = &app;
    myTarget = &gui;
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, MyLowLevelKeyBoardProc, NULL, 0);
    if (hHook == NULL)
    {
        Logger::log() << "\n" << "Hook Failed" << LOGENDL;
    }
#endif

    if (argc >= 2)
    {
        std::filesystem::path path(argv[1]);
        if (path.extension() == ".tlp" || path.extension() == ".TLP")
        {
            splash.showMessage("Loading Project");
            controller.getControlListener()->notifyUIControl(new control::project::DropLoad(path));
        }
    }

    splash.showStatusMessage("Loading GUI");

    gui.launch();
    splash.finish(&gui);

    gui.showAuthorManager();

    // Main Qt loop
    app.exec();

    splash.show();
    splash.showStatusMessage("Stop controller");

    dataDispatcher.setActive(false);

    // Close the application
    //controller.stop();

    // Close Vulkan application
    splash.showStatusMessage("Unloading GPU Resources");
    tl_pce_shutdown();

    splash.showStatusMessage("Destroy all main objects");

    return (0);
}