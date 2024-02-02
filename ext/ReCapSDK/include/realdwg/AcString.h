//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2020 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license
//  agreement provided at the time of installation or download, or which
//  otherwise accompanies this software in either electronic or hard copy form.
//
//  AcString.h
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Ac_String_h_
#define _Ac_String_h_

#include "acbasedefs.h"
#include "adesk.h"
#include "AcHeapOpers.h"
#include "AdAChar.h"
#include "rxresource.h"
#include <cstdarg>

class AcDbHandle;
class AcRxResourceInstance;

// Notes:
// 1. All wchar_t arguments are assumed to be "widechar" Unicode values.
// 2. The pointer returned from utf8Ptr() and kwszPtr() is valid until the next time
//    this AcString is modified.
// 3. Never cast away const from a pointer obtained by utf8Ptr() or kwszPtr() buffer, in order
//    to modify the buffer directly. These buffers may be shared by multiple AcString instances.
//    Instead, to operate on buffers directly, first call getBuffer() to obtain a pointer
// 4. Although utf8Ptr() is a const member function, it may reallocate the string's
//    internal buffer and thus invalidate pointers returned by a previous kwszPtr() call
//    utf8Ptr() is the only const method which can modify the AcString
// 5. kwszPtr(), constPtr(), kTCharPtr(), kACharPtr() and operator const wchar_t *()
//    are all equivalent.  They return a pointer to the null-terminated widechar string.
//    The redundancy is for historical reasons.
// 6. All index values (also known as position values) are 0-based.  For example, in
//    the string "abcd", the 'c' character has position 2
//

class AcString : public AcHeapOperators
{
public:
    /// <summary>Types of narrow char encoding supported.</summary>
    enum Encoding {
        /// <summary>Unicode utf-8 encoding.</summary>
        Utf8
    };

    //
    // Constructors and destructor
    //
    /// <summary>Default ctor, initializes to empty string.</summary>
    ACBASE_PORT AcString();

    /// <summary>Initialize with a single Unicode character</summary>
    /// <param name="wch">input character</param>
    ACBASE_PORT AcString(wchar_t wch);

    /// <summary>Initialize from a narrow char string.</summary>
    /// <param name="psz">Input narrow string. Null terminated.</param>
    /// <param name="encoding"> Input string's encoding format.</param>
    /// <remarks>Currently, only Utf8 encoding is supported.</remarks>
    ACBASE_PORT AcString(const char *psz, Encoding encoding);

    /// <summary>Initialize from a narrow char string.</summary>
    /// <param name="psz">Input narrow string. Null terminated.</param>
    /// <param name="encoding">Input string's encoding format.</param>
    /// <param name="nByteCount">Number of input bytes from psz to use.</param>
    /// <remarks>Currently, only Utf8 encoding is supported.
    /// <remarks> psz is not required to be null terminated.
    ///           If bytecount causes last multi-byte utf8 char in psz to be
    ///           truncated, then the behavior is undefined.
    /// </remarks>
    ACBASE_PORT AcString(const char *psz, Encoding encoding, unsigned int nByteCount);

    /// <summary>Initialize from a Unicode string</summary>
    /// <param name="wpsz">input pointer to zero terminated source string</param>
    ACBASE_PORT AcString(const wchar_t *pwsz);

    /// <summary>Initialize from a Unicode string</summary>
    /// <param name="wpsz">input pointer to source</param>
    /// <param name="count"> number of characters to use from the input string</param>
    ACBASE_PORT AcString(const wchar_t *pwsz, unsigned int count);

    /// <summary>Copy constructor</summary>
    /// <param name="acs">input reference to an existing AcString object</param>
    ACBASE_PORT AcString(const AcString & acs);

    /// <summary>Move constructor</summary>
    /// <param name="acs">input reference to an existing temp AcString object</param>
    ACBASE_PORT AcString(AcString && acs);

    /// <summary>Values for the nCtorFlags arg of the following constructor.</summary>
    enum eFormat {
        /// <summary>Format the arg as signed int</summary>
        kSigned = 0x0002,
        /// <summary>Format the arg as unsigned int</summary>
        kUnSigned = 0x0003,
        /// <summary>Format the arg as hexadecimal</summary>
        kHex = 0x0004
    };

    /// <summary>
    ///  Multi-purpose constructor, takes an unsigned argument and
    ///  uses it either to load a resource string or to create a
    ///  numerical string (base 10 or hex).
    /// </summary>
    /// <param name="nCtorFlags">input flags, indicating type of construction</param>
    /// <param name="nArg">input argument value, interpreted according to flags</param>
    ACBASE_PORT AcString(eFormat nCtorFlags, unsigned nArg);
    /// <summary>repeat a character n times.</summary>
    /// <param name="ch">character value</param>
    /// <param name="nRepeatTimes">repate times</param>
    ACBASE_PORT AcString(ACHAR ch, unsigned nRepeatTimes);
    /// <summary>Formats an AcDbHandle value in hex, as in: "a2f".</summary>
    /// <param name="h">input reference to an acdb handle value</param>
    ACBASE_PORT AcString(const AcDbHandle &h);
    /// <summary>Load String from resource instance.</summary>
    /// <param name="hDll">AxResourceInstance object to load string</param>
    /// <param name="nId">input id of the string resource in the specified resource dll</param>
    ACBASE_PORT AcString(const AcRxResourceInstance& hDll, unsigned int nId);

    /// <summary>Destructor: decrements use count and frees memory if the count
    //           goes to zero.</summary>
    ACBASE_PORT ~AcString();

    //
    // Querying methods
    //

    /// <summary>Get the string as utf-8.</summary>
    /// <returns>A pointer to a null-terminated utf8 string.</returns>
    /// <remarks> The pointer is only valid until this object is next modified.
    ///           Warning: this method can modify the object, even though it is const
    /// </remarks>
    ACBASE_PORT const char * utf8Ptr() const;

    /// <summary>Get the string as widechar unicode.</summary>
    /// <returns>A pointer to a null-terminated widechar string.</returns>
    /// <remarks> The pointer is only valid until this object is next modified.
    //            Note that ACHAR is currently always defined as wchar_t.</remarks>
    const wchar_t * kwszPtr() const;
    const wchar_t *  constPtr() const;
    const wchar_t * kTCharPtr() const;
    const ACHAR * kACharPtr() const;

    /// <summary>Operator for casting this string to a widechar unicode string pointer.</summary>
    /// <returns>A pointer to a null terminated widechar string.</returns>
    /// <remarks>Pointer is valid only until this AcString is next modified.</remarks>
    operator const wchar_t * () const;

    /// <summary>Test whether this string is null, i.e. has logical length zero.</summary>
    /// <returns>True if the string is empty, else false.</returns>
    bool isEmpty() const;

    /// <summary>Get logical length of this string.</summary>
    /// <returns>The number of characters in the string. Zero if it's empty.</returns>
    /// <remarks>Null terminator is not counted in logical length.</remarks>
    //
    ACBASE_PORT unsigned length() const;

    /// <summary>Get logical length of this string.</summary>
    /// <returns>The number of characters in the string. Zero if it's empty.</returns>
    /// <remarks>This method is dDeprecated. Please use length() instead.</remarks>
    unsigned tcharLength() const
    {
        return this->length();
    }

    /// <summary>Get the maximum logical length that this string can currently achieve
    //           without growing or reallocating its buffer.</summary>
    /// <returns>Number of characters the current buffer can hold.</returns>
    /// <remarks>Null terminator is not counted in logical length.</remarks>
    ACBASE_PORT unsigned capacity() const;

    /// <summary>Grows or (possibly) shrinks the buffer to match the requested capacity .</summary>
    /// <param name="nCapacity">Number of characters of space needed, including terminator.</param>
    /// <returns>True if the buffer was re-allocated, else false.</returns>
    /// <remarks>Shrink requests may be ignored, depending on current buffer size,
    ///          string length and refcount.</remarks>
    ACBASE_PORT bool reserve(unsigned nCapacity);

    /// <summary>Check if all characters are in the ascii range: 0x20..0x7f.</summary>
    /// <returns>True if all characters in the ASCII range, else false.</returns>
    /// <remarks>Codes 0x..0x1f are considered control characters and cause this method
    ///          to return false, for historical reasons.</remarks>
    ACBASE_PORT bool isAscii() const;

    /// <summary>Check if all characters are in the range 0x01 through 0x7f.</summary>
    /// <returns>True if all characters have their high bit (0x80) clear.</returns>
    /// <remarks>Codes in 0x01 through 0x7f tend to have the same meaning across
    ///          all encoding schemes (ansi code pages, utf-8, utf-16, etc.</remarks>
    ACBASE_PORT bool is7Bit() const;

    //
    // Parsing methods.
    //

    /// <summary>Flag values specifying how to handle errors such as
    ///          invalid characters or overflow during string parsing.
    ///
    enum {
        ///<summary>Return zero on errors.</summary>
        ///
        kParseZero = 0,

        ///<summary>Return -1 or ffff.</summary>
        ///
        kParseMinus1 = 0x01,

        ///<summary>Pop an assert in debug build.</summary>
        ///
        kParseAssert = 0x02,

        ///<summary>Throw an int exception.</summary>
        ///
        kParseExcept = 0x04,

        ///<summary>Treat empty string as error.</summary>
        ///
        kParseNoEmpty = 0x08,

        ///<summary>Default error handling behavior.</summary>
        ///
        kParseDefault = (kParseAssert | kParseZero)
    };

    /// <summary>Parse the current string as decimal, return a signed int.</summary>
    /// <param name="nFlags">input bits specifying how to do the parsing</param>
    /// <returns>The int value parsed from the string.</returns>
    ACBASE_PORT int asDeci(int nFlags = kParseDefault) const;

    /// <summary>Parse the current string as hexadecimal, return a signed int.</summary>
    /// <param name="nFlags">input bits specifying how to do the parsing</param>
    /// <returns>The int value parsed from the string.</returns>
    ACBASE_PORT int asHex(int nFlags = kParseDefault) const;

    /// <summary>Parse the current string as decimal, return an unsigned int.</summary>
    /// <param name="nFlags">input bits specifying how to do the parsing</param>
    /// <returns>The int value parsed from the string.</returns>
    ACBASE_PORT unsigned int asUDeci(int nFlags = kParseDefault) const;

    /// <summary>Parse the current string as hexadecimal, return an unsigned int.</summary>
    /// <param name="nFlags">input bits specifying how to do the parsing</param>
    /// <returns>The int value parsed from the string.</returns>
    ACBASE_PORT unsigned int asUHex(int nFlags = kParseDefault) const;

    /// <summary>Parse the current string as decimal, return a signed int64.</summary>
    /// <param name="nFlags">input bits specifying how to do the parsing</param>
    /// <returns>The int value parsed from the string.</returns>
    ACBASE_PORT int64_t asDeci64(int nFlags = kParseDefault) const;

    /// <summary>Parse the current string as hexadecimal, return a signed int64.</summary>
    /// <param name="nFlags">input bits specifying how to do the parsing</param>
    /// <returns>The int value parsed from the string.</returns>
    ACBASE_PORT int64_t asHex64(int nFlags = kParseDefault) const;

    /// <summary>Parse the current string as decimal, return an unsigned int64.</summary>
    /// <param name="nFlags">input bits specifying how to do the parsing</param>
    /// <returns>The int value parsed from the string.</returns>
    ACBASE_PORT Adesk::UInt64 asUDeci64(int nFlags = kParseDefault) const;

    /// <summary>Parse the current string as hexadecimal, return an unsigned int64.</summary>
    /// <param name="nFlags">input bits specifying how to do the parsing</param>
    /// <returns>The int value parsed from the string.</returns>
    ACBASE_PORT Adesk::UInt64 asUHex64(int nFlags = kParseDefault) const;

    /// <summary>Parse the current string as hexadecimal.  Return the handle..</summary>
    /// <param name="nFlags">input bits specifying how to do the parsing</param>
    /// <returns>The handle value parsed from the string.</returns>
    ACBASE_PORT AcDbHandle asAcDbHandle(int nFlags = kParseDefault) const;

    //
    // Find methods:
    //   Search for a character, a substring or for any of a group of characters,
    //   or for any character not in the group.
    //   Search for first or last occurrence.
    //

    /// <summary>Find a single character in the string..</summary>
    /// <param name="ch">input character to search for</param>
    /// <returns>The position of the found character. -1 if not found.</returns>
    /// <remarks>The first character position is zero.</remarks>
    int find(ACHAR ch) const;

    /// <summary> Find a single character in the string.  </summary>
    /// <param name="ch">input character to search for</param>
    /// <param name="nStartPos">first character position to look at</param>
    /// <returns> The position of the found character. -1 if not found.</returns>
    ACBASE_PORT int find(ACHAR ch, int nStartPos) const;

    /// <summary> Find a substring in the string </summary>
    /// <param name="psz">input string to search for</param>
    /// <param name="nStartPos">first character position in this string to look at</param>
    /// <returns> The position of the found substring. -1 if not found.</returns>
    ACBASE_PORT int find(const ACHAR *psz, int nStartPos = 0) const;

    /// <summary>Find an AcString in the string..</summary>
    /// <param name="acs">input string object to search for
    /// <returns>The position of the found string, -1 if not found.</returns>
    ACBASE_PORT int find(const AcString & acs) const;

    /// <summary> Find first character in this string which matches any in a group of
    ///           input characters</summary>
    /// <param name="psz">input group of characters to search for</param>
    /// <param name="nStartPos">first character position in this string to look at</param>
    /// <returns> The position of the found character. -1 if not found.</returns>
    /// <remarks> If psz is null, then we search for whitespace.</remarks>
    ACBASE_PORT int findOneOf(const ACHAR *psz, int nStartPos = 0) const;

    /// <summary> Find first character in this string which does not match any in a group
    ///           of input characters</summary>
    /// <param name="psz">input group of characters to search for</param>
    /// <param name="nStartPos">first character position to look at</param>
    /// <returns> The position of the "not found" character. -1 if all were found.</returns>
    /// <remarks> If psz is null, then we search for whitespace.</remarks>
    ACBASE_PORT int findNoneOf(const ACHAR *psz, int nStartPos = 0) const;

    /// <summary> Find last occurrence of a character in the string</summary>
    /// <param name="ch">input character to search for</param>
    /// <returns> The position of the found character. -1 if not found.</returns>
    /// <remarks> This method is DEPRECATED. Please use findLast() instead.</remarks>
    int findRev(ACHAR ch) const;

    /// <summary> Find last occurrence of a substring in the string</summary>
    /// <param name="psz">input substring to search for</param>
    /// <returns> The position of the found substring result. -1 if not found.</returns>
    /// <remarks> This method is DEPRECATED. Please use findLast() instead.</remarks>
    int findRev(const ACHAR *psz) const;

    /// <summary> Find last occurrence of a substring in the string</summary>
    /// <param name="acs">input substring to search for</param>
    /// <returns> The position of the found substring result. -1 if not found.</returns>
    /// <remarks> This method is DEPRECATED. Please use findLast() instead.</remarks>
    int findRev(const AcString & acs) const;

    /// <summary> Find last character in this string from a group of characters</summary>
    /// <param name="psz">input group of characters to match</param>
    /// <returns> The position of the found character. -1 if not found.</returns>
    /// <remarks> This method is DEPRECATED. Please use findLast() instead.</remarks>
    int findOneOfRev(const ACHAR *psz) const;

    /// <summary> Find last occurrence of a character in the string</summary>
    /// <param name="ch">input character to search for</param>
    /// <param name="nEndPos">Last character position to look at</param>
    /// <returns> The position of the found character. -1 if not found.</returns>
    int findLast(ACHAR ch, int nEndPos = -1) const;

    /// <summary> Find last occurrence of a substring in the string</summary>
    /// <param name="psz">input substring to search for</param>
    /// <param name="nEndPos">Last character position to look at</param>
    /// <returns> The position of the found substring result. -1 if not found.</returns>
    ACBASE_PORT int findLast(const ACHAR *psz, int nEndPos = -1) const;

    /// <summary> Find last character in this string from a group of characters</summary>
    /// <param name="psz">input group of characters to match</param>
    /// <param name="nEndPos">Last character position to look at</param>
    /// <returns> The position of the found character. -1 if not found.</returns>
    /// <remarks> If psz is null, then we search for whitespace.</remarks>
    ACBASE_PORT int findLastOneOf(const ACHAR *psz, int nEndPos = -1) const;

    /// <summary> Find last character in this string which does not match any in a group
    ///           of characters</summary>
    /// <param name="psz">input group of characters to search for</param>
    /// <param name="nEndPos">first character position to look at</param>
    /// <returns> The position of the "not found" character. -1 if all were found.</returns>
    /// <remarks> If psz is null, then we search for whitespace.</remarks>
    ACBASE_PORT int findLastNoneOf(const ACHAR *psz, int nEndPos = -1) const;

    //
    // Extraction methods
    // Note: mid() and substr() are the same thing - we define both
    //       for compatibility with CString and std::string
    //
    // The input index arguments are character indices into the string.

    /// <summary>Get substring from the a specified position to the string's end.</summary>
    /// <param name="nStart">The zero-based start position of the substring to get.</param>
    /// <returns>An AcString consisting of the specified substring</returns>
    AcString mid(int nStart) const;

    /// <summary>Get a substring from the string.  (same as substr() method).</summary>
    /// <param name="nStart">input index (in characters) from the start of the string</param>
    /// <param name="nNumChars">input number of characters to retrieve.
    //              If nNumChars is -1, then return the rest of the string.</param>
    /// <returns>An AcString consisting of the specified substring</returns>
    AcString mid(int nStart, int nNumChars) const;

    /// <summary>Get a substring from the start of string..</summary>
    /// <param name="nNumChars">input number of characters to retrieve.</param>
    //             if nNumChars is -1, then return the rest of the string
    /// <returns>An AcString consisting of the specified substring</returns>
    AcString substr(int numChars) const;

    /// <summary>Get a substring from the string.  (same as mid() method).</summary>
    /// <param name="nStart">input 0-based index from the start of the string</param>
    /// <param name="nNumChars">input number of characters to retrieve.</param>
    //             if nNumChars is -1, then return the rest of the string
    /// <returns>An AcString consisting of the specified substring</returns>
    ACBASE_PORT AcString substr(int nStart, int nNumChars) const;

    /// <summary>Get a substring from the end of string..</summary>
    /// <param name="nNumChars">input number of characters to retrieve.</param>
    /// <returns>An AcString consisting of the specified substring</returns>
    ACBASE_PORT AcString substrRev(int numChars) const;

    /// <summary>
    /// Return a nNumChars length substring from the start of string.
    /// </summary>
    /// <param name="nNumChars">The count of characters of the substring to get.</param>
    /// <returns>An AcString consisting of the specified substring</returns>
    AcString left(int nNumChars) const;

    /// <summary>
    /// Return a nNumChars length substring from the end of string.
    /// </summary>
    /// <param name="nNumChars">The count of characters of the substring to get.</param>
    /// <returns>An AcString consisting of the specified substring</returns>
    AcString right(int nNumChars) const;

    //
    // Assignment operators and methods
    //
    
    /// <summary>assign a Unicode character to the string.</summary>
    /// <param name="wch">input character to assign</param>
    /// <returns>A reference to this string object.</returns>
    AcString & assign(wchar_t wch);

    /// <summary>assign a string of narrow chars to the string.</summary>
    /// <param name="psz">input pointer to the string of narrow chars to assign</param>
    /// <param name="encoding"> input Encoding type</param>
    /// <returns>A reference to this string object.</returns>
    /// <remarks>Currently, only Utf8 encoding is supported.</remarks>
   ACBASE_PORT  AcString & assign(const char *psz, Encoding encoding);

    /// <summary>assign a string of Unicode characters to the string.</summary>
    /// <param name="pwsz">input pointer to the string of characters to assign</param>
    /// <returns>A reference to this string object.</returns>
    ACBASE_PORT AcString & assign(const wchar_t *pwsz);

    /// <summary>assign an AcString object to the string.</summary>
    /// <param name="acs">input reference to the AcString</param>
    /// <returns>A reference to this string object.</returns>
    ACBASE_PORT AcString & assign(const AcString & acs);

    /// <summary>assign an AcDbHandle object to the string (format it as hex).</summary>
    /// <param name="h">input reference to the AcDbHandle object</param>
    /// <returns>A reference to this string object.</returns>
    ACBASE_PORT AcString & assign(const AcDbHandle & h);

    /// <summary>assign a Unicode character to the string.</summary>
    /// <param name="wch">input character to assign</param>
    /// <returns>A reference to this string object.</returns>
    AcString & operator = (wchar_t wch);

    /// <summary>assign a string of Unicode characters to the string.</summary>
    /// <param name="pwsz">input pointer to the string of characters to assign</param>
    /// <returns>A reference to this string object.</returns>
    AcString & operator = (const wchar_t *pwsz);

    /// <summary>assign an AcString object to the string.</summary>
    /// <param name="acs">input reference to the AcString</param>
    /// <returns>A reference to this string object.</returns>
    AcString & operator = (const AcString & acs);

    /// <summary>move a temp AcString object to the string.</summary>
    /// <param name="acs">input reference to the temp AcString</param>
    /// <returns>A reference to this string object.</returns>
    ACBASE_PORT AcString & operator = (AcString && acs);

    /// <summary>assign an AcDbHandle object to the string (format it as hex).</summary>
    /// <param name="h">input reference to the AcDbHandle object</param>
    /// <returns>A reference to this string object.</returns>
    AcString & operator = (const AcDbHandle & h);

    /// <summary>Set the string to be empty..</summary>
    /// <returns>A reference to this string object.</returns>
    ACBASE_PORT AcString & setEmpty();

    /// <summary>Set the string from a resource string.</summary>
    /// <param name="hDll">AxResourceInstance object to load string</param>
    /// <param name="nId">input id of the string resource in the specified resource dll</param>
    /// <returns>A reference to this string object.</returns>
    ACBASE_PORT bool loadString(const AcRxResourceInstance& hDll, unsigned nId);

    /// <summary>Format the string using "printf" rules..</summary>
    /// <param name="pszFmt">input pointer to the printf format string</param>
    /// <returns>A reference to this string object.</returns>
    ACBASE_PORT AcString & format (const ACHAR    *pszFmt,  ...);

    /// <summary>Format the string using "printf" rules.</summary>
    /// <param name="pszFmt">input pointer to the printf format string</param>
    /// <param name="args">input variable args list, containing values to be formatted</param>
    /// <returns>A reference to this string object.</returns>
    ACBASE_PORT AcString & formatV(const ACHAR   *pszFmt,  va_list args);

    /// <summary>
    /// Append formated data to this string using "printf" rules
    /// </summary>
    /// <param name="pszFmt">input pointer to the printf format string</param>
    /// <param name="args">input variable args list, containing values to be formatted</param>
    /// <returns> Reference to this AcString.</returns>
    ACBASE_PORT AcString & appendFormat(const ACHAR   *pszFmt, ...);

    //
    // Modifying operators and methods
    //

    /// <summary>append a Unicode character to the end of the string.</summary>
    /// <param name="wch">input character to append</param>
    /// <returns>A reference to this string object.</returns>
    AcString & operator += (wchar_t wch);

    /// <summary>append a Unicode string to the end of the string.</summary>
    /// <param name="pwsz">input pointer to the Unicode string</param>
    /// <returns>A reference to this string object.</returns>
    AcString & operator += (const wchar_t * pwsz);

    /// <summary>append an AcString object to the end of the string.</summary>
    /// <param name="acs">input reference to the AcString</param>
    /// <returns>A reference to this string object.</returns>
    AcString & operator += (const AcString & acs);

    /// <summary>append a Unicode character to the end of the string.</summary>
    /// <param name="wch">input character to append</param>
    /// <returns>A reference to this string object.</returns>
    AcString & append(wchar_t wch);

    /// <summary>append a char string to the end of the string.</summary>
    /// <param name="psz">input pointer to the narrow char string</param>
    /// <param name="encoding">input Encoding type</param>
    /// <returns>A reference to this string object.</returns>
    /// <remarks>Currently, only Utf8 encoding is supported.</remarks>
    ACBASE_PORT AcString & append(const char *psz, Encoding encoding);

    /// <summary>append a Unicode string to the end of the string.</summary>
    /// <param name="pwsz">input pointer to the Unicode string</param>
    /// <returns>A reference to this string object.</returns>
    ACBASE_PORT AcString & append(const wchar_t *pwsz);

    /// <summary>append an AcString object to the end of the string.</summary>
    /// <param name="acs">input reference to the AcString</param>
    /// <returns>A reference to this string object.</returns>
    ACBASE_PORT AcString & append(const AcString & acs);

    // Catenation operators and methods  These are like append,
    // but they do not modify the current string.  They return a
    // new combined string.

    /// <summary>Copy the string and append a Unicode character to it.</summary>
    /// <param name="ch">input character to append to the string copy</param>
    /// <returns>A reference to this string object.</returns>
    AcString operator + (wchar_t wch) const;

    /// <summary>Copy the string and append a string of Unicode characters to it.</summary>
    /// <param name="pwsz">input pointer to the string to append</param>
    /// <returns>A reference to this string object.</returns>
    AcString operator + (const wchar_t * pwsz) const;

    /// <summary>Copy the string and append an AcString to it.</summary>
    /// <param name="pwsz">input reference to the AcString to append</param>
    /// <returns>A reference to this string object.</returns>
    AcString operator + (const AcString & acs) const;

    /// <summary>Copy the string and append a Unicode character to it.</summary>
    /// <param name="ch">input character to append to the string copy</param>
    /// <returns>A reference to this string object.</returns>
    AcString concat(wchar_t wch) const;

    /// <summary>Copy the string and append a string of narrow chars to it.</summary>
    /// <param name="psz">input pointer to the narrow string to append</param>
    /// <param name="encoding">input Encoding type</param>
    /// <returns>A reference to this string object.</returns>
    /// <remarks>Currently, only Utf8 encoding is supported.</remarks>
    ACBASE_PORT AcString concat(const char * psz, Encoding encoding) const;

    /// <summary>Copy the string and append a string of Unicode characters to it.</summary>
    /// <param name="pwsz">input pointer to the string to append</param>
    /// <returns>A reference to this string object.</returns>
    ACBASE_PORT AcString concat(const wchar_t * pwsz) const;

    /// <summary>Copy the string and append an AcString to it.</summary>
    /// <param name="pwsz">input reference to the AcString to append</param>
    /// <returns>A reference to this string object.</returns>
    ACBASE_PORT AcString concat(const AcString & acs) const;

    // These copy the current string and then insert the character or
    // string in front of it.  They're used by the global "+" operators.

    /// <summary>Copy the string and insert a character in front of it.</summary>
    /// <param name="ch">input character to insert</param>
    /// <returns>A reference to this string object.</returns>
    AcString precat(ACHAR ch) const;

    /// <summary>Copy the string and insert a string of narrow chars in front of it.</summary>
    /// <param name="psz">input pointer to the string of narrow chars to insert</param>
    /// <param name="encoding">input Encoding type</param>
    /// <returns>A reference to this string object.</returns>
    /// <remarks>Currently, only Utf8 encoding is supported.</remarks>
    ACBASE_PORT AcString precat(const char * psz, Encoding encoding) const;

    /// <summary>Copy the string and insert a string of characters in front of it.</summary>
    /// <param name="psz">input pointer to the string of characters to insert</param>
    /// <returns>A reference to this string object.</returns>
    ACBASE_PORT AcString precat(const wchar_t * psz) const;

    //
    // Comparison operators and methods
    // The int return value is -1, 0 or 1, indicating <, == or >
    //

    /// <summary>Compare the string to a single Unicode char.</summary>
    /// <param name="wch">input character to compare to</param>
    /// <returns>0 if this string equals wch, a value < 0 if this string is less
    ///          than wch and a value > 0 if this string is greater than wch.</returns>
    int  compare(wchar_t wch) const;

    /// <summary>Compare the string to a string of narrow chars.</summary>
    /// <param name="psz">input pointer to the string of narrow chars to compare to</param>
    /// <param name="encoding">input Encoding type</param>
    /// <returns>0 if this string equals psz, a value < 0 if this string is less
    ///          than psz and a value > 0 if this string is greater than psz.</returns>
    /// <remarks>Currently, only Utf8 encoding is supported.</remarks>
    ACBASE_PORT int  compare(const char *psz, Encoding encoding) const;

    /// <summary>Compare the string to a string of Unicode characters.</summary>
    /// <param name="pwsz">input pointer to the string of characters to compare to</param>
    /// <returns>0 if this string equals pwsz, a value < 0 if this string is less
    ///          than pwsz and a value > 0 if this string is greater than pwsz.</returns>
    ACBASE_PORT int  compare(const wchar_t *pwsz) const;

    /// <summary>Compare the string to a string of Unicode characters.</summary>
    /// <param name="acs">input reference of the other AcString to compare to</param>
    /// <returns>0 if this string equals acs, a value < 0 if this string is less
    ///          than acs and a value > 0 if this string is greater than acs.</returns>
    int  compare(const AcString & acs) const;


    /// <summary>Compare the string to another string using collation.</summary>
    /// <param name="pwsz">input pointer to the string of characters to compare to</param>
    /// <returns>0 if this string equals pwsz, a value < 0 if this string is less
    ///          than pwsz and a value > 0 if this string is greater than pwsz.</returns>
    ACBASE_PORT int  collate (const wchar_t *pwsz) const;
    
    /// <summary>Compare the string to another AcString object using collation.</summary>
    /// <param name="acs">input AcString object to compare to </param>
    /// <returns>0 if this string equals acs, a value < 0 if this string is less
    ///          than acs and a value > 0 if this string is greater than acs.</returns>
    int  collate(const AcString & acs) const;

    /// <summary>Compare the string case-independently to a Unicode char.</summary>
    /// <param name="wch">input character to compare to</param>
    /// <returns>0 if this string equals wch, a value < 0 if this string is less
    ///          than wch and a value > 0 if this string is greater than wch.</returns>
    int  compareNoCase(wchar_t wch) const;

    /// <summary>Compare the string case-independently to a string of narrow chars.</summary>
    /// <param name="psz">input pointer to the string of narrow chars to compare to</param>
    /// <param name="encoding">input Encoding type</param>
    /// <returns>0 if this string equals psz, a value < 0 if this string is less
    ///          than psz and a value > 0 if this string is greater than psz.</returns>
    /// <remarks>Currently, only Utf8 encoding is supported.</remarks>
    ACBASE_PORT int  compareNoCase(const char *psz, Encoding encoding) const;

    /// <summary>Compare the string case-independently to a string of Unicode characters.</summary>
    /// <param name="pwsz">input pointer to the string of characters to compare to</param>
    /// <returns>0 if this string equals pwsz, a value < 0 if this string is less
    ///          than pwsz and a value > 0 if this string is greater than pwsz.</returns>
    ACBASE_PORT int  compareNoCase(const wchar_t *pwsz) const;

    /// <summary>Compare the string case-independently to another AcString.</summary>
    /// <param name="acs">input reference to the other AcString</param>
    /// <returns>0 if this string equals acs, a value < 0 if this string is less
    ///          than acs and a value > 0 if this string is greater than acs.</returns>
    int  compareNoCase(const AcString & acs) const;

    /// <summary>Compares two AcStrings for equality, ignoring case.</summary>
    /// <param name="left"> one AcString object </param>
    /// <param name="right"> another AcString object </param>
    /// <returns>true if left equals right, else false.</returns>
    /// <remarks>May be useful as a comparator function for STL functions.</remarks>
    static bool equalsNoCase(const AcString& left, const AcString& right);

    /// <summary>Compare case-independently to another string using collation.</summary>
    /// <param name="psz">input pointer to the string of characters to compare to </param>
    /// <returns>0 if this string equals psz, a value < 0 if this string is less
    ///          than psz and a value > 0 if this string is greater than psz.</returns>
    ACBASE_PORT int collateNoCase(const wchar_t *psz) const;

    /// <summary>Compare case-independently to another AcString using collation./// </summary>
    /// <param name="acs"> input reference to the other AcString to compare to </param>
    /// <returns>0 if this string equals acs, a value < 0 if this string is less
    ///          than acs and a value > 0 if this string is greater than acs.</returns>
    int collateNoCase(const AcString& acs) const;

    /// <summary>Compare this string for equality with a wide char.</summary>
    /// <param name="wch">input character to compare to</param>
    /// <returns>True if this string equals wch, else false.</returns>
    bool operator == (wchar_t wch) const;

    /// <summary>Compare the string for equality with a string of wide characters.</summary>
    /// <param name="pwsz">input pointer to the string of characters</param>
    /// <returns>True if this string equals pwsz, else false.</returns>
    bool operator == (const wchar_t *pwsz) const;

    /// <summary>Compare the string for equality with another AcString.</summary>
    /// <param name="acs">input reference to the other AcString</param>
    /// <returns>True if this string equals acs, else false.</returns>
    bool operator == (const AcString & acs) const;

    /// <summary>Compare the string for non-equality with a wide char.</summary>
    /// <param name="wch">input character to compare to</param>
    /// <returns>True if this string does not equal wch, false if they are equal.</returns>
    bool operator != (wchar_t wch) const;

    /// <summary>Compare the string for non-equality with a string of wide characters.</summary>
    /// <param name="pwsz">input pointer to the string of characters</param>
    /// <returns>True if this string does not equal pwsz, false if they are equal.</returns>
    bool operator != (const wchar_t *pwsz) const;

    /// <summary>Compare the string for non-equality with another AcString.</summary>
    /// <param name="acs">input reference to the other AcString</param>
    /// <returns>True if this string does not equal acs, false if they are equal.</returns>
    bool operator != (const AcString & acs) const;

    /// <summary>Compare the string for greater than a wide char.</summary>
    /// <param name="wch">input character to compare to</param>
    /// <returns>True if this string is greater than wch, false otherwise.</returns>
    bool operator >  (wchar_t wch) const;

    /// <summary>Compare the string for greater than a string of wide characters.</summary>
    /// <param name="pwsz">input pointer to the string of characters to compare to</param>
    /// <returns>True if this string is greater than pwsz, false otherwise.</returns>
    bool operator >  (const wchar_t *pwsz) const;

    /// <summary>Compare the string for greater than another AcString.</summary>
    /// <param name="acs">input reference to the other AcString</param>
    /// <returns>True if this string is greater than acs, false otherwise.</returns>
    bool operator >  (const AcString & acs) const;

    /// <summary>Compare the string for greater than or equal to a wide char.</summary>
    /// <param name="wch">input character to compare to</param>
    /// <returns>True if this string is greater than or equal to wch, false otherwise.</returns>
    bool operator >= (wchar_t wch) const;

    /// <summary>Compare for greater than/equal to a string of wide characters.</summary>
    /// <param name="pwsz">input pointer to the string of characters</param>
    /// <returns>True if this string is greater than or equal to pwsz, false otherwise.</returns>
    bool operator >= (const wchar_t *pwsz) const;

    /// <summary>Compare the string for greater than or equal to another AcString.</summary>
    /// <param name="acs">input reference to the other AcString</param>
    /// <returns>True if this string is greater than or equal to acs, false otherwise.</returns>
    bool operator >= (const AcString & acs) const;

    /// <summary>Compare the string for less than a wide char.</summary>
    /// <param name="wch">input character to compare to</param>
    /// <returns>True if this string is less than wch, false otherwise.</returns>
    bool operator <  (wchar_t wch) const;

    /// <summary>Compare the string for less than a string of wide characters.</summary>
    /// <param name="pwsz">input pointer to the string of characters to compare to</param>
    /// <returns>True if this string is less than pwsz, false otherwise.</returns>
    bool operator <  (const wchar_t *pwsz) const;

    /// <summary>Compare the string for less than another AcString.</summary>
    /// <param name="acs">input reference to the other AcString</param>
    /// <returns>True if this string is less than acs, false otherwise.</returns>
    bool operator <  (const AcString & acs) const;

    /// <summary>Compare the string for less than or equal to a wide char.</summary>
    /// <param name="wch">input character to compare to</param>
    /// <returns>True if this string is less than or equal to wch, false otherwise.</returns>
    bool operator <= (wchar_t wch) const;

    /// <summary>Compare the string for less than/equal to a string of wide characters.</summary>
    /// <param name="pwsz">input pointer to the string of characters</param>
    /// <returns>True if this string is less than or equal to pwsz, false otherwise.</returns>
    bool operator <= (const wchar_t *pwsz) const;

    /// <summary>Compare the string for less or equal to than another AcString.</summary>
    /// <param name="acs">input reference to the other AcString</param>
    /// <returns>True if this string is less than or equal to acs, false otherwise.</returns>
    bool operator <= (const AcString & acs) const;

    // The match() methods return the number of positions in the string that have the same
    // character value as that position in the other string.  E.g., matching "abcd" and
    // "abxyz" returns 2.  Matching "abc" and "xyz" returns 0.

    /// <summary>See how many characters match a string of narrow chars.</summary>
    /// <param name="psz">input pointer to the string of narrow chars</param>
    /// <param name="encoding">input Encoding type</param>
    /// <returns>The number of characters that match psz.</returns>
    /// <remarks>Currently, only Utf8 encoding is supported.</remarks>
    ACBASE_PORT int  match(const char *psz, Encoding encoding) const;

    /// <summary>See how many characters match a string of wide characters.</summary>
    /// <param name="pwsz">input pointer to the string of characters</param>
    /// <returns>The number of characters that match pwsz.</returns>
    ACBASE_PORT int  match(const wchar_t *pwsz) const;

    /// <summary>See how many characters match another AcString.</summary>
    /// <param name="acs">input reference to the other AcString</param>
    /// <returns>The number of characters that match pwsz.</returns>
    ACBASE_PORT int  match(const AcString & acs) const;

    /// <summary>See how many characterss case-independently match a narrow string.</summary>
    /// <param name="psz">input pointer to the string of narrow chars</param>
    /// <param name="encoding">input Encoding type</param>
    /// <returns>The number of characters that match psz.</returns>
    /// <remarks>Currently, only Utf8 encoding is supported.</remarks>
    ACBASE_PORT int  matchNoCase(const char *psz, Encoding encoding) const;

    /// <summary>See how many characters case-independently match a wide string.</summary>
    /// <param name="pwsz">input pointer to the string of characters</param>
    /// <returns>The number of characters that match pwsz.</returns>
    ACBASE_PORT int  matchNoCase(const wchar_t *pwsz) const;

    /// <summary>See how many characters case-independently match another AcString.</summary>
    /// <param name="acs">input reference to the other AcString</param>
    /// <returns>The number of characters that match acs.</returns>
    ACBASE_PORT int  matchNoCase(const AcString & acs) const;

    /// <summary>Convert this string's lowercase characters to upper case.</summary>
    /// <returns> Reference to this AcString.</returns>
    ACBASE_PORT AcString & makeUpper();
    
    /// <summary>Convert this string's uppercase characters to lower case.</summary>
    /// <returns> Reference to this AcString.</returns>
    ACBASE_PORT AcString & makeLower();
    
    /// <summary>Reverse the characters in this string./// </summary>
    /// <returns>A reference to this AcString.</returns>
    ACBASE_PORT AcString& makeReverse();

    /// <summary> Remove all occurrences of a character from front of this string.</summary>
    /// <returns> Reference to this AcString.</returns>
    /// <remarks> No-op if wch arg is null character.</remarks>
    ///
    ACBASE_PORT AcString & trimLeft(wchar_t wch);

    /// <summary> Remove all occurrences of a character from end of this string.</summary>
    /// <returns> Reference to this AcString.</returns>
    /// <remarks> No-op if wch arg is null character.</remarks>
    ///
    ACBASE_PORT AcString & trimRight(wchar_t wch);

    /// <summary> Remove all occurrences of a character from both ends of this string.</summary>
    /// <returns> Reference to this AcString.</returns>
    /// <remarks> No-op if wch arg is null character.</remarks>
    ///
    ACBASE_PORT AcString & trim(wchar_t wch);

    /// <summary> Remove all whitespace characters from beginning of the string.</summary>
    /// <returns> Reference to this AcString.</returns>
    ///
    AcString & trimLeft();

    /// <summary> Remove all designated characters from beginning of the string.</summary>
    /// <returns> Reference to this AcString.</returns>
    /// <remarks> Trims whitespace if pwszChars arg is null.</remarks>
    ///
    ACBASE_PORT AcString & trimLeft(const wchar_t *pwszChars);

    /// <summary> Remove all whitespace characters from the end of the string.</summary>
    /// <returns> Reference to this AcString.</returns>
    ///
    AcString & trimRight();

    /// <summary> Remove all designated characters from the end of the string.</summary>
    /// <returns> Reference to this AcString.</returns>
    /// <remarks> Trims whitespace if pwszChars arg is null.</remarks>
    ///
    ACBASE_PORT AcString & trimRight(const wchar_t *pwszChars);

    /// <summary> Remove all whitespace characters from both ends of the string.</summary>
    /// <returns> Reference to this AcString.</returns>
    ///
    AcString & trim();

    /// <summary> Remove all designated characters from both ends of the string.</summary>
    /// <returns> Reference to this AcString.</returns>
    /// <remarks> Trims whitespace if pwszChars arg is null.</remarks>
    ///
    ACBASE_PORT AcString & trim(const wchar_t *pwszChars);

    /// <summary> Remove all occurrences of the specified character.</summary>
    /// <returns> Number of characters removed. Zero if the string was not changed.</returns>
    /// <remarks> Removes whitespace characters if wch arg is zero.</remarks>
    ///
    ACBASE_PORT int remove(wchar_t wch);

    /// <summary> Remove all occurrences of whitespace.</summary>
    /// <returns> Number of characters removed. Zero if the string was not changed.</returns>
    ///
    int remove()
    {
        return this->remove(0);
    }

    /// <summary> Extract substring up to the first instance of a designated character.</summary>
    /// <returns> AcString that contains the substring</returns>
    ///
    ACBASE_PORT AcString spanExcluding(const wchar_t *pwszChars) const;

#if  defined(_AFX) || defined (__OSX_WINAPI_UNIX_STRING_H__) || defined(__ATLSTR_H__)
    //
    // MFC CString-using methods.  The CStringA class is the ansi
    // code page based CString, while CStringW is Unicode based.
    // CString maps to one or the other depending on whether the
    // UNICODE preprocessor symbol is defined.
    //

    /// <summary>Construct an AcString from a CStringW.</summary>
    /// <param name="csw">input reference to the CStringW</param>
    AcString(const CStringW &csw);

    /// <summary>Initialize this AcString from a CStringW.</summary>
    /// <param name="csw">input reference to the CStringW</param>
    /// <returns>A reference to this AcString object.</returns>
    AcString & operator = (const CStringW &csw);

    /// <summary>Append a CStringW to this AcString.</summary>
    /// <param name="csa">input reference to the CStringW</param>
    /// <returns>A reference to this AcString object.</returns>
    AcString & operator += (const CStringW &csw);

    /// <summary>Compare this string to a CStringW.</summary>
    /// <param name="csw">input reference to the CStringW</param>
    /// <returns>0 if this string equal csw, a value < 0 if tihs string is
    ///          is less than csw, a value > 0 if this string is greater than csw.</returns>
    int  compare(const CStringW & csw) const;

    /// <summary>Compare this string case independently to a CStringW.</summary>
    /// <param name="csw">input reference to the CStringW</param>
    /// <returns>0 if this string equal csw, a value < 0 if tihs string is
    ///          is less than csw, a value > 0 if this string is greater than csw.</returns>
    int  compareNoCase(const CStringW & csw) const;

    /// <summary>Compare for equality with a CStringW.</summary>
    /// <param name="csw">input reference to the CStringW</param>
    /// <returns>True if this string equals the CStringW, else false.</returns>
    bool operator == (const CStringW & ) const;

    /// <summary>Compare for non-equality with a CStringW.</summary>
    /// <param name="csw">input reference to the CStringW</param>
    /// <returns>True if this string does not equal the CStringW, false if they're equal.</returns>
    bool operator != (const CStringW & ) const;

    /// <summary>Compare for less than a CStringW.</summary>
    /// <param name="csw">input reference to the CStringW</param>
    /// <returns>True if this string is less than the CStringW, else false.</returns>
    bool operator <  (const CStringW & ) const;

    /// <summary>Compare for less than or equal to a CStringW.</summary>
    /// <param name="csw">input reference to the CStringW</param>
    /// <returns>True if this string is less than or equal to the CStringW, else false.</returns>
    bool operator <= (const CStringW & ) const;

    /// <summary>Compare for greater than a CStringW.</summary>
    /// <param name="csw">input reference to the CStringW</param>
    /// <returns>True if this string is greater than the CStringW, else false.</returns>
    bool operator >  (const CStringW & ) const;

    /// <summary>Compare for greater than or equal to a CStringW.</summary>
    /// <param name="csw">input reference to the CStringW</param>
    /// <returns>True if this string is greater than or equal to the CStringW, else false.</returns>
    bool operator >= (const CStringW & ) const;

    /// <summary>Get number of characters matching a CStringW.</summary>
    /// <param name="csw">input reference to the CStringW</param>
    /// <returns>The number of characters in this string matching csw.</returns>
    int  match(const CStringW & csw) const;

    /// <summary>Return number of characters case-independently matching a CStringW.</summary>
    /// <param name="csw">input reference to the CStringW</param>
    /// <returns>The number of characters in this string matching csw.</returns>
    int  matchNoCase(const CStringW & csw) const;

#endif

    /// <summary>
    /// Rplaces instances of the substring with instances of the new string 
    /// </summary>
    /// <param name="pwszOld"> A pointer to a string containing the character to be replaced by lpszNew. </param>
    /// <param name="pwszNew"> A pointer to a string containing the character replacing lpszOld. </param>
    /// <returns>The number of replaced instances of the substring.
    ///          Zero if the string is not changed.</returns>
    ACBASE_PORT  int replace(const wchar_t* pwszOld, const wchar_t* pwszNew);

    /// <summary>
    /// Replace a character with another.
    /// </summary>
    /// <param name="wchOld"> character that will be replaced </param>
    /// <param name="wchNew"> new character that will be replaced with </param>
    /// <returns>The number of replaced instances of the wchOld.
    ///          Zero if the string is not changed.</returns>
    ACBASE_PORT int replace(wchar_t wchOld, wchar_t wchNew);

    /// <summary>
    /// Deletes character(s) from a string starting with the character at given index.
    /// </summary>
    /// <param name="iIndex"> start position to delete </param>
    /// <param name="nCount"> character number to be deleted </param>
    /// <returns>Return the length of the changed string.</returns>
    ACBASE_PORT int deleteAtIndex(int iIndex, int nCount = 1);

    /// <summary>
    /// Finds the next token in a target string
    /// </summary>
    /// <param name="pszTokens">A string containing token delimiters. The order of these delimiters is not important.</param>
    /// <param name="iStart">The zero-based index to begin the search.</param>
    /// <returns>An AcString containing the current token value.</returns>
    ACBASE_PORT AcString tokenize(const wchar_t* pszTokens, int& iStart) const;


    /// <summary>
    /// Set the character at the given postion to the specified character.
    /// </summary>
    /// <param name="nIndex">Zero-based postion of character in the string.</param>
    /// <param name="ch">The new character to replace the old one.</param>
    /// <returns>A reference to this AcString object.</returns>
    ACBASE_PORT AcString& setAt(int nIndex, ACHAR ch);

    /// <summary>
    /// Get one character at the given postion from the string.
    /// </summary>
    /// <param name="nIndex">Zero-based postion of character in the string.</param>
    /// <returns> Return the character at the specified position in the string </returns>
    /// <remarks> Does NOT do range checking on the nIndex arg.
    ///           Results for out of range nIndex args are unpredictable.
    ///           Indexing via [] may also work, causing an implicit call to
    ///           the const wchar_t * operator
    /// </remarks>
    wchar_t getAt(int nIndex) const;

    /// <summary>
    /// Inserts a single character at the given index within the string.
    /// </summary>
    /// <param name="nIndex">The index of the character before which the insertion will take place.</param>
    /// <param name="ch">The character to be inserted.</param>
    /// <returns>A reference to this AcString object.</returns>
    ACBASE_PORT AcString& insert(int nIndex, wchar_t ch);

    /// <summary>
    /// Inserts a substring at the given index within the string.
    /// </summary>
    /// <param name="nIndex">The index of the character before which the insertion will take place.</param>
    /// <param name="ch">A pointer to the substring to be inserted.</param>
    /// <returns>A reference to this AcString object.</returns>
    ACBASE_PORT AcString& insert(int nIndex, const wchar_t* pwsz);

    /// <summary>
    /// Returns a pointer to the internal character buffer of the string object, allowing
    /// direct access to and modification of the string contents.
    /// The returned buffer contains the string contents and is null terminated.
    /// The buffer is at least large enough to hold nMinBufferLength characters plus
    /// a null terminator.  Buffer memory after the terminator may be uninitialized.
    /// Clients should call releaseBuffer() when they're done accessing the buffer, and they
    /// should not call any other methods (except an implicit call to the dtor) before then.
    /// </summary>
    /// <param name="nMinBufferLength">Number of characters that should fit in the buffer,
    /// not including the null terminator.
    /// </param>
    /// <returns>
    /// wchar_t pointer to the AcString's (null-terminated) character buffer.
    /// The call fails and returns nullptr if nMinBufferLength is < 0.
    /// </returns>
    ACBASE_PORT ACHAR* getBuffer(int nMinBufferLength = 0);

    /// <summary>
    /// Use releaseBuffer() to end the use of a buffer allocated by the getBuffer() method.
    /// The pointer returned by getBuffer() is invalid after the call to releaseBuffer().
    /// </summary>
    /// <param name="nMinBufferLength">Sets the new length of the AcString.
    /// If -1, then the string's length is determined by the null terminator's index.
    /// Otherwise the new length is set to the minimum of nMinBufferLength and the null
    /// terminator's index.
    /// </param>
    /// <returns>True if success, false on errors such as invalid length args or
    /// no previous call to getBuffer().</returns>
    ACBASE_PORT bool   releaseBuffer(int nNewLength = -1);

private:

    friend class AcStringImp;
    wchar_t *m_wsz;
};


#ifdef AC_ACARRAY_H
typedef
AcArray< AcString, AcArrayObjectCopyReallocator< AcString > > AcStringArray;
#endif

//
// Global operator declarations
//

/// <summary>Compare an AcString and a Unicode character for equality.</summary>
/// <param name="wch">input character to compare</param>
/// <param name="acs">input reference to the AcString</param>
/// <returns>True if wch and acs are equal, else false.</returns>
bool operator == (wchar_t wch, const AcString & acs);

/// <summary>Compare an AcString and a string of Unicode characters for equality.</summary>
/// <param name="pwsz">input character to the string of Unicode characters</param>
/// <param name="acs">input reference to the AcString</param>
/// <returns>True if pwsz and acs are equal, else false.</returns>
bool operator == (const wchar_t *pwsz, const AcString & acs);

/// <summary>Compare an AcString and a Unicode character for non-equality.</summary>
/// <param name="wch">input character to compare</param>
/// <param name="acs">input reference to the AcString</param>
/// <returns>True if wch and acs are not equal, else false.</returns>
bool operator != (wchar_t wch, const AcString & acs);

/// <summary>Compare an AcString and a string of Unicode characters for non-equality.</summary>
/// <param name="pwsz">input ptr to the string of Unicode characters</param>
/// <param name="acs">input reference to the AcString</param>
/// <returns>True if pwsz and acs are not equal, else false.</returns>
bool operator != (const wchar_t *pwsz, const AcString & acs);

/// <summary>Return whether a Unicode character is greater than an AcString.</summary>
/// <param name="wch">input character to compare</param>
/// <param name="acs">input reference to the AcString</param>
/// <returns>True if wch is greater than acs, else false.</returns>
bool operator >  (wchar_t wch, const AcString & acs);

/// <summary>Return whether a string of Unicode characters is greater than an AcString.</summary>
/// <param name="pwsz">input pointer to the string of Unicode characters</param>
/// <param name="acs">input reference to the AcString</param>
/// <returns>True if pwsz is greater than acs, else false.</returns>
bool operator >  (const wchar_t *pwsz, const AcString & acs);

/// <summary>Check for a Unicode character being greater than or equal to an AcString.</summary>
/// <param name="wch">input character to compare</param>
/// <param name="acs">input reference to the AcString</param>
/// <returns>True if wch is greater than or equal to acs, else false.</returns>
bool operator >= (wchar_t wch, const AcString & acs);

/// <summary>Check for a string of Unicode characters being greater than/equal to an AcString.</summary>
/// <param name="pwsz">input string to compare</param>
/// <param name="acs">input reference to the AcString</param>
/// <returns>True if pwsz is greater than or equal to acs, else false.</returns>
bool operator >= (const wchar_t *pwsz, const AcString & acs);

/// <summary>Check for a Unicode character being less than an AcString.</summary>
/// <param name="wch">input character to compare</param>
/// <param name="acs">input reference to the AcString</param>
/// <returns>True if wch is less than acs, else false.</returns>
bool operator <  (wchar_t wch, const AcString & acs);

/// <summary>Check for a string of Unicode characters being less than an AcString.</summary>
/// <param name="pwsz">input character to compare</param>
/// <param name="acs">input reference to the AcString</param>
/// <returns>True if pwsz is less than acs, else false.</returns>
bool operator <  (const wchar_t *pwsz, const AcString & acs);

/// <summary>Check for a Unicode character being less than or equal to an AcString.</summary>
/// <param name="wch">input character to compare</param>
/// <param name="acs">input reference to the AcString</param>
/// <returns>True if wch is less than or equal to acs, else false.</returns>
bool operator <= (wchar_t wch, const AcString & acs);


/// <summary>Check for a string of Unicode characters being less than/equal to an AcString.</summary>
/// <param name="pwsz">input characters to compare</param>
/// <param name="acs">input reference to the AcString</param>
/// <returns>True if pwsz is less than or equal to acs, else false.</returns>
bool operator <= (const wchar_t *pwsz, const AcString & acs);

/// <summary>Copy an AcString and insert a Unicode characters in front of it.</summary>
/// <param name="wch">input characters to insert</param>
/// <returns>An AcString consisting of the concatenation of wch and acs.</returns>
AcString operator + (wchar_t wch, const AcString & acs);

/// <summary>Copy an AcString and insert a string of Unicode characters in front of it.</summary>
/// <param name="pwsz">input pointer to the string of characters to insert</param>
/// <returns>An AcString consisting of the concatenation of pwsz and acs.</returns>
AcString operator + (const wchar_t *pwsz, const AcString & acs);

// Accessing inlines
//

inline AcString::operator const wchar_t *() const
{
    return this->kwszPtr();
}

inline const wchar_t * AcString::constPtr() const
{
    return this->kwszPtr();
}

inline const wchar_t * AcString::kTCharPtr() const
{
    return this->kwszPtr();
}

inline const ACHAR * AcString::kACharPtr() const
{
    return this->kwszPtr();
}

inline bool AcString::isEmpty() const
{
    return this->m_wsz[0] == L'\0';
}

inline wchar_t AcString::getAt(int nIndex) const
{
    return this->m_wsz[nIndex];
}

// Searching inlines
//
inline int AcString::find(ACHAR ch) const
{
    const ACHAR str[2] = {ch, '\0'};
    return this->findOneOf(str);
}

inline int AcString::find(wchar_t wch, int nStartPos) const
{
    const wchar_t wsz[2] = {wch, 0};
    return this->findOneOf(wsz, nStartPos);
}

inline int AcString::find(const AcString &s) const
{
    return this->find(s.kwszPtr());
}

inline int AcString::findRev(ACHAR ch) const                    // deprecated
{
    return this->findLast(ch);
}

inline int AcString::findRev(const wchar_t *pwsz) const         // deprecated
{
    return this->findLast(pwsz, -1);
}

inline int AcString::findRev(const AcString &s) const           // deprecated
{
    // see comment above find() about infinite loop and MB strings
        return this->findLast(s.kwszPtr());
}

inline int AcString::findOneOfRev(const wchar_t *pwsz) const    // deprecated
{
    return this->findLastOneOf(pwsz, -1);
}

inline int AcString::findLast(ACHAR ch, int nStartPos) const
{
    const ACHAR str[2] = {ch, '\0'};
    return this->findLastOneOf(str, nStartPos);
}

// Extraction inlines
//
inline AcString AcString::mid(int nStart, int nNumChars) const
{
    return this->substr(nStart, nNumChars);
}

inline AcString AcString::mid(int nStart) const
{
    return this->mid(nStart, -1);
}

inline AcString AcString::substr(int nNumChars) const
{
    return this->substr(0, nNumChars);
}

inline AcString AcString::left(int nNumChars) const
{
    return this->substr(nNumChars);
}

inline AcString AcString::right(int nNumChars) const
{
    return this->substrRev(nNumChars);
}

inline AcString & AcString::trimLeft(wchar_t wch)
{
    const wchar_t wszChars[] = { wch, L'\0' };
    return this->trimLeft(wszChars);
}

inline AcString & AcString::trimLeft()
{
    return this->trimLeft(nullptr);     // trim whitespace
}

inline AcString & AcString::trimRight(wchar_t wch)
{
    const wchar_t wszChars[] = { wch, L'\0' };
    return this->trimRight(wszChars);
}

inline AcString & AcString::trimRight()
{
    return this->trimRight(nullptr);    // trim whitespace
}

inline AcString & AcString::trim(wchar_t wch)
{
    const wchar_t wszChars[] = { wch, L'\0' };
    return this->trim(wszChars);
}

inline AcString & AcString::trim()
{
    return this->trim(nullptr);         // trim whitespace
}

inline AcString & AcString::trim(const wchar_t *pwszChars)
{
    return this->trimRight(pwszChars).trimLeft(pwszChars);
}

// Assignment inlines
//

inline AcString & AcString::assign(wchar_t wch)
{
    const wchar_t wstr[2] = {wch, L'\0'};
    return this->assign(wstr);
}


inline AcString & AcString::operator = (wchar_t wch)
{
    return this->assign(wch);
}

inline AcString & AcString::operator = (const wchar_t *pwsz)
{
    return this->assign(pwsz);
}

inline AcString & AcString::operator = (const AcString & acs)
{
    return this->assign(acs);
}

inline AcString & AcString::operator = (const AcDbHandle & h)
{
    return this->assign(h);
}

// Modifying inlines
//
inline AcString & AcString::operator += (wchar_t wch)
{
    return this->append(wch);
}

inline AcString & AcString::operator += (const wchar_t *pwsz)
{
    return this->append(pwsz);
}

inline AcString & AcString::operator += (const AcString & acs)
{
    return this->append(acs);
}

inline AcString & AcString::append(wchar_t wch)
{
    const wchar_t wstr[2] = {wch, L'\0'};
    return this->append(wstr);
}

// Concatenation inlines
inline AcString AcString::operator + (wchar_t wch) const
{
    return this->concat(wch);
}

inline AcString AcString::operator + (const wchar_t * pwsz) const
{
    return this->concat(pwsz);
}

inline AcString AcString::operator + (const AcString & acs) const
{
    return this->concat(acs);
}

inline AcString AcString::concat(wchar_t wch) const
{
    const wchar_t wstr[2] = {wch, L'\0'};
    return this->concat(wstr);
}

inline AcString AcString::precat(wchar_t ch) const
{
    const wchar_t str[2] = {ch, '\0'};
    return this->precat(str);
}

// Comparison inlines
//

inline const wchar_t * AcString::kwszPtr() const
{
    return this->m_wsz; // this pointer is never null
}

inline int AcString::compare(wchar_t wch) const
{
    const wchar_t wstr[2] = {wch, L'\0'};
    return this->compare(wstr);
}

inline int AcString::compare(const AcString & acs) const
{
    return this->compare(acs.kwszPtr());
}

inline int AcString::compareNoCase(wchar_t wch) const
{
    const wchar_t wstr[2] = {wch, L'\0'};
    return this->compareNoCase(wstr);
}

inline int AcString::compareNoCase(const AcString & acs) const
{
    return this->compareNoCase(acs.kwszPtr());
}

inline int AcString::collate(const AcString & acs) const
{
    return this->collate(acs.kwszPtr());
}

inline int AcString::collateNoCase(const AcString & acs) const
{
    return this->collateNoCase(acs.kwszPtr());
}

inline bool AcString::operator == (wchar_t wch) const
{
    return this->compare(wch) == 0;
}

inline bool AcString::operator == (const wchar_t *pwsz) const
{
    return this->compare(pwsz) == 0;
}

inline bool AcString::operator == (const AcString & acs) const
{
    return this->compare(acs) == 0;
}

inline bool AcString::operator != (wchar_t wch) const
{
    return this->compare(wch) != 0;
}

inline bool AcString::operator != (const wchar_t *pwsz) const
{
    return this->compare(pwsz) != 0;
}

inline bool AcString::operator != (const AcString & acs) const
{
    return this->compare(acs) != 0;
}

inline bool AcString::operator > (wchar_t wch) const
{
    return this->compare(wch) > 0;
}

inline bool AcString::operator > (const wchar_t *pwsz) const
{
    return this->compare(pwsz) > 0;
}

inline bool AcString::operator > (const AcString & acs) const
{
    return this->compare(acs) > 0;
}

inline bool AcString::operator >= (wchar_t wch) const
{
    return this->compare(wch) >= 0;
}

inline bool AcString::operator >= (const wchar_t *pwsz) const
{
    return this->compare(pwsz) >= 0;
}

inline bool AcString::operator >= (const AcString & acs) const
{
    return this->compare(acs) >= 0;
}

inline bool AcString::operator < (wchar_t wch) const
{
    return this->compare(wch) < 0;
}

inline bool AcString::operator < (const wchar_t *pwsz) const
{
    return this->compare(pwsz) < 0;
}

inline bool AcString::operator < (const AcString & acs) const
{
    return this->compare(acs) < 0;
}

inline bool AcString::operator <= (wchar_t wch) const
{
    return this->compare(wch) <= 0;
}

inline bool AcString::operator <= (const wchar_t *pwsz) const
{
    return this->compare(pwsz) <= 0;
}

inline bool AcString::operator <= (const AcString & acs) const
{
    return this->compare(acs) <= 0;
}

// Inline global operators

inline bool operator == (wchar_t wch, const AcString & acs)
{
    return acs.compare(wch) == 0;
}

inline bool operator == (const wchar_t *pwsz, const AcString & acs)
{
    return acs.compare(pwsz) == 0;
}

inline bool operator != (wchar_t wch, const AcString & acs)
{
    return acs.compare(wch) != 0;
}

inline bool operator != (const wchar_t *pwsz, const AcString & acs)
{
    return acs.compare(pwsz) != 0;
}

inline bool operator > (wchar_t wch, const AcString & acs)
{
    return acs.compare(wch) < 0;
}

inline bool operator > (const wchar_t *pwsz, const AcString & acs)
{
    return acs.compare(pwsz) < 0;
}

inline bool operator >= (wchar_t wch, const AcString & acs)
{
    return acs.compare(wch) <= 0;
}

inline bool operator >= (const wchar_t *pwsz, const AcString & acs)
{
    return acs.compare(pwsz) <= 0;
}

inline bool operator < (wchar_t wch, const AcString & acs)
{
    return acs.compare(wch) > 0;
}

inline bool operator < (const wchar_t *pwsz, const AcString & acs)
{
    return acs.compare(pwsz) > 0;
}

inline bool operator <= (wchar_t wch, const AcString & acs)
{
    return acs.compare(wch) >= 0;
}

inline bool operator <= (const wchar_t *pwsz, const AcString & acs)
{
    return acs.compare(pwsz) >= 0;
}

// These don't modify the AcString.  They return a copy.
inline AcString operator + (ACHAR ch, const AcString & acs)
{
    return acs.precat(ch);
}

inline AcString operator + (const wchar_t *pwsz, const AcString & acs)
{
    return acs.precat(pwsz);
}

inline bool AcString::equalsNoCase(const AcString& left, const AcString& right)
{
    return left.compareNoCase(right) == 0;
}

// Return a unique identifier (pointer) for the input string, to allow fast compares
// using pointer values instead of strings.
// Input strings are converted to lowercase, then are looked up in and stored in an
// internal map. So "ABC" and "abc" return the same AcUniqueString pointer.
// AcUniqueString pointers are valid for the process's lifetime
//
class AcUniqueString
{
public:
    ACBASE_PORT static const AcUniqueString *Intern(const wchar_t *);
};


// We can do inline operators that deal with CStrings, without getting
// into binary format dependencies.  Don't make these out-of-line
// functions, because then we'll have a dependency between our
// components and CString-using clients.
//
#if defined(_AFX) || defined(__OSX_WINAPI_UNIX_STRING_H__) || defined(__ATLSTR_H__)


inline AcString::AcString(const CStringW &csw) : AcString()
{
    const wchar_t *pwsz = (const wchar_t *)csw;
    *this = pwsz;
}

inline AcString & AcString::operator=(const CStringW &csw)
{
    const wchar_t *pwsz = (const wchar_t *)csw;
    return this->assign(pwsz);
}

inline AcString & AcString::operator+=(const CStringW &csw)
{
    const wchar_t *pwsz = (const wchar_t *)csw;
    return this->append(pwsz);
}

inline int AcString::compare(const CStringW & csw) const
{
    const wchar_t *pwsz = (const wchar_t *)csw;
    return this->compare(pwsz);
}

inline int AcString::compareNoCase(const CStringW & csw) const
{
    const wchar_t *pwsz = (const wchar_t *)csw;
    return this->compareNoCase(pwsz);
}

inline int AcString::match(const CStringW & csw) const
{
    const wchar_t *pwsz = (const wchar_t *)csw;
    return this->match(pwsz);
}

inline int AcString::matchNoCase(const CStringW & csw) const
{
    const wchar_t *pwsz = (const wchar_t *)csw;
    return this->matchNoCase(pwsz);
}

inline bool AcString::operator == (const CStringW & csw) const
{
    return this->compare(csw) == 0;
}

inline bool AcString::operator != (const CStringW & csw) const
{
    return this->compare(csw) != 0;
}

inline bool AcString::operator > (const CStringW & csw) const
{
    return this->compare(csw) > 0;
}

inline bool AcString::operator >= (const CStringW & csw) const
{
    return this->compare(csw) >= 0;
}

inline bool AcString::operator < (const CStringW & csw) const
{
    return this->compare(csw) < 0;
}

inline bool AcString::operator <= (const CStringW & csw) const
{
    return this->compare(csw) <= 0;
}

#if defined(_AFX) && !defined(__cplusplus_cli)
// Global CString-related operators
inline bool operator == (const CStringW & csw, const AcString & acs)
{
    return acs.compare(csw) == 0;
}

inline bool operator != (const CStringW & csw, const AcString & acs)
{
    return acs.compare(csw) != 0;
}

inline bool operator >  (const CStringW & csw, const AcString & acs)
{
    return acs.compare(csw) < 0;
}

inline bool operator >= (const CStringW & csw, const AcString & acs)
{
    return acs.compare(csw) <= 0;
}

inline bool operator <  (const CStringW & csw, const AcString & acs)
{
    return acs.compare(csw) > 0;
}

inline bool operator <= (const CStringW & csw, const AcString & acs)
{
    return acs.compare(csw) >= 0;
}

#ifndef DISABLE_CSTRING_PLUS_ACSTRING
inline AcString operator + (const CStringW & csw, const AcString & acs)
{
    const wchar_t *pwsz = (const wchar_t *)csw;
    return acs.precat(pwsz);
}
#endif

#endif

#endif // _AFX

#endif // !_Ac_String_h

