#ifndef GUI_DATA_AUTHOR_H
#define GUI_DATA_AUTHOR_H

#include "gui/GuiData/IGuiData.h"

#include "utils/safe_ptr.h"

#include <unordered_set>

class Author;

class GuiDataSendAuthorsList : public IGuiData
{
public:
    GuiDataSendAuthorsList(const std::unordered_set<SafePtr<Author>>& authors, const bool& projectScope);
    GuiDataSendAuthorsList(const std::unordered_set<SafePtr<Author>>& authors, SafePtr<Author> activeAuthor);
    ~GuiDataSendAuthorsList();
    guiDType getType() override;
public:
    const bool is_project_scope_;
    const std::unordered_set<SafePtr<Author>> authors_;
    const SafePtr<Author> active_author_;
};

class GuiDataAuthorNameSelection : public IGuiData
{
public:
    GuiDataAuthorNameSelection(const std::wstring& author);
    guiDType getType() override;
public:
    const std::wstring m_author;
};

#endif
