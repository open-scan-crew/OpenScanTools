#include "gui/GuiData/GuiDataAuthor.h"

#include "models/application/Author.h"

// **** SendAuthors List ****

GuiDataSendAuthorsList::GuiDataSendAuthorsList(const std::unordered_set<SafePtr<Author>>& authors, const bool& projectScope)
    : is_project_scope_(projectScope)
    , authors_(authors)
    , active_author_()
{
}

GuiDataSendAuthorsList::GuiDataSendAuthorsList(const std::unordered_set<SafePtr<Author>>& authors, SafePtr<Author> _active_author)
    : is_project_scope_(false)
    , authors_(authors)
    , active_author_(_active_author)
{
}

GuiDataSendAuthorsList::~GuiDataSendAuthorsList()
{
}

guiDType GuiDataSendAuthorsList::getType()
{
    return guiDType::sendAuthorsList;
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