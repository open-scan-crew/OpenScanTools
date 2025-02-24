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
    ** SaveAndQuitAuthors
    */

    SaveAndQuitAuthors::SaveAndQuitAuthors()
    {
    }

    SaveAndQuitAuthors::~SaveAndQuitAuthors()
    {
    }

    void SaveAndQuitAuthors::doFunction(Controller& controller)
    {
        SaveLoadSystem::ErrorCode errorMsg;
        SaveLoadSystem::saveAuthors(controller.getContext().getLocalAuthors(), errorMsg);
        assert(errorMsg == SaveLoadSystem::ErrorCode::Success);

        controller.updateInfo(new GuiDataSendAuthorsList(controller.getContext().getLocalAuthors(), -1));
        if (errorMsg != SaveLoadSystem::ErrorCode::Success)
            controller.updateInfo(new GuiDataWarning(TEXT_WRITE_FAILED_PERMISSION));

        ReadPtr<Author> rAuth = controller.getContext().getActiveAuthor().cget();
        if (!rAuth || !rAuth->getId().isValid())
            controller.updateInfo(new GuiDataWarning(TEXT_WARNING_AUTHOR_WRONG_CLOSE));
        else
        {
            controller.updateInfo(new GuiDataCloseAuthorsList());
            controller.updateInfo(new GuiDataAuthorNameSelection(rAuth->getName()));
        }
        CONTROLLOG << "control::application::SaveAndQuitAuthors" << LOGENDL;
    }

    bool SaveAndQuitAuthors::canUndo() const
    {
        return (false);
    }

    void SaveAndQuitAuthors::undoFunction(Controller& controller)
    {
    }

    ControlType SaveAndQuitAuthors::getType() const
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
            CONTROLLOG << "control::application::SelectAuthor " << rAuth->getId() << LOGENDL;
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

        controller.updateInfo(new GuiDataSendAuthorsList(controller.getContext().getLocalAuthors(), -1));
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
        controller.getContext().remLocalAuthors({ m_authorToDelete });
        controller.updateInfo(new GuiDataSendAuthorsList(controller.getContext().getLocalAuthors(), -1));
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
        std::unordered_set<SafePtr<Author>> authors(controller.getContext().getLocalAuthors());

        int index(0);
        for (const SafePtr<Author>& auth : authors)
        {
            if (auth == controller.getContext().getActiveAuthor())
                break;
            index++;
        }
        if (index == authors.size())
            index = 0;

        controller.updateInfo(new GuiDataSendAuthorsList(authors, index));
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
}
