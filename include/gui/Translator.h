#ifndef TRANSLATOR_H_
#define TRANSLATOR_H_

#include <qtranslator.h>
#include <filesystem>
#include <map>
#include <set>

class QApplication;

enum LangageType
{
	English,
	Francais,
	Nothing
};

class Translator
{
public:
	static const std::map<LangageType, QString> languageDictionnary;
public:
	Translator(QApplication* application, const std::filesystem::path& translationFolder=L"./translations/");
	~Translator();

	std::set<LangageType> getAvailableLangage() const;
	LangageType getActiveLangage() const;
	bool setActiveLangage(const LangageType& langage);

private:
	QApplication* m_application;
	QTranslator	m_appTranslator;
	QTranslator m_appBaseTranslator;
	LangageType m_current;
	std::map<LangageType, std::vector<QString>> m_langages;
};
#endif
