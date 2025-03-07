#include "controller/messages/NewProjectMessage.h"

NewProjectMessage::NewProjectMessage(const ProjectInfos& projectInfo, const std::filesystem::path& folderPath, const std::filesystem::path& templatePath)
    : m_projectInfo(projectInfo)
    , m_folderPath(folderPath)
    , m_templatePath(templatePath)
    , language_template_(LanguageType::Nothing)
{}

NewProjectMessage::NewProjectMessage(const ProjectInfos& projectInfo, const std::filesystem::path& folderPath, LanguageType language_template)
    : m_projectInfo(projectInfo)
    , m_folderPath(folderPath)
    , m_templatePath(std::filesystem::path())
    , language_template_(language_template)
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
