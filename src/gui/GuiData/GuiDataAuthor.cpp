#include "gui/GuiData/GuiDataAuthor.h"

#include "models/application/Author.h"

// **** SendAuthors List ****

GuiDataSendAuthorsList::GuiDataSendAuthorsList(const std::unordered_set<SafePtr<Author>>& authors, const bool& projectScope)
    : m_isProjectScope(projectScope)
    , m_authors(authors)
    , m_selectedAuthor(-1)
{
}

GuiDataSendAuthorsList::GuiDataSendAuthorsList(const std::unordered_set<SafePtr<Author>>& authors, const int& selectedAuthor)
    : m_isProjectScope(false)
    , m_authors(authors)
    , m_selectedAuthor(selectedAuthor)
{
}

GuiDataSendAuthorsList::~GuiDataSendAuthorsList()
{
}

guiDType GuiDataSendAuthorsList::getType()
{
    return guiDType::sendAuthorsList;
}

// **** CloseAuthors List ****

GuiDataCloseAuthorsList::GuiDataCloseAuthorsList()
{
}

GuiDataCloseAuthorsList::~GuiDataCloseAuthorsList()
{
}

guiDType GuiDataCloseAuthorsList::getType()
{
    return guiDType::closeAuthorList;
}

// **** GuiDataAuthorSelection ****

GuiDataAuthorNameSelection::GuiDataAuthorNameSelection(const std::wstring& author)
    : m_author(author)
{
}

guiDType GuiDataAuthorNameSelection::getType()
{
    return (guiDType::authorSelection);
}