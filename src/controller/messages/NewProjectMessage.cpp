#include "controller/messages/NewProjectMessage.h"
#include "gui/Translator.h"

NewProjectMessage::NewProjectMessage(const ProjectInfos& projectInfo, const std::filesystem::path& folderPath, const std::filesystem::path& templatePath)
    : m_projectInfo(projectInfo)
    , m_folderPath(folderPath)
    , m_templatePath(templatePath)
    , m_baseProjectTemplate()
{}

NewProjectMessage::NewProjectMessage(const ProjectInfos& projectInfo, const std::filesystem::path& folderPath, const ProjectTemplate& projectTemplate)
    : m_projectInfo(projectInfo)
    ,m_folderPath(folderPath)
    , m_templatePath(std::filesystem::path())
    , m_baseProjectTemplate(projectTemplate)
{
}

NewProjectMessage::~NewProjectMessage()
{}

IMessage::MessageType NewProjectMessage::getType() const
{
    return IMessage::MessageType::NEW_PROJECT;
}

IMessage* NewProjectMessage::copy() const
{
    return new NewProjectMessage(*this);
}

NewSubProjectMessage::NewSubProjectMessage(const ProjectInfos& subProjectInfo, const ProjectInternalInfo& subProjectInternal, ObjectStatusFilter objectFilterType)
    : m_subProjectInfo(subProjectInfo)
    , m_subProjectInternal(subProjectInternal)
    , m_objectFilterType(objectFilterType)
{
}

NewSubProjectMessage::~NewSubProjectMessage()
{
}

IMessage::MessageType NewSubProjectMessage::getType() const
{
    return IMessage::MessageType::SUB_PROJECT;
}

IMessage* NewSubProjectMessage::copy() const
{
    return new NewSubProjectMessage(*this);
}
