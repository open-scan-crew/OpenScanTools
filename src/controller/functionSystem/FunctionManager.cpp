#include "controller/functionSystem/FunctionManager.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "controller/Controller.h"
#include "magic_enum/magic_enum.hpp"
#include "controller/messages/IMessage.h"
#include "utils/Logger.h"

IdGiver<ContextId> FunctionManager::s_contextIdGiver(0);

FunctionManager::FunctionManager()
    : m_cFactory()
    , m_actualCId(INVALID_CONTEXT_ID)
    , m_lastFunction(ContextType::none)
{
    s_contextIdGiver = IdGiver<ContextId>(0);
}

FunctionManager::~FunctionManager()
{
    m_actualCId = INVALID_CONTEXT_ID;
    std::lock_guard<std::mutex> lock(m_mutex);
    for (std::pair<const ContextId, AContext*>& it : m_contextList)
        delete (it.second);
    m_contextList.clear();

    for (auto iterator : m_contextMessages)
        for (IMessage* message : iterator.second)
            delete message;

    for (auto iterator : m_threads)
    {
        iterator.second->join();
        delete iterator.second;
    }
}

ContextId FunctionManager::launchFunction(Controller& controller, const ContextType& type)
{
    FUNCLOG << "launch context " << magic_enum::enum_name(type) << LOGENDL;
    //NOTE (Aurélien) POC Undo/Redo context
    //controller->updateInfo(new GuiDataUndoRedoAble(true, true));
    abort(controller, m_actualCId);

    m_actualCId = s_contextIdGiver.giveAutoId();

    AContext* actualContext = m_cFactory.createContext(type, m_actualCId);
    if (actualContext->canAutoRelaunch() == true)
        m_lastFunction = type;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_contextList.insert(std::pair<ContextId, AContext*>(m_actualCId, actualContext));
    }
    actualContext->start(controller);
    controller.updateInfo(new GuiDataActivatedFunctions(type));
    return (m_actualCId);
}

ContextId FunctionManager::launchBackgroundFunction(Controller& controller, const ContextType& type, const ContextId& parentId)
{
    FUNCLOG << "launch backgroung context " << magic_enum::enum_name(type) << LOGENDL;
    ContextId contextId = s_contextIdGiver.giveAutoId();
    AContext* backgroundContext(m_cFactory.createContext(type, contextId, parentId));
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_contextList.insert(std::pair<ContextId, AContext*>(contextId, backgroundContext));
    }
    backgroundContext->start(controller);
    return (contextId);
}

ContextType FunctionManager::isActiveContext()
{
    AContext* actualContext = getContext(m_actualCId);
    if (actualContext != nullptr)
        return (actualContext->getType());
    else
        return (ContextType::none);
}

void FunctionManager::feedMessage(Controller& controller, IMessage* message)
{
    FUNCLOG << "current context get new message" << LOGENDL;
    if (m_actualCId != INVALID_CONTEXT_ID)
        feedMessageToSpecificContext(controller, message, m_actualCId);
}

bool FunctionManager::feedMessageToSpecificContext(Controller& controller, IMessage* message, const ContextType& type)
{
    std::vector<std::pair<ContextId, AContext*>>contextFound;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& iterator : m_contextList)
        {
            if (iterator.second->getType() == type)
            {
                contextFound.push_back({ iterator.first, iterator.second });
            }
        }
    }

    for (const auto& context : contextFound)
    {
        if (context.second->getState() != ContextState::waiting_for_input)
        {
            FUNCLOG << "Stocking message for context " << context.first << " type " << magic_enum::enum_name<IMessage::MessageType>(message->getType()) << LOGENDL;
            m_contextMessages[context.first].push_back(message->copy());
        }
        else
        {
            FUNCLOG << " Feeding message to context " << context.first << LOGENDL;
            context.second->feedMessage(message, controller);
        }
    }
    return (contextFound.size());
}

bool FunctionManager::feedMessageToSpecificContext(Controller& controller, IMessage* message, const ContextId& id)
{
    AContext* specificContext = getContext(id);
    if (specificContext)
    {
        if (specificContext->getState() != ContextState::waiting_for_input)
        {
            FUNCLOG << "Stocking message for context " << id << " type " << magic_enum::enum_name<IMessage::MessageType>(message->getType()) << LOGENDL;
            m_contextMessages[id].push_back(message->copy());
        }
        else
        {
            FUNCLOG << " Feeding message to context " << id << " type " << magic_enum::enum_name<IMessage::MessageType>(message->getType()) << LOGENDL;
            specificContext->feedMessage(message, controller);
        }
        return true;
    }
    return false;
}

bool FunctionManager::feedResultToSpecificContext(Controller& controller, IMessage* message, const ContextId& id)
{
    AContext* specificContext = getContext(id);
    if (specificContext)
    {
        if (specificContext->getState() != ContextState::waiting_for_result)
        {
            FUNCLOG << "Stocking message for context " << id << " type " << magic_enum::enum_name<IMessage::MessageType>(message->getType()) << LOGENDL;
            m_contextResults[id].push_back(message->copy());
        }
        else
        {
            FUNCLOG << " Feeding message to context " << id << " type " << magic_enum::enum_name<IMessage::MessageType>(message->getType()) << LOGENDL;
            specificContext->feedMessage(message, controller);
        }
        return true;
    }
    return false;
}

void FunctionManager::validate(Controller& controller)
{
    //FUNCLOG << "Validate current function" << LOGENDL;
    controller.updateInfo(new GuiDataTmpMessage(""));
    if (m_actualCId == INVALID_CONTEXT_ID)
    {
        if (m_lastFunction != ContextType::none)
            launchFunction(controller, m_lastFunction);
        return;
    }
    else
    {
        validate(controller, m_actualCId);
    }
}

void FunctionManager::validate(Controller& controller, const ContextId& id)
{
    AContext* context = getContext(id);
    if (context != nullptr)
        context->validate(controller);
}

void FunctionManager::abort(Controller& controller)
{
    abort(controller, m_actualCId);
    controller.updateInfo(new GuiDataTmpMessage(""));
}

void FunctionManager::abort(Controller& controller, const ContextId& id)
{
    AContext* context = getContext(id);
    if (context != nullptr)
    {
        FUNCLOG << "Abort context " << magic_enum::enum_name(context->getType()) << LOGENDL;
        context->abort(controller);
    }
}

void FunctionManager::updateContexts(Controller& controller)
{
    std::vector<ContextId> contextToErase;
    std::unordered_map<ContextId, AContext*> copyContexts;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        copyContexts = m_contextList;
    }

    for (auto contextIt : copyContexts)
    {
        if (contextIt.second->getState() == ContextState::abort ||
            contextIt.second->getState() == ContextState::done)
        {
            contextToErase.push_back(contextIt.first);
        }
        else if (contextIt.second->getState() == ContextState::ready_for_using)
        {
            if (m_threads.find(contextIt.first) != m_threads.end() &&
                m_threads[contextIt.first] != nullptr)
            {
                if (!m_threads[contextIt.first]->joinable())
                    continue;
                FUNCLOG << "Update deleting thread context " << contextIt.first << LOGENDL;
                m_threads[contextIt.first]->join();
                delete m_threads[contextIt.first];
                m_threads[contextIt.first] = nullptr;
            }
            AContext* pContext = contextIt.second;
            pContext->setState(ContextState::running);
            m_threads[contextIt.first] = new std::thread([&controller, pContext]() { pContext->launch(controller); });
        }
        else if (contextIt.second->getState() == ContextState::waiting_for_input)
        {
            if (m_contextMessages.find(contextIt.first) != m_contextMessages.end() && m_contextMessages.at(contextIt.first).size())
            {
                FUNCLOG << "unstock context (" << contextIt.first << ") message " << " type " << magic_enum::enum_name<IMessage::MessageType>((*m_contextMessages.at(contextIt.first).begin())->getType()) << " size: " << m_contextMessages.at(contextIt.first).size() << LOGENDL;
                contextIt.second->feedMessage(*m_contextMessages.at(contextIt.first).begin(), controller);
                delete* m_contextMessages.at(contextIt.first).begin();
                m_contextMessages.at(contextIt.first).pop_front();
            }
        }
        else if (contextIt.second->getState() == ContextState::waiting_for_result)
        {
            if (m_contextResults.find(contextIt.first) != m_contextResults.end() && m_contextResults.at(contextIt.first).size())
            {
                FUNCLOG << "unstock context (" << contextIt.first << ") result " << " type " << magic_enum::enum_name<IMessage::MessageType>((*m_contextResults.at(contextIt.first).begin())->getType()) << " size: " << m_contextResults.at(contextIt.first).size() << LOGENDL;
                contextIt.second->feedMessage(*m_contextResults.at(contextIt.first).begin(), controller);
                delete* m_contextResults.at(contextIt.first).begin();
                m_contextResults.at(contextIt.first).pop_front();
            }
        }
    }

    AContext* actualContext = getContext(m_actualCId);
    if (actualContext != nullptr)
    {
        /*if (actualContext->getState() == ContextState::running)
        {
            m_actualCId = INVALID_CONTEXT_ID;
        }
        else */if (actualContext->getState() == ContextState::abort ||
            actualContext->getState() == ContextState::done)
        {
            m_actualCId = INVALID_CONTEXT_ID;
            controller.updateInfo(new GuiDataActivatedFunctions(ContextType::none));
        }
    }

    // Safely erase the contexts from the manager list
    for (ContextId id : contextToErase)
    {
        killContext(controller, id);
    }
}

AContext* FunctionManager::getActiveContext()
{
    if (m_contextList.find(m_actualCId) == m_contextList.end())
        return (nullptr);
    return (m_contextList.at(m_actualCId));
}

void FunctionManager::killContext(Controller& controller, ContextId idToKill)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::unordered_map<ContextId, AContext*>::iterator contextIt = m_contextList.find(idToKill);
    if (contextIt != m_contextList.end())
    {
        FUNCLOG << "Killing context " << idToKill << LOGENDL;
        if (m_threads.find(idToKill) != m_threads.end())
        {
            if (!m_threads[idToKill]->joinable())
                return;
            FUNCLOG << "Killing thread for context " << idToKill << LOGENDL;
            m_threads[idToKill]->join();
            delete m_threads[idToKill];
            m_threads.erase(idToKill);
        }
        if (m_contextMessages.find(idToKill) != m_contextMessages.end())
        {
            for (IMessage* message : m_contextMessages.at(idToKill))
                delete message;
            m_contextMessages.erase(idToKill);
        }
        if (m_contextResults.find(idToKill) != m_contextResults.end())
        {
            for (IMessage* message : m_contextResults.at(idToKill))
                delete message;
            m_contextResults.erase(idToKill);
        }
        delete (m_contextList.at(idToKill));
        m_contextList.erase(idToKill);
    }
    else
    {
        FUNCLOG << "ERROR: Try to kill an unreferenced context (id " << idToKill << ")" << LOGENDL;
    }
}

AContext* FunctionManager::getContext(ContextId cid)
{
    //Fixme (Aurélien) : To do better but it correct AQ 481
    std::lock_guard<std::mutex> lock(m_mutex);
    std::unordered_map<ContextId, AContext*>::iterator contextIt = m_contextList.find(cid);
    if (contextIt != m_contextList.end())
        return (contextIt->second);
    else
        return nullptr;
}