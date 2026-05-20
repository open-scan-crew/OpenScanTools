OpenScanTools – Project Evolution Notice
==============================================================================

OpenScanTools has significantly evolved over time and gained a growing user base.
It has become increasingly difficult to maintain this project as a fully public open-source repository, due to a lack of active contributors and the scale of the development, maintenance and support work required.
To ensure the long-term sustainability of the software and allow continued development under manageable conditions, the project is now transitioning to a new development model.

Public Legacy Repository
==============================================================================

This public repository will remain available as the historical open source version of OpenScanTools under the GPLv3 license.
The latest version of OpenScanTools available is v1.1.2.
This repository will continue to provide:
* access to the existing source code,
* the current feature set,
* community visibility,
* historical reference for the project.
You can still use, modify, and redistribute it freely.

Future Development (v2.0+)
==============================================================================

Future active development will move to a private repository maintained by Yan Koch (company Scankukai).
This new development branch will introduce:
* new advanced features,
* bug fixes,
* long-term product support,
* professional services,
* Core mode (free): includes all historical features from v1.1.2,
* Professional mode: includes all Core mode features + new advanced features.

New versions (v2.0+) will be distributed as closed-source binaries with license management.
The existing public repository will therefore become a legacy/community environment, while the private branch will continue the main evolution of the software.




Prerequisites for building the project OpenScanTools in Debug, Release

Install Qt 5
==============================================================================

* Download the installer at https://www.qt.io/download-qt-installer
* Install version 5.15.2 and the corresponding binaries for your developpement environment (i.e. MSVC 2019 64-bit)
* /!\ Qt 6 is not supported.
* source code and optional Qt modules are not needed (it can save a lot of space)
* Allow for executing binaries by adding the current path to your "PATH" environment variable: <qt_install_dir>5.15.2\msvc2019_64\bin\

Install the Vulkan SDK
==============================================================================

* Download the version 1.2.148 (or higher) at https://vulkan.lunarg.com/sdk/home
* Install the SDK. You can let the default installation location or choose another. In any case the ENV variables are automatically configured.
* Warning: if your user name (in Windows) contains a space, you may have shaders compiling errors in case the code is copied in your user folder. If so, move the code in a path with no space.

Configure Visual Studio
==============================================================================

* General project properties:
    * right-click on the project "OpenScanTools" -> Properties
    * Debugging -> Working Directory -> set to `$(SolutionDir)`
    * Do this for every compilation configuration

Install Qt VS Tool:
==============================================================================

It is a plugin needed to compile the Qt dependencies (moc, ui, qrc).

You can find it in Tools->Extensions and Updates->Qt Visual Stutio Tools.

Add the path where Qt is installed on your machine in:

* Qt VS Tools->Qt Options->Qt Versions->Add (i.e. `C:\Qt\5.12.x\msvc2017_64`)

Generate doc:
==============================================================================
* Download the doxygen 1.9.1 at https://www.doxygen.nl/download.html
* Install it. The ENV variables aren't automatically configured. Add the doxygen/bin path to your system path.
* You can generate the doc (in the "doc" folder) with generateDoc.bat

  
Build with Inno Setup:
==============================================================================
* Download and install the last version of Inno Setup : https://jrsoftware.org/isdl.php
* Launch OpenScanTools.sln solution with Visual Studio
