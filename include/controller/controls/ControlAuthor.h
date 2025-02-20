#ifndef CONTROL_AUTHOR_H
#define CONTROL_AUTHOR_H

#include "controller/controls/IControl.h"
#include "models/application/Author.h"

#include "utils/safe_ptr.h"

namespace control::author
{

    class SaveAndQuitAuthors : public AControl
    {
    public:
        SaveAndQuitAuthors();
        ~SaveAndQuitAuthors();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    };

    class SelectAuthor : public AControl
    {
    public:
        SelectAuthor(SafePtr<Author> author);
        ~SelectAuthor();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        SafePtr<Author> m_authorSelected;
    };

    class CreateNewAuthor : public AControl
    {
    public:
        CreateNewAuthor(const std::wstring& authorName);
        ~CreateNewAuthor();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        std::wstring m_authorNameToCreate;
    };

    class DeleteAuthor : public AControl
    {
    public:
        DeleteAuthor(SafePtr<Author> author);
        ~DeleteAuthor();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        SafePtr<Author> m_authorToDelete;
    };

    class SendAuthorList : public AControl
    {
    public:
        SendAuthorList();
        ~SendAuthorList();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    };
}

#endif