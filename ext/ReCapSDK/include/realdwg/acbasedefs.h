//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2020 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//////////////////////////////////////////////////////////////////////////////
//
#pragma once

#ifndef  ACBASEDEFS_H
#define ACBASEDEFS_H

//API exported by ac1stXX.dll

#ifdef ACBASE_API
#define ACBASE_PORT ADESK_EXPORT
#else
#define ACBASE_PORT
#endif // ACBASE_API

#endif //ACBASEDEFS_H
