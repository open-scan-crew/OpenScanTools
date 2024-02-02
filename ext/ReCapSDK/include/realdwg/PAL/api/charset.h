//  Copyright 2020 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form

#ifndef _PAL_FONT_CHARSET_
#define _PAL_FONT_CHARSET_

enum Charset : int
{
    kUndefinedCharset = -1,
    kAnsiCharset = 0,//ANSI_CHARSET
    kUnicodeCharset = 1,//DEFAULT_CHARSET
    kSymbolCharset = 2,//SYMBOL_CHARSET
    kJapaneseCharset = 128,//SHIFTJIS_CHARSET
    kKoreanCharset = 129,//HANGEUL_CHARSET
    kChineseSimpCharset = 134,//GB2312_CHARSET
    kChineseTradCharset = 136,//CHINESEBIG5_CHARSET
    kJohabCharset = 130,//JOHAB_CHARSET
    kHebrewCharset = 177,//HEBREW_CHARSET
    kArabicCharset = 178,//ARABIC_CHARSET
    kGreekCharset = 161,//GREEK_CHARSET
    kTurkishCharset = 162,//TURKISH_CHARSET
    kVietnameseCharset = 163,//VIETNAMESE_CHARSET
    kThaiCharset = 222,//THAI_CHARSET
    kEastEuropeCharset = 238, //EASTEUROPE_CHARSET
    kRussianCharset = 204, //RUSSIAN_CHARSET
    kBalticCharset = 186,//BALTIC_CHARSET
    kDefaultCharset = kUnicodeCharset,
    kINTERNALCHARSET = 256,

    kBengaliCharset = kINTERNALCHARSET,//BENGALI_CHARSET
    kGurmukhiCharset,//GURMUKHI_CHARSET
    kGujaratiCharset,//GUJARATI_CHARSET
    kTamilCharset,//TAMIL_CHARSET
    kTeluguCharset,//TELUGU_CHARSET
    kKannadaCharset,//KANNADA_CHARSET
    kMalayalamCharset,//MALAYALAM_CHARSET
    kDevanagariCharset,//DEVANAGARI_CHARSET
    kOriyaCharset,// ORIYA_CHARSET

    kMarathiCharset = kDevanagariCharset, //MARATHI_CHARSET
    kHindiCharset = kDevanagariCharset, //HINDI_CHARSET
    kKonkaniCharset = kDevanagariCharset, //KONKANI_CHARSET
    kSanskritCharset = kDevanagariCharset, //SANSKRIT_CHARSET
    kPunjabiharset = kGurmukhiCharset,//PUNJABI_CHARSET
    kAssameseCharset = kUnicodeCharset, //ASSAMESE_CHARSET
    kFinnishCharset = kUnicodeCharset, //FINNISH_CHARSET
    kBelgianCharset = kUnicodeCharset, //BELGIAN_CHARSET
    kGeorgianCharset = kUnicodeCharset, //GEORGIAN_CHARSET
};  

#endif


