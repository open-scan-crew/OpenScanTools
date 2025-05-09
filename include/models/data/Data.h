#ifndef DATA_H_
#define DATA_H_

#include <string>
#include "crossguid/guid.hpp"
#include "utils/safe_ptr.h"
#include "utils/Color32.hpp"
#include "models/data/Marker.h"

#include <unordered_map>

typedef xg::Guid hLinkId;

class Author;

struct s_hyperlink
{
	std::wstring hyperlink;
	std::wstring name;

	bool operator== (const s_hyperlink& s1) const
	{
		if (s1.hyperlink == this->hyperlink && s1.name == this->name)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};

class Controller;

class Data
{
public:
	Data();
    Data(const Data& data);
	virtual ~Data();

	void copyUIData(const Data& data, bool copyId);

	void setId(xg::Guid id);
	void setVisible(bool visible);
	void setSelected(bool selected);

	void setAuthor(SafePtr<Author> author);
	void setCreationTime(time_t time);
	void setModificationTime(time_t time);
	void setUserIndex(uint32_t userIndex);

	void setName(const std::wstring& name);
	void setDescription(const std::wstring& desc);
	void setDescription_str(const std::string& desc);
	void setDiscipline(const std::wstring& discipline);
	void setPhase(const std::wstring& phase);
	void setIdentifier(const std::wstring& identifier);
	void setHyperlinks(const std::unordered_map<hLinkId, s_hyperlink>& links);
	void setColor(const Color32& color);
	void setMarkerIcon(scs::MarkerIcon icon);

	virtual void setDefaultData(const Controller& controller);

	bool operator==(const Data& data) const;

	virtual bool isVisible() const;
	virtual bool isSelected() const;

	xg::Guid getId() const;
	SafePtr<Author> getAuthor() const;
	time_t getCreationTime() const;
	time_t getModificationTime() const;
	uint32_t getUserIndex() const;

	const std::wstring& getName() const;
	virtual std::wstring getComposedName() const;
	std::wstring getStringTimeCreated() const;
	std::wstring getStringTimeModified() const;
	std::wstring getDescription() const;
	std::wstring getDiscipline() const;
	std::wstring getPhase() const;
	std::wstring getIdentifier() const;
	const std::unordered_map<hLinkId, s_hyperlink>& getHyperlinks() const;
	const Color32& getColor() const;
	scs::MarkerIcon getMarkerIcon() const;

protected:
	bool m_visible = true;
	bool m_selected;
	xg::Guid m_id;
	SafePtr<Author> m_author;

	uint32_t m_userIndex;
	std::wstring m_name;
	std::wstring m_description;

	std::wstring m_discipline;
	std::wstring m_phase;

	std::wstring m_identifier;

	std::time_t m_timeCreated;
	std::time_t m_timeModified;
	std::unordered_map<hLinkId, s_hyperlink> m_hyperlinks;

	Color32 m_color;
	scs::MarkerIcon marker_icon_;
};

#endif // !UIDATA_H_