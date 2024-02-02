//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Copyright 2018 by Autodesk, Inc.
//
// The information contained herein is confidential, proprietary to Autodesk,
// Inc., and considered a trade secret as defined in section 499C of the
// penal code of the State of California.  Use of this information by anyone
// other than authorized employees of Autodesk, Inc. is granted only under a
// written non-disclosure agreement, expressly prescribing the scope and
// manner of such use.
//
//        AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS.
//        AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
//        MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC.
//        DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
//        UNINTERRUPTED OR ERROR FREE.
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstddef>
#include <foundation/RCBuffer.h>
#include <foundation/RCCommonDef.h>

namespace Autodesk { namespace RealityComputing { namespace Foundation {
    /// \brief A Unicode string class.
    ///
    class RC_COMMON_API RCString
    {
    public:
        /// \brief Creates an empty string.
        RCString();

        /// \brief Copy constructor.
        RCString(const RCString& string);

        /// \brief Creates a new Unicode string.
        RCString(const wchar_t* string);
        RCString(wchar_t* const string);

        /// \brief Converts from an ANSI or multi-byte string.
        ///
        /// The conversion is dependent on the current locale.
        RCString(const char* string);
        RCString(char* const string);

        /// \brief Converts from std::string or std::wstring
        ///
        /// Internally attempts to call `c_str()` to get the raw string to perform conversion
        template <typename T>
        RCString(T const& val);

        /// \brief Destructor.
        ~RCString();

        /// \brief Assignment operator.
        RCString& operator=(const RCString& string);

        /// \brief Assignment operator.
        RCString& operator=(const wchar_t* string);

        /// \brief Assignment operator.
        ///
        /// The conversion is dependent on the current locale.
        RCString& operator=(const char* string);

        /// \brief Concatenates strings.
        RCString operator+(const RCString& string) const;
        RCString operator+(const wchar_t* string) const;
        RCString operator+(const char* string) const;

        /// \brief Concatenates strings.
        RCString& operator+=(const RCString& string);
        RCString& operator+=(const wchar_t* string);
        RCString& operator+=(const char* string);

        /// \brief: equals operator
        bool operator==(const RCString& string) const;
        bool operator==(const wchar_t* string) const;
        bool operator==(const char* string) const;

        /// \brief: not equals operator
        bool operator!=(const RCString& string) const;
        bool operator!=(const wchar_t* string) const;
        bool operator!=(const char* string) const;

        /// \brief: Compare operator <
        bool operator<(const RCString& string) const;

        /// \brief Tests whether the string is empty.
        bool isEmpty() const;

        /// \brief Returns the number of characters in the string.
        size_t getLength() const;

        /// \brief Returns a temporary pointer to the string.
        ///
        /// The pointer is valid until the next call to a non const method.
        const wchar_t* getWString() const;
        operator const wchar_t*() const;

        /// \brief Returns a temporary pointer to the string.
        ///
        /// The pointer is valid until the next call to a non const method.
        /// The conversion is dependent on the current locale.
        const char* getString() const;
        operator const char*() const;

        /// \brief Functionally identical to std::basic_string<...>::npos
        static const size_t NPOS;

        /// \brief Compares this instance with the `other` instance, if their string contents are lexically identical
        bool isEqualTo(const RCString& other, bool ignorecase = false) const;
        /// \brief Finds a given substring starting from a given index of the current string
        size_t findSubStringAt(const RCString& substr, size_t pos = 0) const;
        /// \brief Given a substring, mutates this instance at given position, over a span of a given length
        RCString replaceSubStringAt(const RCString& str, size_t pos, size_t len) const;
        /// \brief Finds all substrings matching the old pattern, and replaces them with the new pattern, returns the number of replacements
        int replaceSubString(const RCString& oldpattern, const RCString& newpattern);
        /// \brief Returns the substring starting from given position and spanning given length
        RCString getSubString(size_t pos = 0, size_t len = NPOS) const;
        /// \brief Checks if the string ends with the given \p substr
        bool endsWith(const RCString& substr, bool ignorecase = false) const;

        /// \brief Explodes the string along the given delimiter
        RCBuffer<RCString> split(const RCString& delimiter) const;
        /// \brief Strips all trailing whitespace characters
        RCString trim() const;
        /// \brief Strips the specified trailing single character from both ends of the string
        RCString trim(const RCString& singleChar) const;

        /// \brief Strips all trailing and leading whitespace characters
        RCString removeWhitespace() const;
        /// \brief downcases all characters in the string that can be downcased
        RCString toLower() const;
        /// \brief uppercases all characters in the string that can be uppercased
        RCString toUpper() const;
        /// \brief URL-encodes whitespace, newline, `<` , and `>` characters
        RCString toHTML() const;
        /// \brief converts URI style links into html link attributes
        RCString toHyperlink() const;

        /// \brief Append another string \p rhs to the current string
        RCString& append(const RCString& rhs);

        /// \brief Joins each string in \p stringlist with \p delimiter in between and return an \p RCString
        static RCString join(const RCBuffer<RCString>& stringlist, const RCString& delimiter);

        /// \brief Checks if this string contains any given sub string \p substr
        bool contains(const RCString& substr, bool ignorecase = false) const;

        /// \brief Converts the current string into an integer value
        bool toInteger(int& value) const;

        /// \brief Converts the current string into an unsigned integer value
        bool toUnsignedInteger(unsigned int& value) const;

        /// \brief Converts the current string into a float value
        bool toFloat(float& value) const;

        /// \brief Converts the current string into a double value
        bool toDouble(double& value) const;

        /// \brief Converts an integer to a string
        static RCString fromNumber(int value);

        /// \brief Converts an unsigned integer to a string
        static RCString fromNumber(unsigned int value);

        /// \brief Converts a size_t value to a string
        static RCString fromNumber(size_t value);

        /// \brief Converts a float value to a string
        static RCString fromNumber(float value);

        /// \brief Converts a double value to a string
        static RCString fromNumber(double value);

        /// \brief Generates and returns a hash value for the string
        size_t getHash() const;

        /// \brief Generates and returns a hash value for the string
        RCString getHashString() const;

    private:
        class Cache;
        Cache* mCache;
    };

    template <typename T>
    RCString::RCString(T const& val) : RCString(val.c_str())
    {
    }

}}}    // namespace Autodesk::RealityComputing::Foundation

bool operator==(const wchar_t* lhs, const Autodesk::RealityComputing::Foundation::RCString& rhs);
bool operator==(const char* lhs, const Autodesk::RealityComputing::Foundation::RCString& rhs);
bool operator!=(const wchar_t* lhs, const Autodesk::RealityComputing::Foundation::RCString& rhs);
bool operator!=(const char* lhs, const Autodesk::RealityComputing::Foundation::RCString& rhs);
