#ifndef PROJECT_INFO_MESSAGE_H_
#define PROJECT_INFO_MESSAGE_H_

#include "controller/messages/IMessage.h"
#include "models/project/ProjectInfos.h"
#include "models/project/ProjectTypes.h"
#include "models/OpenScanToolsModelEssentials.h"

#include "io/exports/ExportParameters.hpp"

#include <filesystem>

class NewProjectMessage : public IMessage
{
public:
    NewProjectMessage(const ProjectInfos& projectInfo, const std::filesystem::path& folderPath, const std::filesystem::path& templatePath);
    NewProjectMessage(const ProjectInfos& projectInfo, const std::filesystem::path& folderPath, const ProjectTemplate& projectTemplate);
    ~NewProjectMessage();
    MessageType getType() const override;	
    IMessage* copy() const;

public:
    const ProjectInfos m_projectInfo;
    const std::filesystem::path m_folderPath;
    const std::filesystem::path m_templatePath;
    ProjectTemplate m_baseProjectTemplate;
};

class NewSubProjectMessage : public IMessage
{
public:
    NewSubProjectMessage(const ProjectInfos& subProjectInfo, const ProjectInternalInfo& subProjectInternal, ObjectStatusFilter objectFilterType);
    ~NewSubProjectMessage();
    MessageType getType() const override;
    IMessage* copy() const;

public:
    const ProjectInfos m_subProjectInfo;
    const ProjectInternalInfo m_subProjectInternal;
    const ObjectStatusFilter m_objectFilterType;
};
#endif