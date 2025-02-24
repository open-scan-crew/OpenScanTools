#include "controller/controls/ControlAuthor.h"

#include "controller/Controller.h"
#include "controller/ControllerContext.h"

#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataAuthor.h"

#include "gui/texts/ErrorMessagesTexts.hpp"

#include "gui/texts/AuthorTexts.hpp"

#include "io/SaveLoadSystem.h"

#include "utils/Logger.h"

namespace control::author
{
    /*
    ** SaveAuthors
    */

    SaveAuthors::SaveAuthors()
    {
    }

    SaveAuthors::~SaveAuthors()
    {
    }

    void SaveAuthors::doFunction(Controller& controller)
    {
        SaveLoadSystem::ErrorCode errorMsg;
        SaveLoadSystem::saveAuthors(controller.getContext().getLocalAuthors(), errorMsg);
        assert(errorMsg == SaveLoadSystem::ErrorCode::Success);

        if (errorMsg != SaveLoadSystem::ErrorCode::Success)
            controller.updateInfo(new GuiDataWarning(TEXT_WRITE_FAILED_PERMISSION));

        ReadPtr<Author> rAuth = controller.getContext().getActiveAuthor().cget();
        if (!rAuth || !rAuth->getId().isValid())
            controller.updateInfo(new GuiDataWarning(TEXT_WARNING_AUTHOR_WRONG_CLOSE));

        CONTROLLOG << "control::application::SaveAuthors" << LOGENDL;
    }

    bool SaveAuthors::canUndo() const
    {
        return (false);
    }

    void SaveAuthors::undoFunction(Controller& controller)
    {
    }

    ControlType SaveAuthors::getType() const
    {
        return (ControlType::saveAuthors);
    }

    /*
    ** SelectAuthor
    */

    SelectAuthor::SelectAuthor(SafePtr<Author> author)
    {
        m_authorSelected = author;
    }

    SelectAuthor::~SelectAuthor()
    {
    }

    void SelectAuthor::doFunction(Controller& controller)
    {
        controller.getContext().setActiveAuthor(m_authorSelected);
        ReadPtr<Author> rAuth = m_authorSelected.cget();
        if (rAuth)
        {
            CONTROLLOG << "control::application::SelectAuthor " << rAuth->getId() << LOGENDL;
            controller.updateInfo(new GuiDataAuthorNameSelection(rAuth->getName()));
        }
        else
            CONTROLLOG << "control::application::SelectAuthor cannot select" << LOGENDL;
    }

    bool SelectAuthor::canUndo() const
    {
        return (false);
    }

    void SelectAuthor::undoFunction(Controller& controller)
    {
    }

    ControlType SelectAuthor::getType() const
    {
        return (ControlType::selectAuthor);
    }

    /*
    ** CreateNewAuthor
    */

    CreateNewAuthor::CreateNewAuthor(const std::wstring& authorName)
    {
        m_authorNameToCreate = authorName;
    }

    CreateNewAuthor::~CreateNewAuthor()
    {
    }

    void CreateNewAuthor::doFunction(Controller& controller)
    {
        CONTROLLOG << "control::application::CreateNewAuthor" << LOGENDL;
        bool nameAlreadyExist = false;
        for (const SafePtr<Author>& auth : controller.getContext().getLocalAuthors())
        {
            ReadPtr<Author> rAuth = auth.cget();
            if (rAuth && rAuth->getName() == m_authorNameToCreate)
            {
                nameAlreadyExist = true;
                break;
            }
        }
        if (nameAlreadyExist)
        {
            controller.updateInfo(new GuiDataWarning(TEXT_WARNING_AUTHOR_CREATE));
            return;
        }

        SafePtr<Author> newAuth = make_safe<Author>(m_authorNameToCreate);
        controller.getContext().addLocalAuthors({ newAuth });

        SendAuthorList::sendAuthorList(controller);
    }

    bool CreateNewAuthor::canUndo() const
    {
        return (false);
    }

    void CreateNewAuthor::undoFunction(Controller& controller)
    {
    }

    ControlType CreateNewAuthor::getType() const
    {
        return (ControlType::createAuthor);
    }

    /*
    ** DeleteAuthor
    */

    DeleteAuthor::DeleteAuthor(SafePtr<Author> author)
    {
        m_authorToDelete = author;
    }

    DeleteAuthor::~DeleteAuthor()
    {
    }

    void DeleteAuthor::doFunction(Controller& controller)
    {
        controller.getContext().remLocalAuthors(m_authorToDelete);
        SendAuthorList::sendAuthorList(controller);
        CONTROLLOG << "control::application::DeleteAuthor" << LOGENDL;
    }

    bool DeleteAuthor::canUndo() const
    {
        return (false);
    }

    void DeleteAuthor::undoFunction(Controller& controller)
    {
    }

    ControlType DeleteAuthor::getType() const
    {
        return (ControlType::deleteAuthor);
    }

    /*
    ** SendAuthorList
    */

    SendAuthorList::SendAuthorList()
    {
    }

    SendAuthorList::~SendAuthorList()
    {
    }

    void SendAuthorList::doFunction(Controller& controller)
    {
        sendAuthorList(controller);
        CONTROLLOG << "control::application::SendAuthorList" << LOGENDL;
    }

    bool SendAuthorList::canUndo() const
    {
        return (false);
    }

    void SendAuthorList::undoFunction(Controller& controller)
    {
    }

    ControlType SendAuthorList::getType() const
    {
        return (ControlType::sendAuthorList);
    }

    void SendAuthorList::sendAuthorList(Controller& controller)
    {
        std::unordered_set<SafePtr<Author>> authors(controller.getContext().getLocalAuthors());
        SafePtr<Author> active = controller.getContext().getActiveAuthor();

        controller.updateInfo(new GuiDataSendAuthorsList(authors, active));
    }
}
