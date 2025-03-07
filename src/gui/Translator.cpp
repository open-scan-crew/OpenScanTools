#include "gui/Translator.h"
#include "utils/Logger.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qdir.h>
#include <QtCore/qtranslator.h>
#include "QtCore/qlocale.h"

#include <clocale>

#include <map>

#define TEMPLATE_TRANSLATION_FILE "qt_*.qm"

#define TrLog Logger::log(LoggerMode::TranslatorLog)

static QTranslator s_appTranslator;
static QTranslator s_appBaseTranslator;
LanguageType s_current;
std::map<LanguageType, std::vector<QString>> m_languages;

const std::map<LanguageType, QString> cst_languageDictionnary = {
    { LanguageType::English, "English" },
    { LanguageType::Francais, "Français" }
};

static const std::map<QString, LanguageType> dictionnary = {
    { "en", LanguageType::English },
    { "fr", LanguageType::Francais }
};

static const std::unordered_map<LanguageType, std::string> locales = {
    { LanguageType::English, "en_EN.UTF-8" },
    { LanguageType::Francais, "fr_FR.UTF-8" }
};

void Translator::initTranslationFolder(const std::filesystem::path& translationFolder)
{
    s_current = LanguageType::English;

    // format systems language
    QString defaultLocale = QLocale::system().name(); // e.g. "de_DE"
    defaultLocale.truncate(defaultLocale.lastIndexOf('_')); // e.g. "de"
    QString path(QString::fromStdWString(translationFolder.wstring()));
    QDir dir(path);
    QStringList fileNames = dir.entryList(QStringList());
    TrLog << "Load from " << translationFolder << Logger::endl;
    for (int i = 0; i < fileNames.size(); ++i) 
    {
        // get locale extracted by filename
        QString locale(fileNames[i]);// "TranslationExample_de.qm"
        locale.truncate(locale.lastIndexOf('.')); // "TranslationExample_de"
        locale.remove(0, locale.indexOf('_') + 1); // "de"
        if (dictionnary.find(locale) != dictionnary.end())
        {
            LanguageType language = dictionnary.at(locale);
            if (m_languages.find(language) != m_languages.end())
                m_languages[language].push_back(path + "/" + fileNames[i]);
            else
                m_languages[language] = { path + "/" + fileNames[i] };
            TrLog << "Load " << path.toStdString() << "/" << fileNames[i].toStdString() << Logger::endl;
        }
    }
}

std::set<LanguageType> Translator::getAvailableLanguage()
{
    std::set<LanguageType> languages;
    for (auto iterator : m_languages)
        languages.insert(iterator.first);
    return languages;
}

LanguageType Translator::getActiveLanguage()
{
    return s_current;
}

bool Translator::setActiveLanguage(LanguageType language)
{
    if (m_languages.find(language) == m_languages.end() || m_languages.at(language).size() < 2)
        return false;
    QCoreApplication::removeTranslator(&s_appTranslator);
    QCoreApplication::removeTranslator(&s_appBaseTranslator);

    if (!s_appTranslator.load(m_languages.at(language)[0]))
        return false;
    if (!s_appBaseTranslator.load(m_languages.at(language)[1]))
        return false;

    if (locales.find(language) != locales.end())
    {
        std::setlocale(LC_TIME, locales.at(language).c_str());
    }

    TrLog << "Active language set to " << getLanguageQStr(language).toStdString() << Logger::endl;
    QCoreApplication::installTranslator(&s_appTranslator);
    QCoreApplication::installTranslator(&s_appBaseTranslator);

    s_current = language;
    return true;
}

QString Translator::getLanguageQStr(LanguageType language)
{
    if (cst_languageDictionnary.find(language) != cst_languageDictionnary.end())
        return cst_languageDictionnary.at(language);
    else
        return QString("Error_not_a_language");
}