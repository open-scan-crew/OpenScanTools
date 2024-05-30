Prerequisites for building the project OpenScanTools in Debug, Release

Install Qt
==============================================================================

* Download the installer at https://www.qt.io/download-qt-installer
* Choose any of the version 5.15.x proposed (5.15 is a LTS)

Install the Vulkan SDK
==============================================================================

* Download the version 1.2.148 (or higher) at https://vulkan.lunarg.com/sdk/home
* Install the SDK. You can let the default installation location or choose another. In any case the ENV variables are automatically configured.

Configure Visual Studio
==============================================================================

* General project properties:
    * right-click on the project "OpenScanTools" -> Properties -> Debugging -> Working Directory -> set to `$(SolutionDir)`

Install Qt VS Tool:
==============================================================================

It is a plugin needed to compile the Qt dependencies (moc, ui, qrc).

You can find it in Tools->Extensions and Updates->Qt Visual Stutio Tools.

Add the path where Qt is installed on your machine in:

* Qt VS Tools->Qt Options->Qt Versions->Add (i.e. `C:\Qt\5.12.x\msvc2017_64`)

WARNING : The version 2.6.0.7 is stable but some bugs have been observed after an update. It is recommended to disable the automatic update of this plugin.

Change project working repo:
==============================================================================
* Right-click project "OpenScanTools" in Visual Studio solution tree and click "Properties"
* In "Debug", change the working repositery value "$(ProjectDir)" into "$(SolutionDir)"
* Do this for every compilation configuration


Generate doc:
==============================================================================
* Download the doxygen 1.9.1 at https://www.doxygen.nl/download.html
* Install it. The ENV variables aren't automatically configured. Add the doxygen/bin path to your system path.
* You can generate the doc (in the "doc" folder) with generateDoc.bat

  
Build with Inno Setup:
==============================================================================
* Download and install the last version of Inno Setup : https://jrsoftware.org/isdl.php
* Launch OpenScanTools.sln solution with Visual Studio
* Generate solution with "Release_Prod" configuration.
* Launch OpenScanTools.iss in \tools\InstallScripts with Inno Setup
* Run (F9) the script
* The .exe setup is generated in \tools\InstallScript\Output\

