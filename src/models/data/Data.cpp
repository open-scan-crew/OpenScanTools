#include "models/data/Data.h"
#include "controller/ControllerContext.h"
#include "gui/LanguageType.h"
#include "utils/Utils.h"
#include "utils/Config.h"
#include "utils/ProjectColor.hpp"
#include "utils/time.h"


Data::Data()
	: m_userIndex(0)
	, m_name(L"")
	, m_author(SafePtr<Author>())
	, m_timeCreated(time(nullptr))
	, m_timeModified(time(nullptr))
	, m_visible(true)
	, m_selected(false)
	, m_description(L"")
	, m_discipline(L"")
	, m_phase(L"")
	, m_identifier(L"")
	, m_hyperlinks({})
	, m_color(ProjectColor::getColor("BLUE"))
{
	LanguageType language = Config::getLanguage();
	std::unordered_map<LanguageType, std::wstring> disciplineDefault;
	std::unordered_map<LanguageType, std::wstring> phaseDefault;
	disciplineDefault.insert({ LanguageType::English, L"All" });
	disciplineDefault.insert({ LanguageType::Francais, L"Toutes" });
	phaseDefault.insert({ LanguageType::English,  L"All" });
	phaseDefault.insert({ LanguageType::Francais, L"Toutes" });

	if (disciplineDefault.find(language) != disciplineDefault.end() && phaseDefault.find(language) != phaseDefault.end())
	{
		m_discipline = disciplineDefault.at(language);
		m_phase = phaseDefault.at(language);
	}
	else
	{
		m_discipline = disciplineDefault.at(LanguageType::English);
		m_phase = phaseDefault.at(LanguageType::English);
	}

	m_id = xg::newGuid();

	time_t timeNow;
	setCreationTime(time(&timeNow));
	setModificationTime(time(&timeNow));
}

Data::Data(const Data& copy)
	: m_id(copy.getId())
	, m_userIndex(copy.getUserIndex())
	, m_name(copy.getName())
	, m_author(copy.getAuthor())
	, m_timeCreated(copy.getCreationTime())
	, m_timeModified(copy.getModificationTime())
	, m_visible(copy.isVisible())
	, m_selected(copy.isSelected())
	, m_description(copy.getDescription())
	, m_discipline(copy.getDiscipline())
	, m_phase(copy.getPhase())
	, m_identifier(copy.getIdentifier())
	, m_hyperlinks(copy.getHyperlinks())
	, m_color(copy.getColor())
{}

Data::~Data()
{ }


void Data::copyUIData(const Data& data, bool copyId)
{
	m_name = data.getName();
	if(copyId)
		m_id = data.getId();
	m_timeModified = data.getModificationTime();
	m_author = data.getAuthor();
	m_visible = data.isVisible();
	m_description = data.getDescription();
	m_discipline = data.getDiscipline();
	m_phase = data.getPhase();
	m_identifier = data.getIdentifier();
	m_userIndex = data.getUserIndex();
	m_hyperlinks = data.getHyperlinks();
	m_color = data.getColor();
}

void Data::setName(const std::wstring& name)
{
	m_name = name;
}

void Data::setId(xg::Guid  id)
{
	m_id = id;
}

void Data::setCreationTime(time_t time)
{
	m_timeCreated = time;
}

void Data::setModificationTime(time_t time)
{
	m_timeModified = time;
}

void Data::setAuthor(SafePtr<Author> author)
{
	m_author = author;
}

void Data::setVisible(bool visible)
{
	m_visible = visible;
}

void Data::setSelected(bool selected)
{
	m_selected = selected;
}

void Data::setDescription(const std::wstring& desc)
{
	m_description = desc;
}

void Data::setDescription_str(const std::string& desc)
{
	m_description = Utils::from_utf8(desc);
}

void Data::setDiscipline(const std::wstring& discipline)
{
	m_discipline = discipline;
}

void Data::setPhase(const std::wstring& phase)
{
	m_phase = phase;
}

void Data::setIdentifier(const std::wstring& identifier)
{
	m_identifier = identifier;
}

void Data::setUserIndex(uint32_t userIndex)
{
	m_userIndex = userIndex;
}

void Data::setHyperlinks(const std::unordered_map<hLinkId, s_hyperlink>& links)
{
	m_hyperlinks = links;
}

void Data::setColor(const Color32& color)
{
	m_color = color;
}

void Data::setDefaultData(const ControllerContext& context)
{
	if(!context.getActiveName().empty())
		setName(context.getActiveName());

	setAuthor(context.getActiveAuthor());
	setDiscipline(context.getActiveDiscipline());
	setPhase(context.getActivePhase());
	setIdentifier(context.getActiveIdentifer());
	setColor(context.getActiveColor());
}


bool Data::operator==(const Data& data) const
{
	return (this->getId() == data.getId());
}

const std::wstring& Data::getName() const
{
	return (m_name);
}

std::wstring Data::getComposedName() const
{
	if (m_identifier.empty())
		return Utils::wCompleteWithZeros(m_userIndex) + L"." + m_name;
	return Utils::wCompleteWithZeros(m_userIndex) + L"." + m_identifier + L"." + m_name;
}

xg::Guid Data::getId() const
{
	return (m_id);
}

time_t Data::getCreationTime() const
{
	return (m_timeCreated);
}

time_t Data::getModificationTime() const
{
	return (m_timeModified);
}

std::wstring Data::getStringTimeCreated() const
{
	wchar_t strDate[256];
	std::wcsftime(strDate, sizeof(strDate), DISPLAY_WIDE_TIME_FORMAT, std::localtime(&m_timeCreated));

	return (std::wstring(strDate));
}

std::wstring Data::getStringTimeModified() const
{
	wchar_t strDate[256];
	std::wcsftime(strDate, sizeof(strDate), DISPLAY_WIDE_TIME_FORMAT, std::localtime(&m_timeModified));

	return (std::wstring(strDate));
}

SafePtr<Author> Data::getAuthor() const
{
	return (m_author);
}

bool Data::isVisible() const
{
	return (m_visible);
}

bool Data::isSelected() const
{
	return (m_selected);
}

const std::unordered_map<hLinkId, s_hyperlink>& Data::getHyperlinks() const
{
	return (m_hyperlinks);
}

std::wstring Data::getDescription() const
{
	return (m_description);
}

std::wstring Data::getDiscipline() const
{
	return (m_discipline);
}

std::wstring Data::getPhase() const
{
	return (m_phase);
}

std::wstring Data::getIdentifier() const
{
	return (m_identifier);
}

uint32_t Data::getUserIndex() const
{
	return (m_userIndex);
}

const Color32& Data::getColor() const
{
	return m_color;
}

