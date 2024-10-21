#include "controller/functionSystem/ContextConvertionScan.h"
#include "io/FileUtils.h"
#include "models/pointCloud/PointXYZIRGB.h"
#include "io/imports/IScanFileReader.h"
#include "io/exports/IScanFileWriter.h"
#include "controller/messages/ConvertionMessage.h"
#include "controller/Controller.h"
#include "controller/messages/ImportMessage.h"
#include "controller/controls/ControlProject.h"
#include "controller/controls/ControlApplication.h"
#include "controller/controls/ControlTree.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "io/SaveLoadSystem.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataTree.h"
#include "gui/GuiData/GuiDataIO.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "gui/texts/SplashScreenTexts.hpp"
#include "gui/texts/ContextTexts.hpp"
#include "gui/texts/PointCloudTexts.hpp"
#include "controller/messages/ModalMessage.h"
#include "pointCloudEngine/PCE_core.h"
#include "gui/widgets/ConvertionOptionsBox.h"
#include "utils/Utils.h"

#include "models/graph/ScanNode.h"
#include "models/graph/ScanObjectNode.h"
#include "models/graph/GraphManager.hxx"
// Temporary for large coordinates
#include "io/exports/TlsFileWriter.h"

#include <chrono>
//#include "qmessagebox.h"

#define Ok 0x00000400

constexpr uint64_t POINTS_PER_READ = 2 * 1048576;

BoundingBoxD getScansBoundingBox(const std::vector<glm::dvec3>& scansPositions)
{
    BoundingBoxD projectBBox;
    projectBBox.setEmpty();

    for (const glm::dvec3& position : scansPositions)
    {
        if ((position.x) > projectBBox.xMax) projectBBox.xMax = (position.x);
        if ((position.y) > projectBBox.yMax) projectBBox.yMax = (position.y);
        if ((position.z) > projectBBox.zMax) projectBBox.zMax = (position.z);

        if ((position.x) < projectBBox.xMin) projectBBox.xMin = (position.x);
        if ((position.y) < projectBBox.yMin) projectBBox.yMin = (position.y);
        if ((position.z) < projectBBox.zMin) projectBBox.zMin = (position.z);

    }

    return projectBBox;
}

ContextConvertionScan::ContextConvertionScan(const ContextId& id)
	: ARayTracingContext(id)
    , m_userEdits(true)
    , m_keepOnlyVisiblePoints(true)
{}

ContextConvertionScan::~ContextConvertionScan()
{}

ContextState ContextConvertionScan::start(Controller& controller)
{
	return (m_state = ContextState::waiting_for_input);
}

ContextState ContextConvertionScan::feedMessage(IMessage* message, Controller& controller)
{
    switch (message->getType())
    {
    case IMessage::MessageType::IMPORT_SCAN:
    {
        ImportScanMessage* out = dynamic_cast<ImportScanMessage*>(message);
        if (out == nullptr)
        {
            FUNCLOG << "failed to convert in importMessage" << LOGENDL;
            return (m_state);
        }
        if (out->m_data.paths.empty())
            break;
        m_scanInfo = out->m_data;
        uint64_t maskType(ConvertionOptionsBox::BoxOptions::TRUNCATE_COORDINATES);
        bool force(false);
        std::vector<std::filesystem::path> outputFiles;
        std::filesystem::path scanDir = controller.getContext().cgetProjectInternalInfo().getScansFolderPath();

        controller.updateInfo(new GuiDataSplashScreenStart(TEXT_MESSAGE_SPLASH_SCREEN_READING_DATA, GuiDataSplashScreenStart::SplashScreenType::Message));
        for (const std::filesystem::path& file : m_scanInfo.paths)
        {
            //Note (Aurélien) Brute force to try to fix #227
            std::vector<tls::ScanHeader> headers;
            bool renaming;
            std::wstring log;
            int result = checkScansExist(file, scanDir, outputFiles, headers, renaming, log);
            if (result < 0)
            {
                controller.updateInfo(new GuiDataSplashScreenEnd(GuiDataSplashScreenEnd::SplashScreenType::Message));
                QString msg = TEXT_CONVERTION_ERROR_FILE_NOT_VALID.arg(QString::fromStdWString(file.wstring()));
                msg.append("\n");
                msg.append(QString::fromStdWString(log));
                controller.updateInfo(new GuiDataModal(Ok, msg));
                return (m_state = ContextState::abort);
            }
            if(renaming)
                maskType |= ConvertionOptionsBox::BoxOptions::AUTO_RENAMING_MULTISCAN;
    
            for (const tls::ScanHeader& header : headers)
                m_importScanPosition.push_back(glm::dvec3(header.transfo.translation[0], header.transfo.translation[1], header.transfo.translation[2]));

            switch (ExtensionDictionnary.at(file.extension().string()))
            {
            case FileType::FARO_LS:
            case FileType::FARO_PROJ:
                force |= result > 0;
                maskType |= ConvertionOptionsBox::BoxOptions::ASK_COLOR_PRESENT;
            case FileType::E57:
            case FileType::RCS:
            case FileType::RCP:
            case FileType::PTS:
                maskType |= ConvertionOptionsBox::BoxOptions::PRECISION;
                break;
            }
        }
        controller.updateInfo(new GuiDataSplashScreenEnd(GuiDataSplashScreenEnd::SplashScreenType::Message));

        BoundingBoxD pbbox(getScansBoundingBox(m_importScanPosition));

        if (force)
        {
            int p(0);
            QString arg;
            for (int iterator(0); iterator < outputFiles.size(); iterator++)
            {
                if (iterator >= 5)
                    break;
                arg += QString::fromStdWString(outputFiles[iterator].wstring());
                arg += "\n";
            }
            if (outputFiles.size() >= 5)
                arg += "...\n";
            controller.updateInfo(new GuiDataWarning(TEXT_CONVERTION_FILE_ALREADY_EXIST.arg(arg)));
            maskType |= ConvertionOptionsBox::BoxOptions::FORCE_FILE_OVERWRITE;
        }
        //ask for convertion widget with the good FilType for display options

        glm::dvec3 scanTranslation;
        //Si le projet ne contient pas encore de scans
        if (controller.getGraphManager().getNodesByTypes({ ElementType::Scan }).size() == 0)
        {
            double xTrans = (pbbox.xMax >= BIG_COORDINATES_THRESHOLD || pbbox.xMin <= -BIG_COORDINATES_THRESHOLD) ? round((pbbox.xMax + pbbox.xMin) / 200.0) * 100. : 0.;
            double yTrans = (pbbox.yMax >= BIG_COORDINATES_THRESHOLD || pbbox.yMin <= -BIG_COORDINATES_THRESHOLD) ? round((pbbox.yMax + pbbox.yMin) / 200.0) * 100. : 0.;
            double zTrans = (pbbox.zMax >= BIG_COORDINATES_THRESHOLD || pbbox.zMin <= -BIG_COORDINATES_THRESHOLD) ? round((pbbox.zMax + pbbox.zMin) / 200.0) * 100. : 0.;

            scanTranslation = -glm::dvec3(xTrans, yTrans, zTrans);
            controller.getContext().getProjectInfo().m_importScanTranslation = scanTranslation;
        }
        else
            scanTranslation = controller.getContext().getProjectInfo().m_importScanTranslation;

        controller.updateInfo(new GuiDataConversionOptionsDisplay(maskType, scanTranslation));
        controller.updateInfo(new GuiDataConversionFilePaths(m_scanInfo.paths));

        m_state = ContextState::waiting_for_input;
        break;
    }
    case IMessage::MessageType::CONVERTION_OPTION:
    {
        ConvertionMessage* castedMsg = dynamic_cast<ConvertionMessage*>(message);
        if (castedMsg == nullptr)
        {
            FUNCLOG << "failed to convert in convertionMessage" << LOGENDL;
            return (m_state);
        }
        m_properties = castedMsg->m_properties;

        if (m_scanInfo.positionOption == PositionOptions::ClickPosition)
        {
            m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_POINT_CLOUD_OBJECT_START });
            return ARayTracingContext::start(controller);
        }
        else
            return (m_state = ContextState::ready_for_using);

        break;
    }
    default:
    {
        return ARayTracingContext::feedMessage(message, controller);
        FUNCLOG << "wrong message type" << LOGENDL;
        break;
    }
    }
	return (m_state);
}

ContextState ContextConvertionScan::launch(Controller& controller)
{
    // --- Ray Tracing ---
    if (m_scanInfo.positionOption == PositionOptions::ClickPosition) {
        ARayTracingContext::getNextPosition(controller);
        if (pointMissing())
            return waitForNextPoint(controller);
    }
    // -!- Ray Tracing -!-

    GraphManager& graphManager = controller.getGraphManager();

    std::unordered_set<SafePtr<AGraphNode>> scans = graphManager.getNodesByTypes({ ElementType::Scan, ElementType::PCO });
    std::vector<glm::dvec3> allScansPosition;

    for (const SafePtr<AGraphNode>& scan : scans)
    {
        ReadPtr<AGraphNode> rScan = scan.cget();
        if (!rScan)
            continue;

        allScansPosition.push_back(rScan->getCenter());
    }

    for (glm::dvec3 position : m_importScanPosition)
    {
        position.x += m_properties.truncate.x;
        position.y += m_properties.truncate.y;
        position.z += m_properties.truncate.z;

        allScansPosition.push_back(position);
    }

    BoundingBoxD pbbox(getScansBoundingBox(allScansPosition));
    // FIXME - On doit comparer la bounding box avec maximum pour l'affichage.
    //       - On doit vérifier que les valeurs ne sont pas NaN ou Infinity.
    constexpr double bigFloat = 1.0e+6;
    if (abs(pbbox.xMax - pbbox.xMin) > bigFloat ||
        abs(pbbox.yMax - pbbox.yMin) > bigFloat ||
        abs(pbbox.zMax - pbbox.zMin) > bigFloat)
    {
        // NOTE - Si on ajoute les scans déjà présent dans le projet au scans que l'on veut convertir on aura toujours un problème d'envergure globale des scans.
        //  - Le message est trompeur car on aura toujours une limite avec la solution actuelle des "grandes coordonnées".
        controller.updateInfo(new GuiDataModal(Ok, TEXT_CONVERTION_SCAN_PROJECT_TOO_BIG));
        return (m_state = ContextState::abort);
    }

	//NOTE (Aur?lien) Not Good... but don't want to have that in .h (2)
	tls::PrecisionType precision = (tls::PrecisionType)m_properties.filePrecision;
	m_log.clear();
	auto start = std::chrono::steady_clock::now();

    uint64_t nbScanBeforeImport = graphManager.getNodesByTypes({ ElementType::Scan }).size();
	for (const std::filesystem::path& inputFile : m_scanInfo.paths)
	{
		if (m_state != ContextState::running)
			return ContextState::abort;
	
        convertFile(controller, inputFile, precision);
	}
	float time = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - start).count();
	updateStep(controller, TEXT_CONVERTION_DONE, 0);

	QString log(QString(TEXT_CONVERTION_DONE_TEXT).arg(QString::number(time)));
	controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(log));
	controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));

	controller.getControlListener()->notifyUIControl(new control::project::StartSave());
    
    if (nbScanBeforeImport == 0)
        controller.updateInfo(new GuiDataMoveToData(controller.getContext().getDefaultScan()));

	FUNCLOG << m_log.toStdString() << LOGENDL;
	return  ContextState::done;
}

ContextState ContextConvertionScan::validate(Controller& controller)
{
	return abort(controller);
}

bool ContextConvertionScan::canAutoRelaunch() const
{
	return (false);
}

ContextType ContextConvertionScan::getType() const
{
	return ContextType::scanConversion;
}

void ContextConvertionScan::registerConvertedScan(Controller& controller, const std::filesystem::path& filename, bool overwritedFile, bool asObject, float time)
{
    bool importSuccess = false;
	SaveLoadSystem::ErrorCode error;

    if (m_scanInfo.asObject)
    {
        SafePtr<ScanObjectNode> scanObj = SaveLoadSystem::ImportTlsFileAsObject(filename, controller, error);
        if (scanObj)
        {
            WritePtr<ScanObjectNode> wScanObj = scanObj.get();
            switch (m_scanInfo.positionOption)
            {
            case PositionOptions::ClickPosition:
                wScanObj->setPosition(m_clickResults[0].position);
                break;
            case PositionOptions::GivenCoordinates:
                wScanObj->setPosition(m_scanInfo.positionAsObject);
                break;
            }
            importSuccess = true;
        }
    }
    else
        importSuccess = bool(SaveLoadSystem::ImportNewTlsFile(filename, controller, error));

	if (!importSuccess)
	{
		QString log;
		if (overwritedFile == false)
		{
			FUNCLOG << "Error during control::project::ImportScan" << LOGENDL;
			log = QString(TEXT_SCAN_IMPORT_FAILED).arg(QString::fromStdWString(filename.stem().wstring()));
		}
		else
			log = QString(TEXT_SCAN_IMPORT_DONE_TEXT).arg(QString::fromStdWString(filename.stem().wstring()));

		controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(log));
		controller.updateInfo(new GuiDataTmpMessage(log));
		return;
	}
	FUNCLOG << "update infos during control::project::ImportScan" << LOGENDL;
	QString log(QString(TEXT_SCAN_IMPORT_DONE_TIMING_TEXT).arg(QString::fromStdWString(filename.stem().wstring()),QString::number(time)));

	controller.updateInfo(new GuiDataTmpMessage(log));
	controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(log));

}

void ContextConvertionScan::convertFile(Controller& controller, const std::filesystem::path& inputFile, const tls::PrecisionType& precision)
{
    std::wstring wlog;

    controller.updateInfo(new GuiDataProcessingSplashScreenStart(m_scanInfo.paths.size(), TEXT_CONVERTION_CONVERTING, QString()));
    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString::fromStdWString(inputFile.wstring())));
    IScanFileReader* fileReader = nullptr;

    Import::AsciiInfo asciiInfo;
    if (m_scanInfo.mapAsciiInfo.find(inputFile) != m_scanInfo.mapAsciiInfo.end())
        asciiInfo = m_scanInfo.mapAsciiInfo.at(inputFile);

    if (getScanFileReader(inputFile, wlog, &fileReader, asciiInfo, m_properties.readFlsColor) == false)
    {
        QString log = QString(TEXT_CONVERTION_ERROR_FILE_NOT_VALID).arg(QString::fromStdWString(inputFile.wstring()));
        controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(log));
        FUNCLOG << log.toStdString() << LOGENDL;
        m_log += log;
        return;
    }

    for (uint32_t s = 0; s < fileReader->getScanCount(); ++s) 
    {
        if (m_state != ContextState::running)
        {
            delete fileReader;
            return;
        }
        auto start = std::chrono::steady_clock::now();
        IScanFileWriter* fileWriter = nullptr;

        std::filesystem::path outputDir;

        if(!m_scanInfo.asObject)
            outputDir = controller.getContext().cgetProjectInternalInfo().getScansFolderPath();
        else
            outputDir = controller.getContext().cgetProjectInternalInfo().getObjectsFilesFolderPath();

       
        std::wstring outputName;
        if (fileReader->getScanCount() > 1)
        {
            if (fileReader->getScanCount() > 1 && fileReader->getTlsScanHeader(1).name == fileReader->getTlsScanHeader(0).name)
            {
                std::filesystem::path converter(Utils::completeWithZeros(s + 1));
                outputName = inputFile.stem();
                outputName += L"_" + converter.wstring();
            }
            else
            {
                std::filesystem::path converter(fileReader->getTlsScanHeader(s).name);
                outputName = converter.wstring();
            }
        }
        else
            outputName = inputFile.stem();

        if (!m_properties.overwriteExisting && std::filesystem::exists(outputDir / (outputName + std::wstring(L".tls"))))
        {
            QString log(QString(TEXT_CONVERTION_FILE_SKIPPING).arg(QString::fromStdWString(outputName)));
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(log));
            FUNCLOG << m_log.toStdString() << LOGENDL;
            continue;
        }
        if (getScanFileWriter(outputDir, outputName, FileType::TLS, wlog, &fileWriter) == false)
        {
            QString log(QString(TEXT_CONVERTION_ERROR_FILE_FAILED_TO_WRITE).arg(QString::fromStdWString(outputDir.wstring())));
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(log));
            FUNCLOG << m_log.toStdString() << LOGENDL;
            controller.getControlListener()->notifyUIControl(new control::project::StartSave());
            continue;
        }

        updateStep(controller, TEXT_CONVERTION_CONVERTING, fileReader->getScanCount());

        convertOne(fileReader, s, fileWriter, precision);

        // Truncate the coordinates after conversion (Temporary solution for large coords)
        static_cast<TlsFileWriter*>(fileWriter)->translateOrigin(m_properties.truncate.x, m_properties.truncate.y, m_properties.truncate.z);

        float time = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - start).count();
        registerConvertedScan(controller, fileWriter->getFilePath(), m_properties.overwriteExisting, false, time);

        delete fileWriter;
    }
    delete fileReader;

    if(!m_log.isEmpty())
        controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(m_log));
    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(TEXT_CONVERTION_DONE));
    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));
    FUNCLOG << m_log.toStdString() << LOGENDL;
};


// NOTE - can be used to divide a multiScanin multiple files
bool ContextConvertionScan::convertOne(IScanFileReader* reader, uint32_t readerOffset, IScanFileWriter* writer, tls::PrecisionType outPrec)
{
    if (readerOffset >= reader->getScanCount())
        return false;

    tls::ScanHeader inHeader = reader->getTlsScanHeader(readerOffset);
    tls::ScanHeader outHeader = inHeader;

    // Define the metadata of the future point cloud
    outHeader.precision = outPrec;

    // Begin new point cloud in file
    writer->appendPointCloud(outHeader);

    reader->startReadingScan(readerOffset);

    PointXYZIRGB* pointsBuf = new PointXYZIRGB[POINTS_PER_READ];
    uint64_t N;
    while (reader->readPoints(pointsBuf, POINTS_PER_READ, N))
    {
        writer->addPoints(pointsBuf, N);
    }
    delete[] pointsBuf;

    // End point cloud and finilize the write
    writer->flushWrite();

    return true;
}

void ContextConvertionScan::updateStep(Controller& controller, const QString& state, const uint64_t& step)
{
    m_currentStep += step;
    controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(state, m_currentStep));
}

int ContextConvertionScan::checkScansExist(const std::filesystem::path& inputFile, std::filesystem::path destDir, std::vector<std::filesystem::path>& outputFiles, std::vector<tls::ScanHeader>& headers, bool& renaming, std::wstring& log)
{
    IScanFileReader* fileReader = nullptr;
    int retCode(0);

    Import::AsciiInfo asciiInfo;
    if (m_scanInfo.mapAsciiInfo.find(inputFile) != m_scanInfo.mapAsciiInfo.end())
        asciiInfo = m_scanInfo.mapAsciiInfo.at(inputFile);

    if (getScanFileReader(inputFile, log, &fileReader, asciiInfo) == false)
    {
        return -1;
    }

    if (fileReader->getScanCount() == 0)
    {
        return -1;
    }

    for (uint32_t s = 0; s < fileReader->getScanCount(); ++s)
    {
        std::filesystem::path outputFile = destDir;
        std::filesystem::path converter(fileReader->getTlsScanHeader(s).name);
        outputFile /= fileReader->getScanCount() == 1 ? inputFile.stem() : std::filesystem::path(converter.wstring());
        outputFile += ".tls";
        headers.push_back(fileReader->getTlsScanHeader(s));
        if (std::filesystem::exists(outputFile))
        {
            outputFiles.push_back(outputFile);
            retCode++;
        }
    }
    //Note (Aurélien) only a quick check of scan name on the first ones, not sure there is a case where we need to check every scans...
    renaming = (fileReader->getScanCount() > 1 && fileReader->getTlsScanHeader(1).name == fileReader->getTlsScanHeader(0).name);
    delete fileReader;
    return retCode;
}