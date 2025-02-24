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
    GuiDataSendAuthorsList(const std::unordered_set<SafePtr<Author>>& authors, const int& selectedAuthor);
    ~GuiDataSendAuthorsList();
    guiDType getType() override;
public:
    const bool m_isProjectScope;
    const std::unordered_set<SafePtr<Author>> m_authors;
    const int m_selectedAuthor;
};

class GuiDataCloseAuthorsList : public IGuiData
{
public:
    GuiDataCloseAuthorsList();
    ~GuiDataCloseAuthorsList();
    guiDType getType() override;
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
