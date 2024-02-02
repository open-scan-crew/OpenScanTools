#include "gui/Translator.h"
#include "utils/Logger.h"
#include <QtWidgets/qapplication.h>
#include <QtCore/qdir.h>
#include <clocale>

#define TEMPLATE_TRANSLATION_FILE "qt_*.qm"

#define TrLog Logger::log(LoggerMode::TranslatorLog)


const std::map<LangageType, QString> Translator::languageDictionnary = { {LangageType::English, "English"},
																		 {LangageType::Francais, QString::fromStdWString(L"Français")} };
static const std::map<QString, LangageType> dictionnary = { {"en",LangageType::English},
															{"fr",LangageType::Francais} };

static const std::unordered_map<LangageType, std::string> locales = { {LangageType::English, "en_EN.UTF-8"}, {LangageType::Francais, "fr_FR.UTF-8"} };

Translator::Translator(QApplication* application, const std::filesystem::path& translationFolder)
	: m_application(application)
	, m_current(LangageType::English)
{
    // format systems langage
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
			if (m_langages.find(dictionnary.at(locale)) != m_langages.end())
				m_langages[dictionnary.at(locale)].push_back(path + "/" + fileNames[i]);
			else
				m_langages[dictionnary.at(locale)] = { path + "/" + fileNames[i] };
			TrLog << "Load " << path.toStdString() << "/" << fileNames[i].toStdString() << Logger::endl;
		}
	}
}

Translator::~Translator()
{}

std::set<LangageType> Translator::getAvailableLangage() const
{
	std::set<LangageType> langages;
	for (auto iterator : m_langages)
		langages.insert(iterator.first);
	return langages;
}

LangageType Translator::getActiveLangage() const
{
	return m_current;
}

bool Translator::setActiveLangage(const LangageType& langage)
{
	if (m_langages.find(langage) == m_langages.end() || m_langages[langage].size() < 2)
		return false;
	m_application->removeTranslator(&m_appTranslator);
	m_application->removeTranslator(&m_appBaseTranslator);

	if (!m_appTranslator.load(m_langages.at(langage)[0]))
		return false;
	if (!m_appBaseTranslator.load(m_langages.at(langage)[1]))
		return false;

	if (locales.find(langage) != locales.end())
	{
		std::setlocale(LC_TIME, locales.at(langage).c_str());
	}

	TrLog << "Activation of  " << langage << Logger::endl;
	m_application->installTranslator(&m_appTranslator);
	m_application->installTranslator(&m_appBaseTranslator);

	m_current = langage;
	return true;
}