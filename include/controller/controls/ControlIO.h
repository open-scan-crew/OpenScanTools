#ifndef CONTROL_IO_H_
#define CONTROL_IO_H_

#include "controller/controls/IControl.h"
#include "io/ImageTypes.h"
#include "io/exports/ExportParameters.hpp"

#include "models/project/ProjectInfos.h"
#include "models/application/TagTemplate.h"
#include "models/ElementType.h"

#include <filesystem>
#include <fstream>
#include <unordered_set>

#include <QtGui/qimage.h>

class AGraphNode;
class CameraNode;

namespace control::io
{
    class ExportSubProject : public AControl
    {
    public:
        ExportSubProject(std::filesystem::path folder, ProjectInfos subProjectInfos, ObjectStatusFilter filterType, bool openFolderAfterExport);
        ~ExportSubProject();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        std::filesystem::path m_folder;
        ProjectInfos m_subProjectInfos;
        ObjectStatusFilter m_filterType;
        bool m_openFolderAfterExport;
    };

    class ExportTemplate : public AControl
    {
    public:
        ExportTemplate(std::filesystem::path path, std::unordered_set<SafePtr<sma::TagTemplate>> toExportTemplates);
        ~ExportTemplate();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        std::filesystem::path m_path;
        std::unordered_set<SafePtr<sma::TagTemplate>> m_toExportTemplates;
    };

    class ImportTemplate : public AControl
    {
    public:
        ImportTemplate(std::filesystem::path path);
        ~ImportTemplate();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        std::filesystem::path _path;
    };

    class LinkOSTObjectsContext : public AControl
    {
    public:
        LinkOSTObjectsContext();
        ~LinkOSTObjectsContext();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    };

    class RefreshScanLink : public AControl
    {
    public:
        RefreshScanLink();
        ~RefreshScanLink();
        void doFunction(Controller& controller) override;
        ControlType getType() const override;
    };

    class ImportOSTObjects : public AControl
    {
    public:
        ImportOSTObjects(const std::vector<std::filesystem::path>& filespath);
        ~ImportOSTObjects();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const std::vector<std::filesystem::path>	m_filesPath;
    };

    class ItemsTo : public AControl
    {
    public:
        ItemsTo(const std::filesystem::path& filePath, const std::unordered_set<ElementType>& types, const ObjectStatusFilter& filter, const PrimitivesExportParameters& parameters);
        ~ItemsTo();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
    protected:
        virtual void launchContext(Controller& controller) = 0;
    protected:
        std::filesystem::path                   m_filePath;
        const std::unordered_set<ElementType>   m_types;
        const ObjectStatusFilter                m_filter;
        PrimitivesExportParameters              m_parameters;
    };

    class ItemsToDxf : public ItemsTo
    {
    public:
        ItemsToDxf(const std::filesystem::path& filePath, const std::unordered_set<ElementType>& types, const ObjectStatusFilter& filter, const PrimitivesExportParameters& parameters);
        ~ItemsToDxf();
        ControlType getType() const override;
    protected:
        void launchContext(Controller& controller);
    };

    class ItemsToCSV : public ItemsTo
    {
    public:
        ItemsToCSV(const std::filesystem::path& filePath, const std::unordered_set<ElementType>& types, const ObjectStatusFilter& filter, const PrimitivesExportParameters& parameters);
        ~ItemsToCSV();
        ControlType getType() const override;
    protected:
        void launchContext(Controller& controller);
    };

	class ItemsToStep : public ItemsTo
	{
	public:
		ItemsToStep(const std::filesystem::path& filePath, const std::unordered_set<ElementType>& types, const ObjectStatusFilter& filter, const PrimitivesExportParameters& parameters);
		~ItemsToStep();
		ControlType getType() const override;
	protected:
		void launchContext(Controller& controller);
	};

    class ItemsToOST : public ItemsTo
    {
    public:
        ItemsToOST(const std::filesystem::path& filePath, const std::unordered_set<ElementType>& types, const ObjectStatusFilter& filter, const PrimitivesExportParameters& parameters);
        ~ItemsToOST();
        ControlType getType() const override;
    protected:
        void launchContext(Controller& controller);
    };

    class ItemsToObj : public ItemsTo
    {
    public:
        ItemsToObj(const std::filesystem::path& filePath, const std::unordered_set<ElementType>& types, const ObjectStatusFilter& filter, const PrimitivesExportParameters& parameters);
        ~ItemsToObj();
        ControlType getType() const override;
    protected:
        void launchContext(Controller& controller);
    };

    class ItemsToFbx : public ItemsTo
    {
    public:
        ItemsToFbx(const std::filesystem::path& filePath, const std::unordered_set<ElementType>& types, const ObjectStatusFilter& filter, const PrimitivesExportParameters& parameters);
        ~ItemsToFbx();
        ControlType getType() const override;
    protected:
        void launchContext(Controller& controller);
    };

    class QuickScreenshot : public AControl
    {
    public:
        QuickScreenshot(ImageFormat format, std::filesystem::path filepath = "", bool includeAlpha = true);
        ~QuickScreenshot();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        const ImageFormat m_format;
        std::filesystem::path m_filepath;
        bool m_includeAlpha;
    };

    class RecordPerformance : public AControl
    {
    public:
        RecordPerformance();
        ~RecordPerformance();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    };

    class SetupImageHD : public AControl
    {
    public:
        SetupImageHD(SafePtr<CameraNode> viewport, glm::ivec2 imageSize, int samples, ImageFormat format, ImageHDMetadata metadata, std::filesystem::path filepath, bool showProgressBar, uint32_t hdimagetilesize, bool fullResolutionTraversal);
        ~SetupImageHD();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        SafePtr<CameraNode> m_viewport;
        const glm::ivec2 m_imageSize;
        const int m_multisample;
        const ImageFormat m_format;
        const ImageHDMetadata m_metadata;
        std::filesystem::path m_filepath;
        bool m_showProgressBar;
        uint32_t m_hdimagetilesize;
        bool m_fullResolutionTraversal;
    };

    class GenerateVideoHD : public AControl
    {
    public:
        GenerateVideoHD(std::filesystem::path videoPath, const VideoExportParameters& videoParams);
        ~GenerateVideoHD();
        void doFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        std::filesystem::path m_videoPath;
        VideoExportParameters m_videoParams;
    };

    class ImportScantraModifications : public AControl
    {
    public :
        ImportScantraModifications(std::filesystem::path sqliteDbPath);
        ~ImportScantraModifications();
        void doFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        std::filesystem::path m_sqliteDbPath;
    };

    class ConvertImageToPointCloud : public AControl
    {
    public:
        struct ConvertImage
        {
            QImage inputImage;
            std::filesystem::path outputPath;
            glm::vec3 origin;
            int normalAxeMode; //0 is xy, 1 is xz, 2 is yz
            float length;
            int colorTransparencyMode; //0 is ignore, 1 is white, 2 is black
        };
    public:
        ConvertImageToPointCloud(const ConvertImage& params);
        ~ConvertImageToPointCloud();
        void doFunction(Controller& controller) override;
        ControlType getType() const override;

    private:
        ConvertImage m_params;
        std::ofstream m_ptsWriteScan;
    };
}

#endif
