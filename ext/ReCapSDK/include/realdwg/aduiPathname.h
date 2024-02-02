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

#include "adesk.h"
#include "acbasedefs.h"
#include "aduipathenums.h"
#include "AcString.h"
#pragma pack (push, 8)

class CAdUiVolumeDescriptor;

AcString GetDriveForRoottypePath(const CAdUiVolumeDescriptor* const);

namespace Autodesk
{
    namespace AutoCAD
    {
        namespace PAL
        {
            namespace FileIO
            {
                class File;
            }
        }
    }
}

// Some oft used char constants for volume and file names
extern __declspec(selectany) const ACHAR kBackslash = ACRX_T('\\');
extern __declspec(selectany) const ACHAR kColon = ACRX_T(':');
extern __declspec(selectany) const ACHAR kDoubleQuote = ACRX_T('"');
extern __declspec(selectany) const ACHAR kEOS = ACRX_T('\0');
extern __declspec(selectany) const ACHAR kPeriod = ACRX_T('.');
extern __declspec(selectany) const ACHAR kSlash = ACRX_T('/');
extern __declspec(selectany) const ACHAR kSpace = ACRX_T(' ');
extern __declspec(selectany) const ACHAR kAsterisk = ACRX_T('*');
extern __declspec(selectany) const ACHAR kQuestionmark = ACRX_T('?');
extern __declspec(selectany) const ACHAR kZero = ACRX_T('0');


class ACBASE_PORT CAdUiPathname {

    friend CAdUiVolumeDescriptor* NewVolDescriptorHelper(
                                                         const CAdUiPathname* pPathName, 
                                                         const AcString& vol_name);

    friend bool IsPathValid(
        CAdUiPathname* pPathName,
        const CAdUiPathname* rel_path,
        const AcString* work);
    
    friend void HandleSpecialPath(
                                  CAdUiPathname* pPathName, 
                                  const CAdUiPathname* rel_path, 
                                  AcString* work, 
                                  int index, 
                                  bool wildcard_ok);
    
    friend void SetupVolForRoottypePath(
                                  CAdUiPathname* pPathName,
                                  const CAdUiPathname* pRelPath,
                                  const wchar_t * pPathStr);

    friend AcString GetDriveForRoottypePath(const CAdUiVolumeDescriptor* );
    
public:

// methods
        // constructor (initializes private data)
        CAdUiPathname(void);
        CAdUiPathname(const CAdUiPathname&);

        // destructor (frees allocated storage)
        virtual ~CAdUiPathname(void);

        // *******************************************
        // Methods to set up a path
        // *******************************************

        // reinitialize the data without destroying the object
        void Empty(void);

        // parse a string, validate the format and fill in the data
        //  This routine actually calls ParseRelative with the
        //  current document path (if any) or the current application
        //  path as the relative path.
        virtual PathErr Parse( const wchar_t *, bool wildcard_ok = false );

        // parse a string relative to a supplied path, validate the
        //  format, and fill in the data.  The third parameter
        //  indicates whether an absolute path will be accepted
        //  or cause an error.  NOTE: this routine will recognize
        //  device names and proceed accordingly.
        PathErr ParseRelative( const wchar_t * name,
                               const CAdUiPathname* relpath,
                               bool wildcard_ok = false );

        // parse a string and attempt to find a matching file by OS
        //  rules, ie, whatever the current OS search path is.  Like
        //  an implicit ParseRelative.  NOTE: this does NOT call
        //  ParseRelative, it uses OS calls to find the file and get
        //  the path.  This means that it doesn't recognize device names
        bool ParseAndOSFind( const wchar_t * );

        // create an assignment operator that does the right thing.
        const CAdUiPathname& operator=(const CAdUiPathname& pathSrc);
        const CAdUiPathname& operator=(const CAdUiPathname* pathSrc);
 
        // generate a unique pathname based on this one.
        // returns a pointer to a new CAdUiPathname object if successful
        // returns FALSE if no success after 200 tries.
        // the file will be created and handle be closed. You need to delete this created file if it is not wanted.
        bool GetUniqueName(CAdUiPathname&, 
                           bool alwaysMangle = false ) const;

        // *******************************************
        // Methods to get parts of the path
        // *******************************************

        // return the string to left of last period in
        //  the rightmost component.  Returns NULL if
        //  this is not a file.
        void GetFilename(AcString&) const;

        // return the string to right of last period in 
        //  the rightmost component.  Returns NULL if
        //  this is not a file.
        void GetExtension(AcString&) const;

        // return the rightmost component of the path
        //  (a directory name or file.ext).  If the
        //  path terminates in a slash, return NULL.
        void GetBasename(AcString&) const;

        // return the parent path of the current path
        //  c:\foo is the parent of c:\foo\bar
        void GetParent(CAdUiPathname& ) const;

        // *******************************************
        // Methods to modify the path
        // *******************************************

        // remove the last component from the path leaving
        //  the path terminated with a slash.  If this is a
        //  partial (relative) path, and there is no slash
        //  remaining (the trimmed path would be empty), return
        //  an error.
        PathErr Trim(void);

        // remove a trailing component separator
        //  if the last character is not a slash, do nothing
        //  return an error if removing the slash would leave
        //  an empty path.
        PathErr TrimSlash(void);

        // remove the last component, including the beginning
        //  slash.  Return an error if removing the slash would
        //  leave an empty path.
        PathErr TrimComponent(void);

        // append a string to the end of the path.
        //  This routine doesn't check whether the path
        //  ends in a slash.  Returns an error if the
        //  resulting path would be invalid.
        PathErr Append( const wchar_t * );

        // append a component separator to the end of the path
        //  Does nothing if the path already ends in a slash.
        //  returns an error if the resulting path would be
        //  invalid 
        PathErr AppendSlash(void);

        // append a component separator to the end of the path
        //  followed by the string.  Will never produce a "//"
        //  error.  Returns an error if the resulting path
        //  would be invalid.
        PathErr AppendComponent( const wchar_t * );

        // add a new extension regardless of any existing extension
        //  Will never produce a ".." error.  Returns an error if
        //  the resulting path would be invalid, or if the path
        //  is not a file.
        PathErr AddExtensionAlways( const wchar_t * );

        // add a new extension if there is no existing extension
        // or if the existing extension doesn't match the argument.
        // Note that the argument can be a semi-colon delimited
        // list of extensions.  If the existing extension doesn't
        // match anything in the list, the first extension is used.
        //  Will never produce a ".." error.  Returns an error if
        //  the resulting path would be invalid, or if the path
        //  is not a file.
        PathErr AddExtensionIfNoMatch( const wchar_t * );

        // add a new extension or replace any existing extension.
        //  Will never produce a ".." error.  Returns an error if
        //  the resulting path would be invalid, or if the path
        //  is not a file.
        PathErr ReplaceExtensionAlways( const wchar_t * );

        // add a new extension if there is no existing extension
        // or replace the existing extension if it n doesn't match 
        // the argument.  Note that the argument can be a 
        // semi-colon delimited list of extensions.  If the existing 
        // extension doesn't match anything in the list, the first 
        // extension is used.
        //  Will never produce a ".." error.  Returns an error if
        //  the resulting path would be invalid, or if the path
        //  is not a file.
        PathErr ReplaceExtensionIfNoMatch( const wchar_t * );

        // if there is an extension, remove it
        // removes the '.' too.
        void RemoveExtensionAlways(void);
        
        // if there is an extension, and it matches the argument,
        // remove it.  Note that the argument can be a 
        // semi-colon delimited list of extensions.  
        // removes the '.' too.
        void RemoveExtensionIfMatch( const wchar_t * );

        // modify the case of the basename (name.ext).
        //  mostly used for compatibility mode stuff
        void BaseToLower( void );
        void BaseToUpper( void );

        // strip leading/trailing spaces off of the file name
        void RemoveSpaces( void );

        // *******************************************
        // Methods to get the name in various formats
        // *******************************************

        // return a display name for this path in the argument.
        // Abbreviation rules are applied to reduce the length
        // of the display name if its length is > maxchars
        // If maxchars is -1, then the path is not truncated
        // If maxchars is < 12 and != -1, then effective maxchars is 12
        //
        void GetDisplayName( AcString& strResult, int maxchars ) const;

        // return the pathname as a string
        const wchar_t * GetPathString(void) const;

        // returns the full path starting with the local volume
        //  name.  Relative paths are fully resolved.
        bool GetFullPath( AcString& ) const;
        bool GetFullPath(wchar_t* pBuf, size_t strBuffLen ) const;

        // template allows caller to pass in fixed size array without passing in its size
        template<size_t nBufLen> inline bool GetFullPath(wchar_t (& buf)[nBufLen]) const
        {
            return this->GetFullPath(buf, nBufLen);
        }

        // Returns the UNC path if 'this' represents a remote drive. Otherwise,
        // returns NULL.
        void GetFullUNCPath(AcString&) const;

        // *******************************************
        // Methods to get information about the path
        // *******************************************

        // return TRUE if the path_type is file or dir
        //  (otherwise trim/append is not supported).
        bool IsPath(void) const;

        // return TRUE if the path_type is NO_PATH
        bool IsEmpty(void) const;

        // return the path type of this pathname
        //  (file, dir, device special, empty)
        path_type GetPathType(void) const;

        // return the volume descriptor of this pathname
        const CAdUiVolumeDescriptor* GetVolumeDescriptor(void) const;

        // return TRUE if the two paths refer to the same object
        bool PathMatch( const CAdUiPathname* ) const;
        bool PathMatch( const wchar_t * ) const;

        // checks whether this path points to an existing
        //  file/directory.  Returns FALSE if the path is
        //  empty.
        bool Exists(void);

        // check whether the extension (if any) matches one of
        //  extension in extlist.  extlist is a list of extensions
        //  separated by semicolons.
        bool VerifyExtension( const wchar_t * extlist ) const;

        // return TRUE if path string passed to the object had
        // a prefix.
        bool HadPrefix(void) const;

        // return TRUE if the path string passed to the object
        // was an absolute path.
        bool WasAbsolute(void) const;

        // return TRUE if the path string passed to the object
        // contained a leading separator (e.g., "\file.txt").
        bool WasRoot(void) const;

        // *******************************************
        // Methods that actually affect the file
        // *******************************************

        /// <summary>
        /// Try to open a file in read-only mode
        /// </summary>
        /// <param name="">Input File to open</param>
        /// <returns>Return true if the open succeeded, false otherwise.</returns>
        bool OpenReadOnly(Autodesk::AutoCAD::PAL::FileIO::File &);
        /// <summary>
        /// Try to open a file in write mode
        /// </summary>
        /// <param name="">Input File to open</param>
        /// <returns>Return true if the open succeeded, false otherwise.</returns>
        bool OpenWriteOnly(Autodesk::AutoCAD::PAL::FileIO::File &);

        // Tries to rename the file pointed to by this path
        //  Return TRUE if the operation succeeds, FALSE otherwise.
        //  Note that this routine sets the current directory to
        //  the appropriate place before opening, and resets it
        //  afterward.
        bool Rename(const ACHAR *newname);

        // Tries to remove the file pointed to by this path
        //  Return TRUE if the operation succeeds, FALSE otherwise.
        //  Note that this routine sets the current directory to
        //  the appropriate place before opening, and resets it
        //  afterward.
        bool Remove(void);

        // check status for the path
        path_status Status();

protected:

// data elements

        //the type of the pathname
        path_type                       m_this_type = NO_PATH;

        // pointer to the volume descriptor for this path
        const CAdUiVolumeDescriptor*        m_pvolume = nullptr;

        // the actual path and filename buffer
        //  as part of the general design philosopy, this string
        //      should never be accessable from outside the class or
        //  its descendants.
        AcString*                        m_pathbuffer = nullptr;

        // the relative path buffer
        //  for relative paths, this buffer contains the base path.
        //  the base path is always an absolute path.
        AcString*                        m_relpath = nullptr;

        // ****
        path_category                   m_path_cat;

        // TRUE if the original path string parsed via PathRelative
        // had a prefix in it.
        bool                            m_had_prefix = false;

// methods

        // find or create (as required), the volume descriptor
        //  for the specified local name (drive letter, etc.)
        const CAdUiVolumeDescriptor* FindVolumeDescriptor(const AcString& vol_name) const;

        // create a new volume descriptor
        const CAdUiVolumeDescriptor* NewVolDescriptor( const AcString& vol_name) const;

        // get the file system type of a volume
        static FS_TYPE GetFileSystem(AcString& vol_name);

        // return TRUE if the path is a device name (OS dependent)
        static bool IsDeviceName(const wchar_t * );

        // If string starts with optional whitespace followed by a double quote, and
        // it ends with a double quote followed by optional whitespace, then the
        // double quotes and optional whitespace are stripped off.
        // Returns true if the string was modified, false otherwise
        //
        static bool Unquote( AcString& );

        // get the native OS Separator
        static ACHAR GetSeparator(void);

        // the guts of the assignment operator
virtual void AssignCopy( const CAdUiPathname& );

        // Returns the "last character" of an AcString (AcString).
        // Was more useful in Ansi narrow char releases.
        // Now deprecated - please use AcString methods directly.
        //
        static ACHAR GetLastCharacter( const AcString& s)
        {
            const int kLen = s.length();
            return (kLen == 0) ? L'\0' : s[kLen-1];
        }
        
private:
        void GetAttributes(const AcString* path,
                const AcString* relpath,
                path_type* this_type);
};


// inline definitions

inline CAdUiPathname::CAdUiPathname(const CAdUiPathname& pathSrc)
{
        AssignCopy( pathSrc );
}

inline path_type CAdUiPathname::GetPathType(void) const
{
        return m_this_type;
}

inline bool CAdUiPathname::IsPath(void) const
{
        return ((m_this_type == FILE_PATH)
                || (m_this_type == DIR_PATH)
                || (m_this_type == NEW_PATH) 
                || (m_this_type == WC_PATH) );
        
}        

inline bool CAdUiPathname::IsEmpty(void) const
{
        return (m_this_type == NO_PATH);
}        

inline const CAdUiVolumeDescriptor* CAdUiPathname::GetVolumeDescriptor(void) const
{
        return m_pvolume;
}

inline bool CAdUiPathname::HadPrefix(void) const
{
        return m_had_prefix;
}

inline const CAdUiPathname& CAdUiPathname::operator=(const CAdUiPathname& pathSrc)
{
        AssignCopy( pathSrc);

        return *this;
}

inline const CAdUiPathname& CAdUiPathname::operator=(const CAdUiPathname* pathSrc)
{
        AssignCopy( *pathSrc);

        return *this;
}


inline bool CAdUiPathname::WasRoot(void) const
{
    return (m_path_cat == ROOT_TYPE);
}

#pragma pack (pop)

