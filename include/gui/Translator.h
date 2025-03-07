#ifndef TRANSLATOR_H_
#define TRANSLATOR_H_

#include "gui/LanguageType.h"

#include <filesystem>
#include <set>

#include <QtCore/qstring.h>

class Translator
{
public:
    static void initTranslationFolder(const std::filesystem::path& translationFolder);

    static std::set<LanguageType> getAvailableLanguage();
    static LanguageType getActiveLanguage();
    static bool setActiveLanguage(LanguageType language);
    static QString getLanguageQStr(LanguageType language);
};

#endif
