Prerequisites for building the project OpenScanTools in Debug, Release

Install Qt
==============================================================================

* Download the installer at https://www.qt.io/download-qt-installer
* Minimum install :
  * Qt 5.15.2
    -> MSVC 2019 64-bit
  * Developer and Designer Tools
    -> Qt Creator 13.0.0
    -> Debugging Tools for Windows

* Add the path "<qt_install_dir>/bin/" to your environment variable "PATH"

Install the Vulkan SDK
==============================================================================

* Download the version 1.2.148 (or higher) at https://vulkan.lunarg.com/sdk/home
* Install the SDK. You can let the default installation location or choose another. In any case the ENV variables are automatically configured.
* Warning: if your user name (in Windows) contains a space, you may have shaders compiling errors in case the code is copied in your user folder. If so, move the code in a path with no space.

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

Dev team:
==============================================================================
* Yan Koch: Founder - Product manager, specialist of industrial 3D scanning, compulsive creator of user stories :-),
* Robin Kervadec: Senior Developer, creator of the 3D point cloud engine (Vulkan), 3D rendering, software architecture, scan conversion, and many cool features...
* Quentin Moiteaux: Junior Developer: many features like the import export of 3D models, tree structure, annotations, animation module...
* Lucas Silve: advanced Maths, objects detection, raytracing, analysis tools...
* Aurélien Milliat: Senior Developer: software architecture, clipping & multi-clipping features, gridded boxes, VR specialist and Teacher (I'm sorry we didn't create a VR module in OST).
