3DxWare SDK 4.0 for Microsoft Windows


1. INTRODUCTION
---------------
The 3DxWare SDK provides a single interface to the 3DxWare driver 3D
navigation software.  

In addition to this SDK, a current installation of 3Dconnexion 3DxWare
version 10.6.5 or newer is required.
See http://www.3Dconnexion.com/ for the latest 3DxWare release.


2. IMPORTANT NOTES
------------------
Only release (non-debug) libraries and assemblies are included.


3. SAMPLE DESKTOP PROGRAMS
--------------------------
(For web applications, see the "3DconnexionJS" section below.)

Three sample desktop programs are provided with source and Microsoft
Visual Studio solutions.  They can be rebuilt with these solutions (see
notes below).

Microsoft Visual Studio version 2017 or newer
is required. The "Community" edition can also be used.

C/C++ Applications must link with a static library called
"TDxNavLib.lib". It provides stubs for the driver routines that live in
the DLL. 

C# Applications must reference the "TDx.SpaceMouse.Navigation3D.dll"
assembly. The assembly is installed by 3DxWare driver in the Global
Assembly Cache (GAC).


3.1 BUILD NOTES
--------------
Each sample can be rebuilt by opening the corresponding Visual Studio
solution in the main subdirectory for each demo.

<3DxWare SDK>\samples\navlib_viewer
This is a C++ sample and requires "Microsoft Foundation Classes for C++".

When building an application, the 3DxWare libraries must be linked in.
In Visual Studio, this is done by selecting Project->Properties->
Linker->Input and General.  Add the libraries to Input and the paths to
General. 

The library paths are:
<3DxWare SDK>\lib\<plaftorm>\TDxNavLib.lib

You will also need to add the <3DxWare SDK>\inc path to the 
application's settings to pick up the header (.h) files.

<3DxWare SDK>\samples\3DxTestNL
This is a C# sample and references the 3DxWare assembly 
<3DxWare SDK>\lib\bin\TDx.SpaceMouse.Navigation3D.dll

<3DxWare SDK>\samples\3DxTraceNL
This is a C++ sample that does not depend on any external reference.


4. 3DCONNEXONJS FRAMEWORK FOR WEB APPLICATIONS
----------------------------------------------
Also included in this SDK is the documentation and samples (web
applications) for the "3DconnexionJS", a JavaScript framework to
develop web applications using the "Navigation Library".

For additional details, refer to the "readme_web.txt" text file
and the PDF documentation in "web" sub-directory.


5. TECHNICAL SUPPORT SERVICES
-----------------------------
Visit www.3Dconnexion.com for end-user and developer technical support.

Meet 3Dconnexion engineering staff in the 3Dconnexion Forum at
http://www.3Dconnexion.com/forum


6. LICENSE
----------
The 3DxWare SDK is licensed according to the terms of the 3Dconnexion
Software Development Kit License Agreement. The Agreement is documented
in the "LicenseAgreementSDK.txt" file included with this SDK.

All materials included in this SDK are copyright 3Dconnexion:

Copyright (c) 2011-2020 3Dconnexion. All rights reserved.
