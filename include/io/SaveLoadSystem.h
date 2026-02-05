#ifndef SAVE_LOAD_SYSTEM_H
#define SAVE_LOAD_SYSTEM_H

#include "utils/safe_ptr.h"
#include "controller/Controller.h"
#include "models/project/ProjectInfos.h"
#include "models/application/TagTemplate.h"

#include <string>
#include <filesystem>
#include <functional>
#include <unordered_set>

class CameraNode;
class AGraphNode;
class PointCloudNode;

class Author;

namespace SaveLoadSystem
{
    enum ErrorCode { Success = 0, Failed_Write_Permission, Failed_To_Open, Failed_To_Load };
    enum class ObjectsFileType { Tlo, Tld, Tlv, Tlo_Backup, Tld_Backup, Tlv_Backup };
    using ProgressCallback = std::function<void(size_t processed, size_t total)>;

    template<typename ListType>
    ListType ImportNewList(const std::filesystem::path& filePath);

    template<typename ListType>
    std::filesystem::path ExportCSVList(ListType list, std::filesystem::path path);

    template<typename ListType>
    std::filesystem::path ExportCSVList(SafePtr<ListType> list, std::filesystem::path path);

    bool IsBackFilesExists(const std::filesystem::path& filePath, std::vector<std::filesystem::path>& backups);
    bool RestoreBackupFiles(const std::vector<std::filesystem::path>& backups);

    bool readProjectTypes(const Controller& controller, const std::filesystem::path& filePath);
    void importJsonProject(const std::filesystem::path& filePath, Controller& controller, std::string& errorMsg);
    SafePtr<PointCloudNode> ImportNewTlsFile(const std::filesystem::path& filePath, bool is_object, Controller& controller, ErrorCode& errorCode, ProgressCallback progress = {});

    void importAuthorObjects(const std::vector<std::filesystem::path>& importFiles, std::unordered_set<SafePtr<AGraphNode>>& succesfulImport, std::unordered_set<SafePtr<AGraphNode>>& fileNotFoundObjectImport, Controller& controller);

    void checkPointCloudPath(SafePtr<PointCloudNode> pcNode);
    std::filesystem::path findPointCloudPath(WritePtr<PointCloudNode>& wPCNode, const ProjectInternalInfo& internalInfo, const std::filesystem::path& searchFolder);

    //Return data of failed file object import 
    std::unordered_set<SafePtr<AGraphNode>> LoadFileObjects(Controller& controller, const std::unordered_set<SafePtr<AGraphNode>>& fileObjects, std::filesystem::path folder, bool forceCopy);
    void ExportToProjectFileObjects(Controller& controller, const ProjectInternalInfo& exportProjectInfo, const std::unordered_set<SafePtr<AGraphNode>>& objectsToExport);

    template<typename ListType>
    std::filesystem::path ExportLists(const std::vector<ListType>& lists, const std::filesystem::path& filePath);

    template<typename ListType>
    std::filesystem::path ExportLists(const std::unordered_set<SafePtr<ListType>>& lists, const std::filesystem::path& filePath);

    template<typename ListType>
    std::vector<ListType> ImportLists(const std::filesystem::path& filePath);

    // filePath is by default set to nothing, it means that the export will automaticaly export in the default template file
    // located in user/document/s/Template/TagTemplates/Templates.tlt
    // Otherwise the export will try to export to the given path. The export will actually fail if a user try to export to a
    // folder protected from writing or administration rights
    std::filesystem::path ExportTemplates(const std::unordered_set<SafePtr<sma::TagTemplate>>& templates, ErrorCode& errorCode, const std::filesystem::path& filePath);
    std::filesystem::path ExportTemplates(const std::vector<sma::TagTemplate>& templates, ErrorCode& errorCode, const std::filesystem::path& filePath);

    // filePath is by default set to nothing, it means that the import will automaticaly look for the default template file
    // located in user/document/OpenScanTools/Template/TagTemplates/Templates.tlt
    // Otherwise the import will look at the path given in parameter
    std::vector<sma::TagTemplate> ImportTemplates(const Controller& controller, const std::filesystem::path& filePath);

    std::unordered_set<SafePtr<Author>> loadLocalAuthors(const Controller& controller, ErrorCode& errorCode, const std::filesystem::path& filePath = "");
    std::filesystem::path saveAuthors(const std::unordered_set<SafePtr<Author>>& authors, ErrorCode& errorCode, const std::filesystem::path& filePath = "");
    std::wstring getAuthorFilename(const Author& auth);

    void loadArboFile(Controller& controller, const std::filesystem::path& folderPath);
    void exportArboFile(const std::filesystem::path& folderPath, const Controller& controller);

    bool ExportProject(Controller& controller, const std::unordered_set<SafePtr<AGraphNode>>& objects, const ProjectInternalInfo& internalInfo, const ProjectInfos& projectInfos, const SafePtr<CameraNode>& camera);

    std::list<std::wstring> splitOnChar(std::wstring origin, std::wstring charList);

    bool ExportAuthorObjects(const Controller& controller, const std::filesystem::path& exportFolder, const std::unordered_set<SafePtr<AGraphNode>>& objectsToExport, bool exportListTemplateWith);
};

#endif
