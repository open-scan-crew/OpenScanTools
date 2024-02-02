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
//  DESCRIPTION: Header for Xref FileLock & Consistency Check

#ifndef   _XREFLOCK_H
#define   _XREFLOCK_H


#include "rxnames.h"
#include "dbidar.h"
#pragma pack (push, 8)

class AcDbDatabase;
class AcDbIdMapping;
class AcDwgFileHandle;

class  AcEdXrefFileLock
{
public:
     ACDBCORE2D_PORT AcEdXrefFileLock();
    ACDBCORE2D_PORT ~AcEdXrefFileLock();

    ACDBCORE2D_PORT Acad::ErrorStatus lockFile(
                         int& retstatus,
                         const AcDbObjectId& selectedBTRid);

    ACDBCORE2D_PORT Acad::ErrorStatus releaseFile(
                        int& retstatus,
                        bool bSkipSaveback = false,
                        bool bReload = true);

    ACDBCORE2D_PORT Acad::ErrorStatus consistencyCheck(
                         int& retstatus,
                         const AcDbObjectId& selectedBTRid,
                         AcDbObjectIdArray& outofSyncBTRids);

    ACDBCORE2D_PORT Acad::ErrorStatus consistencyCheck(
                         int& retstatus,
                         const AcDbObjectId& selectedBTRid);

    ACDBCORE2D_PORT Acad::ErrorStatus consistencyChecklocal(
                         int& retstatus,
                         const AcDbObjectId& selectedBTRid);

    ACDBCORE2D_PORT Acad::ErrorStatus xloadctlType(
                         int& retstatus,
                         const AcDbObjectId& selectedBTRid);

    ACDBCORE2D_PORT int               getlockxloadctlType() const { return mXldctltype; }

    ACDBCORE2D_PORT const AcDbObjectIdArray& getoutofSyncIdArray() const { return mXBTRidArray; }

    ACDBCORE2D_PORT Acad::ErrorStatus reloadFile(
                         int& retstatus,
                         const AcDbObjectIdArray& BTRids,
                         const int xldctltype = -1);

    ACDBCORE2D_PORT Acad::ErrorStatus reloadFile(
                         int& retstatus,
                         const int xldctltype = -1);

    ACDBCORE2D_PORT void              setInternalTNmgmt(bool bTNmgmt);
    ACDBCORE2D_PORT AcDbObjectId      getLockBtrId() const { return mXBTRid; }

private:
    AcDwgFileHandle*    mdwgfile;
    AcDbDatabase*       mpXDB;
    AcDbDatabase*       mpXDBlockFile;
    int                 mXldctltype;
    AcDbObjectId        mXBTRid;
    AcDbObjectIdArray   mXBTRidArray;
    bool                mbIntTNmgmt;
};


/// <summary>
/// The constructor locks the XREF file, checks consistency, and reloads all releated 
/// databases than need to be reloaded. The destructor saves changes made to the XREF
/// database back to the XREF file and releases the lock on the XREF file. If there 
/// were some objects in the XREF database that are open, it temporarily closes them 
/// before doing the save back to the XREF file, and reopens them after the save.
/// </summary>
/// <remarks> This class is at this time for internal use only. </remarks>
///
class  AcDbXrefFileLocker
{
public:
    /// <summary>
    /// Locks the XREF file, checks consistency, and reloads all releated databases than 
    /// need to be reloaded. If the database ix NULL or not an XREF database, it does not
    /// nothing ant treats this condition as an expected condition, so that client code 
    /// does not need to check what database it passes to the constructor. If there are 
    /// any opened objects and the XREF database would need to be reloaded, it returns 
    /// eXrefReloaded ErrorStatus, because it is not possible to reload a database when 
    /// it has some opened objects in it. </summary>
    ///    
    ACDBCORE2D_PORT explicit AcDbXrefFileLocker(AcDbDatabase* pXrefDatabase);

    /// <summary> Just calls release(). </summary>
    ///
    ACDBCORE2D_PORT virtual ~AcDbXrefFileLocker(); 

    /// <summary>
    /// Saves changes made to the XREF database back to the XREF file and releases the file lock.
    /// Subsequent release() calls are no-ops. setSaveBack() allows to control whether to save the 
    /// changes back to the file or not. If status() != Acad::eOk, it does not update the XREF file.
    /// </summary>
    ///
    ACDBCORE2D_PORT void release();

    /// <summary>
    /// If anything went wrong, non-eOk ErrorStatus is returned.
    /// </summary>
    ///
    ACDBCORE2D_PORT Acad::ErrorStatus status() const;

    /// <summary>
    /// By default changes made to the XREF database are saved back to the XREF file 
    /// on release() (or destructor) call. This allows not to save back to the XREF file, 
    /// e.g. when the client operation didn't succeed.
    /// </summary>
    ///
    ACDBCORE2D_PORT void setSaveBackToXrefFile(bool yesNo);

    ACDBCORE2D_PORT static bool isXrefDatabase(const AcDbDatabase* pDatabase);

private: // methods

    /// <summary>
    /// Collects all AcDbObjects in the XREF database that are opened and that need to be 
    /// temporarily closed before the modified XREF database is saved to the file, and then 
    /// reopened.
    /// </summary>
    ///
    void collectAllOpenedObjects(AcDbObjectIdArray& openedObjectIds);

    bool hasOpenedObjects();

    // Temporarily close the opened objects in the XREF database, so that no objects 
    // are open when saving the XREF database to file. Reopen the objects back to their 
    // previous open states after the save is completed
    //
    void closeOpenedObjects (AcDbObjectIdArray& openedObjectIds, AcArray<AcDb::OpenMode>& openModes, AcArray<int>& openCounts);
    void reopenClosedObjects(const AcDbObjectIdArray& openedObjectIds, const AcArray<AcDb::OpenMode>& openModes, const AcArray<int>& openCounts);

private: // data

    AcDbDatabase*     mpXrefDatabase;
    AcEdXrefFileLock  mXrefFileLock;
    Acad::ErrorStatus mStatus;
    bool              mSaveBackToXrefFile;

    // Disable
    AcDbXrefFileLocker();
    AcDbXrefFileLocker(const AcDbXrefFileLocker&);
    AcDbXrefFileLocker& operator =(AcDbXrefFileLocker&);
};

#pragma pack (pop)
#endif  // !_XREFLOCK_H
