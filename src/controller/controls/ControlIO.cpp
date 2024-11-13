#include "controller/controls/ControlIO.h"
#include "controller/Controller.h"
#include "controller/IControlListener.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"

#include "controller/controls/ControlTemplateEdit.h"

#include "io/SaveLoadSystem.h"

#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataHD.h"
#include "gui/GuiData/GuiDataIO.h"
#include "gui/Texts.hpp"
#include "gui/texts/SplashScreenTexts.hpp"

#include "controller/messages/DataIdListMessage.h"
#include "controller/messages/FilesMessage.h"
#include "controller/messages/PrimitivesExportParametersMessage.h"
#include "controller/messages/ClippingExportParametersMessage.h"
#include "controller/messages/VideoExportParametersMessage.h"
#include "controller/messages/NewProjectMessage.h"

#include "models/graph/GraphManager.h"
#include "models/graph/ClusterNode.h"
#include "models/graph/APointCloudNode.h"

#include "io/exports/CSVWriter.h"
#include "utils/time.h"

#include "SQLiteCpp/Database.h"

#include "utils/Utils.h"
#include "utils/System.h"


namespace control::io
{
	ExportSubProject::ExportSubProject(std::filesystem::path folder, ProjectInfos subProjectInfos, ObjectStatusFilter filterType, bool openFolderAfterExport)
	{
		m_folder = folder;
		m_subProjectInfos = subProjectInfos;
		m_filterType = filterType;
		m_openFolderAfterExport = openFolderAfterExport;
	}

	ExportSubProject::~ExportSubProject()
	{
	}

	void ExportSubProject::doFunction(Controller& controller)
	{
		m_subProjectInfos.m_customScanFolderPath.clear();

		ProjectInternalInfo subInternal;
		Utils::System::createDirectoryIfNotExist(m_folder / m_subProjectInfos.m_projectName);
		subInternal.setProjectFolderPath(m_folder / m_subProjectInfos.m_projectName, m_subProjectInfos.m_projectName);

		//Export Scan
		{
			controller.getFunctionManager().launchFunction(controller, ContextType::exportSubProject);

			ExportInitMessage imessage(false, false, true, false);
			controller.getFunctionManager().feedMessage(controller, &imessage);

			ClippingExportParameters param;
			param.pointCloudFilter = ObjectStatusFilter::VISIBLE;
			param.outFileType = FileType::TLS;
			param.method = ExportClippingMethod::SCAN_SEPARATED;
			param.clippingFilter = ExportClippingFilter::ACTIVE;
			param.encodingPrecision = tls::PrecisionType::TL_OCTREE_100UM;
			param.maxScanPerProject = 0;
			param.outFolder = subInternal.getScansFolderPath();
			param.openFolderAfterExport = m_openFolderAfterExport;
			IOLOG << "Sub project out scan folder : " << Utils::to_utf8(param.outFolder.wstring()) << LOGENDL;
			ClippingExportParametersMessage pmessage(param);
			controller.getFunctionManager().feedMessage(controller, &pmessage);

			NewSubProjectMessage smessage(m_subProjectInfos, subInternal, m_filterType);
			controller.getFunctionManager().feedMessage(controller, &smessage);
		}
	}

	bool ExportSubProject::canUndo() const
	{
		return false;
	}

	void ExportSubProject::undoFunction(Controller& controller)
	{}

	ControlType ExportSubProject::getType() const
	{
		return ControlType::exportSubProject;
	}

	// ExportTemplate

	ExportTemplate::ExportTemplate(std::filesystem::path path, std::unordered_set<SafePtr<sma::TagTemplate>> toExportTemplates)
	{
		m_toExportTemplates = toExportTemplates;
		m_path = path;
	}

	ExportTemplate::~ExportTemplate()
	{ }

	void ExportTemplate::doFunction(Controller& controller)
	{
		std::filesystem::path fileName = "";
		if (m_toExportTemplates.size() == 1)
		{
			ReadPtr<sma::TagTemplate> rTemp = (*m_toExportTemplates.begin()).cget();
			if (!rTemp)
				return;
			fileName = rTemp->getName() + L".tlt";
		}
		else
			fileName = "TemplateSelection.tlt";

		SaveLoadSystem::ErrorCode error;
		if (SaveLoadSystem::ExportTemplates(m_toExportTemplates, error, m_path / fileName) == "")
			controller.updateInfo(new GuiDataWarning(TEXT_WRITE_FAILED_PERMISSION));
		CONTROLLOG << "control::tagTemplates::SaveTemplates do" << LOGENDL;
	}

	bool ExportTemplate::canUndo() const
	{
		return (false);
	}

	void ExportTemplate::undoFunction(Controller& controller)
	{ }

	ControlType ExportTemplate::getType() const
	{
		return (ControlType::exportTagTemplate);
	}

	//controll::tagTemplate::ImportTemplate

	ImportTemplate::ImportTemplate(std::filesystem::path path)
	{
		_path = path;
	}

	ImportTemplate::~ImportTemplate()
	{ }

	void ImportTemplate::doFunction(Controller& controller)
	{
		//SaveLoadSystem::ErrorCode error;
		controller.getContext().setTemplates(SaveLoadSystem::ImportTemplates(controller, _path));
		controller.getControlListener()->notifyUIControl(new control::tagTemplate::SendTemplateList());
		CONTROLLOG << "control::tagTemplates::SaveTemplates do" << LOGENDL;
	}

	bool ImportTemplate::canUndo() const
	{
		return (false);
	}

	void ImportTemplate::undoFunction(Controller& controller)
	{ }

	ControlType ImportTemplate::getType() const
	{
		return (ControlType::importTagTemplate);
	}


	/*
	** LinkOSTObjectsContext
	*/

	LinkOSTObjectsContext::LinkOSTObjectsContext()
	{
	}

	LinkOSTObjectsContext::~LinkOSTObjectsContext()
	{
	}

	void LinkOSTObjectsContext::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::linkFileOSTObjects);
	}

	bool LinkOSTObjectsContext::canUndo() const
	{
		return false;
	}

	void LinkOSTObjectsContext::undoFunction(Controller& controller)
	{
	}

	ControlType LinkOSTObjectsContext::getType() const
	{
		return ControlType::linkOSTObjectsContext;
	}

	/*
	** RefreshScanLink
	*/

	RefreshScanLink::RefreshScanLink()
	{}

	RefreshScanLink::~RefreshScanLink()
	{}

	void RefreshScanLink::doFunction(Controller& controller)
	{
		std::unordered_set<SafePtr<AGraphNode>> scans = controller.cgetGraphManager().getNodesByTypes({ ElementType::Scan, ElementType::PCO });
		for (const SafePtr<AGraphNode>& scan : scans)
		{
			WritePtr<APointCloudNode> wScan = static_pointer_cast<APointCloudNode>(scan).get();
			if (!wScan)
				continue;
			wScan->freeScanFile();
		}
		SaveLoadSystem::LoadFileObjects(controller, scans, "", false);
	}

	ControlType RefreshScanLink::getType() const
	{
		return ControlType::refreshScanLink;
	}

	/*
	** ImportOSTObjects
	*/

	ImportOSTObjects::ImportOSTObjects(const std::vector<std::filesystem::path>& filespath)
		: m_filesPath(filespath)
	{}

	ImportOSTObjects::~ImportOSTObjects()
	{}

	void ImportOSTObjects::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::importOSTObjects);
		FilesMessage fmessage(m_filesPath);
		controller.getFunctionManager().feedMessage(controller, &fmessage);

	}

	bool ImportOSTObjects::canUndo() const
	{
		return (false);
	}

	void ImportOSTObjects::undoFunction(Controller& controller)
	{}

	ControlType ImportOSTObjects::getType() const
	{
		return (ControlType::importOSTObjects);
	}
		
	/*
	** ItemsTo
	*/

	ItemsTo::ItemsTo(const std::filesystem::path& filePath, const std::unordered_set<ElementType>& types, const ObjectStatusFilter& filter, const PrimitivesExportParameters& parameters)
		: m_filePath(filePath)
		, m_types(types)
		, m_filter(filter)
		, m_parameters(parameters)
	{}

	ItemsTo::~ItemsTo()
	{}

	void ItemsTo::doFunction(Controller& controller)
	{

		std::unordered_set<SafePtr<AGraphNode>> items = controller.getGraphManager().getNodesByTypes(m_types, m_filter);
		
		launchContext(controller);
		std::vector<std::filesystem::path> path({ m_filePath });
		PrimitivesExportParametersMessage pmessage(m_parameters);
		controller.getFunctionManager().feedMessage(controller, &pmessage);
		FilesMessage fmessage(path);
		controller.getFunctionManager().feedMessage(controller, &fmessage);
		DataListMessage message(items);
		controller.getFunctionManager().feedMessage(controller, &message);
	}

	bool ItemsTo::canUndo() const 
	{
		return (false);
	}

	void ItemsTo::undoFunction(Controller& controller) 
	{}

	/*
	** ItemsToDxf
	*/

	ItemsToDxf::ItemsToDxf(const std::filesystem::path& filePath, const std::unordered_set<ElementType>& types, const ObjectStatusFilter& filter, const PrimitivesExportParameters& parameters)
		: ItemsTo(filePath, types, filter, parameters)
	{}

	ItemsToDxf::~ItemsToDxf()
	{}

	void ItemsToDxf::launchContext(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::exportDxf);
	}

	ControlType ItemsToDxf::getType() const
	{
		return (ControlType::itemsToDXFIO);
	}

	/*
	** ItemsToCSV
	*/

	ItemsToCSV::ItemsToCSV(const std::filesystem::path& filePath, const std::unordered_set<ElementType>& types, const ObjectStatusFilter& filter, const PrimitivesExportParameters& parameters)
		: ItemsTo(filePath, types, filter, parameters)
	{}

	ItemsToCSV::~ItemsToCSV()
	{}

	void ItemsToCSV::launchContext(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::exportCSV);
	}

	ControlType ItemsToCSV::getType() const
	{
		return (ControlType::itemsToCSVIO);
	}

	/*
	** ItemsToStep
	*/

	ItemsToStep::ItemsToStep(const std::filesystem::path & filePath, const std::unordered_set<ElementType>& types, const ObjectStatusFilter & filter, const PrimitivesExportParameters& parameters)
		: ItemsTo(filePath, types, filter, parameters)
	{}

	ItemsToStep::~ItemsToStep()
	{}

	void ItemsToStep::launchContext(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::exportStep);
	}

	ControlType ItemsToStep::getType() const
	{
		return (ControlType::itemsToStepIO);
	}

	/*
	** ItemsToOST
	*/


	ItemsToOST::ItemsToOST(const std::filesystem::path& filePath, const std::unordered_set<ElementType>& types, const ObjectStatusFilter& filter, const PrimitivesExportParameters& parameters)
		: ItemsTo(filePath, types, filter, parameters)
	{}

	ItemsToOST::~ItemsToOST()
	{}

	ControlType ItemsToOST::getType() const
	{
		return ControlType::itemsToOSToolsIO;
	}

	void ItemsToOST::launchContext(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::exportOpenScanTools);
	}

	/*
	** ItemsToObj
	*/

	ItemsToObj::ItemsToObj(const std::filesystem::path& filePath, const std::unordered_set<ElementType>& types, const ObjectStatusFilter& filter, const PrimitivesExportParameters& parameters)
		: ItemsTo(filePath, types, filter, parameters)
	{}

	ItemsToObj::~ItemsToObj()
	{}

	ControlType ItemsToObj::getType() const
	{
		return ControlType::itemsToObjIO;
	}

	void ItemsToObj::launchContext(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::exportObj);
	}

	/*
	** ItemsToFbx
	*/

	ItemsToFbx::ItemsToFbx(const std::filesystem::path& filePath, const std::unordered_set<ElementType>& types, const ObjectStatusFilter& filter, const PrimitivesExportParameters& parameters)
		: ItemsTo(filePath, types, filter, parameters)
	{}

	ItemsToFbx::~ItemsToFbx()
	{}

	ControlType ItemsToFbx::getType() const
	{
		return ControlType::itemsToFbxIO;
	}

	void ItemsToFbx::launchContext(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::exportFbx);
	}

	/*
	** QuickScreenshot
	*/

	QuickScreenshot::QuickScreenshot(ImageFormat format, std::filesystem::path filepath)
		: m_format(format)
		, m_filepath(filepath)
	{}

	QuickScreenshot::~QuickScreenshot()
	{}

	void QuickScreenshot::doFunction(Controller& controller)
	{
		if (m_filepath.empty())
		{
			m_filepath = controller.getContext().cgetProjectInternalInfo().getQuickScreenshotsFolderPath();
			Utils::System::createDirectoryIfNotExist(m_filepath);
			std::time_t result = std::time(nullptr);
			std::tm* time = localtime(&result);
			m_filepath /= std::to_string(1900 + time->tm_year) + "-" + Utils::completeWithZeros(1 + time->tm_mon, 2) + "-" + Utils::completeWithZeros(time->tm_mday, 2) + "_" + std::to_string(time->tm_hour) + "-" + Utils::completeWithZeros(time->tm_min, 2) + "-" + Utils::completeWithZeros(time->tm_sec, 2);
		}
		m_filepath.replace_extension(".png");
		controller.updateInfo(new GuiDataScreenshot(m_filepath, m_format));
	}

	bool  QuickScreenshot::canUndo() const
	{
		return false;
	}
		
	void  QuickScreenshot::undoFunction(Controller& controller)
	{}

	ControlType QuickScreenshot::getType() const
	{
		return (ControlType::quickScreenshot);
	}

	/*
	** RecordPerformance
	*/

	RecordPerformance::RecordPerformance()
	{}

	RecordPerformance::~RecordPerformance()
	{}

	void RecordPerformance::doFunction(Controller& controller)
	{
		std::filesystem::path pathname(controller.getContext().cgetProjectInternalInfo().getProjectFolderPath());
		std::time_t result = std::time(nullptr);
		std::tm* time = localtime(&result);
		pathname /= "PerfRecord_" + std::to_string(1900 + time->tm_year) + "-" + Utils::completeWithZeros(1 + time->tm_mon, 2) + "-" + Utils::completeWithZeros(time->tm_mday, 2) + "_" + std::to_string(time->tm_hour) + "-" + Utils::completeWithZeros(time->tm_min, 2) + "-" + Utils::completeWithZeros(time->tm_sec, 2) + ".csv";
		controller.updateInfo(new GuiDataRenderRecordPerformances(pathname));
	}

	bool RecordPerformance::canUndo() const
	{
		return false;
	}

	void RecordPerformance::undoFunction(Controller& controller)
	{}

	ControlType RecordPerformance::getType() const 
	{
		return (ControlType::recordPerformance);
	}

	/*
	** SetupImageHD
	*/

	SetupImageHD::SetupImageHD(SafePtr<CameraNode> _viewport, glm::ivec2 _imageSize, int _samples, ImageFormat _format, ImageHDMetadata _metadata, std::filesystem::path filepath, bool showProgressBar, uint32_t hdimagetilesize)
		: m_viewport(_viewport)
		, m_imageSize(_imageSize)
		, m_multisample(_samples)
		, m_format(_format)
		, m_metadata(_metadata)
		, m_filepath(filepath)
		, m_showProgressBar(showProgressBar)
		, m_hdimagetilesize(hdimagetilesize)
	{}

	SetupImageHD::~SetupImageHD()
	{}

	void SetupImageHD::doFunction(Controller& controller)
	{
		if (m_filepath.empty())
		{
			if (m_metadata.ortho)
				m_filepath = controller.getContext().cgetProjectInternalInfo().getOrthoHDFolderPath();
			else
				m_filepath = controller.getContext().cgetProjectInternalInfo().getPerspHDFolderPath();
			Utils::System::createDirectoryIfNotExist(m_filepath);

			m_filepath /= "HD_";
			m_filepath += std::to_string(m_imageSize.x) + "x" + std::to_string(m_imageSize.y) + "_";
			std::time_t t = std::time(nullptr);
			char timeStr[100];
			size_t count = std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d_%H-%M-%S", std::localtime(&t));
			m_filepath += std::string(timeStr);
		}

		m_filepath.replace_extension(ImageFormatDictio.at(m_format));
		controller.updateInfo(new GuiDataGenerateHDImage(m_imageSize, 1, m_format, m_viewport, m_filepath, m_metadata, m_showProgressBar, m_hdimagetilesize));
	}

	bool SetupImageHD::canUndo() const
	{
		return false;
	}

	void SetupImageHD::undoFunction(Controller& controller)
	{}

	ControlType SetupImageHD::getType() const
	{
		return (ControlType::setupImageHD);
	}

	/*
	** GenerateVideoHD
	*/

	GenerateVideoHD::GenerateVideoHD(std::filesystem::path videoPath, const VideoExportParameters& videoParams)
		: m_videoPath(videoPath)
		, m_videoParams(videoParams)
	{}

	GenerateVideoHD::~GenerateVideoHD()
	{}

	void GenerateVideoHD::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::exportVideoHD);
		FilesMessage fmessage({ m_videoPath });
		controller.getFunctionManager().feedMessage(controller, &fmessage);
		VideoExportParametersMessage pmessage(m_videoParams);
		controller.getFunctionManager().feedMessage(controller, &pmessage);
	}

	ControlType GenerateVideoHD::getType() const
	{
		return ControlType::generateVideoHD;
	}

	/*
	** ImportScanModifications
	*/

	ImportScantraModifications::ImportScantraModifications(std::filesystem::path sqliteDbPath)
		: m_sqliteDbPath(sqliteDbPath)
	{}

	ImportScantraModifications::~ImportScantraModifications()
	{}

	void ImportScantraModifications::doFunction(Controller& controller)
	{
		if (m_sqliteDbPath.empty())
			return;

		std::unordered_set<SafePtr<AGraphNode>> toActualizeNodes;
		std::vector<std::string> scanNotFounds;

		try
		{
			SQLite::Database  db(m_sqliteDbPath);

			std::unordered_map<std::string, SafePtr<AGraphNode>> scansMap;
			for (const SafePtr<AGraphNode>& node : controller.getGraphManager().getNodesByTypes({ ElementType::Scan }))
			{
				ReadPtr<AGraphNode> rNode = node.cget();
				if (!rNode)
					continue;
				scansMap[Utils::to_utf8(rNode->getName())] = node;
			}

			//Recalage
			{
				SQLite::Statement   rq(db, "SELECT * FROM Results");

				std::string refScanName = "";

				while (rq.executeStep())
				{
					std::string toModifScanName = rq.getColumn("StationID_1");
					if (scansMap.find(toModifScanName) == scansMap.end())
					{
						scanNotFounds.push_back(toModifScanName);
						continue;
					}

					/*
					std::string tempRefScanName = query.getColumn("StationID_2");
					if (refScanName != tempRefScanName)
					{
						if (scansMap.find(tempRefScanName) == scansMap.end())
						{
							scanNotFounds.push_back(tempRefScanName);
							continue;
						}
						ReadPtr<AGraphNode> rNode = scansMap.at(tempRefScanName).cget();
						if (!rNode)
							continue;
						refTransfoMat = rNode->getTransformation();
						refScanName = tempRefScanName;
					}
					*/

					glm::dquat quat(rq.getColumn("q0"), rq.getColumn("qx"), rq.getColumn("qy"), rq.getColumn("qz"));
					glm::dvec3 tr(rq.getColumn("tx"), rq.getColumn("ty"), rq.getColumn("tz"));

					SafePtr<AGraphNode> scanToModif = scansMap[toModifScanName];
					{
						WritePtr<AGraphNode> wScan = scanToModif.get();
						if (!wScan)
							continue;
						wScan->setPosition(tr);
						wScan->setRotation(quat);
					}

					toActualizeNodes.insert(scanToModif);
				}
			}

			//Clusters
			{
				std::unordered_map<int, SafePtr<ClusterNode>> newClusters;

				SQLite::Statement   cq(db, "SELECT * FROM StationGroups");

				while (cq.executeStep())
				{
					int clusterId = cq.getColumn("GroupID");
					std::string clusterName = cq.getColumn("Caption");
					std::string clusterDesc = cq.getColumn("Description");

					SafePtr<ClusterNode> clusterNode = make_safe<ClusterNode>();
					WritePtr<ClusterNode> wClusterNode = clusterNode.get();
					
					wClusterNode->setTreeType(TreeType::Scan);
					wClusterNode->setName(Utils::from_utf8(clusterName));

					newClusters[clusterId] = clusterNode;
				}

				SQLite::Statement   sq(db, "SELECT * FROM Stations");

				std::unordered_set<SafePtr<AGraphNode>> toAddNodes;
				std::unordered_set<SafePtr<AGraphNode>> toActualizeNodes;
				while (sq.executeStep())
				{
					std::string scanName = sq.getColumn("StationID");
					int clusterId = sq.getColumn("GroupID");

					if (scansMap.find(scanName) == scansMap.end())
						continue;

					if (newClusters.find(clusterId) == newClusters.end())
						continue;

					SafePtr<ClusterNode> cluster = newClusters[clusterId];
					SafePtr<AGraphNode> scan = scansMap[scanName];
					AGraphNode::addOwningLink(cluster, scan);
					toAddNodes.insert(cluster);
					toActualizeNodes.insert(cluster);
					toActualizeNodes.insert(scan);

				}

				controller.getGraphManager().addNodesToGraph(toAddNodes);
				controller.actualizeTreeView(toActualizeNodes);
			}


		}
		catch (...)
		{
			controller.updateInfo(new GuiDataWarning(TEXT_ERROR));
		}

		if (!scanNotFounds.empty())
		{
			const ProjectInternalInfo& internalInfo = controller.getContext().cgetProjectInternalInfo();
			Utils::System::createDirectoryIfNotExist(internalInfo.getReportsFolderPath());

			time_t timeNow = std::time(nullptr);
			wchar_t strDate[256];
			std::wcsftime(strDate, sizeof(strDate), WIDE_SERIALIZE_TIME_FORMAT, std::localtime(&timeNow));

			std::string filename = "scantra_registration_not_found_scans_" + Utils::to_utf8(strDate) + ".csv";

			CSVWriter writer = CSVWriter(internalInfo.getReportsFolderPath() / filename, "\t");
			for (std::string scanName : scanNotFounds)
				writer << Utils::from_utf8(scanName) << CSVWriter::endl;
			writer.close();

			controller.updateInfo(new GuiDataWarning(TEXT_SCANTRA_NOT_FOUND_SCANS));
		}

		controller.getContext().setIsCurrentProjectSaved(false);

	}

	ControlType ImportScantraModifications::getType() const
	{
		return ControlType::importScantraModifications;
	}

	/*
	** ConvertImageToPointCloud
	*/

	ConvertImageToPointCloud::ConvertImageToPointCloud(const ConvertImage& params)
		: m_params(params)
		, m_ptsWriteScan()
	{}

	ConvertImageToPointCloud::~ConvertImageToPointCloud()
	{
		m_ptsWriteScan.close();
	}

	void ConvertImageToPointCloud::doFunction(Controller& controller)
	{
		const QImage& image = m_params.inputImage;

		m_ptsWriteScan.open(m_params.outputPath);

		glm::vec3 nextPos = m_params.origin;
		char separator = ' ';

		float step = m_params.length / image.width();
		int size = image.width() * image.height();

		controller.updateInfo(new GuiDataProcessingSplashScreenStart(size, TEXT_EXPORT, TEXT_SPLASH_SCREEN_PIXEL_PROCESSING.arg(0).arg(size)));
		controller.updateInfo(new GuiDataProcessingSplashScreenEnableCancelButton(false));

		int progress = 0;

		float& axeX = m_params.normalAxeMode == 0 ? nextPos.x : 
						m_params.normalAxeMode == 1 ? nextPos.z : nextPos.y;
		float& axeY = m_params.normalAxeMode == 0 ? nextPos.y :
						m_params.normalAxeMode == 1 ? nextPos.x : nextPos.z;

		uint32_t decimalsNumber = std::abs(std::log10(step)) + 1;;
		for (int y = 0; y < image.height(); y++)
		{
			for (int x = 0; x < image.width(); x++)
			{
				QRgb pix = image.pixel(x, image.height() - y - 1);

				Color32 color;
				if (qAlpha(pix) != 255)
				{
					switch (m_params.colorTransparencyMode)
					{
						case 0:
						{
							axeX += step;
							continue;
						}
						break;
						case 1:
						{
							color = Color32(255, 255, 255);
						}
						break;
						case 2:
						{
							color = Color32(0, 0, 0);
						}
						break;
					}
				}
				else
					color = Color32(qRed(pix), qGreen(pix), qBlue(pix));

				std::string pointStr;
				pointStr += Utils::roundFloat(nextPos.x, decimalsNumber) + separator;
				pointStr += Utils::roundFloat(nextPos.y, decimalsNumber) + separator;
				pointStr += Utils::roundFloat(nextPos.z, decimalsNumber) + separator;
				pointStr += std::to_string(color.r) + separator;
				pointStr += std::to_string(color.g) + separator;
				pointStr += std::to_string(color.b);

				pointStr += "\n";
				m_ptsWriteScan << pointStr;

				axeX += step;
			}
			axeY += step;
			axeX = 0;
			progress += image.width();
			controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(TEXT_SPLASH_SCREEN_PIXEL_PROCESSING.arg(progress).arg(size), progress));
		}

		m_ptsWriteScan.close();

		controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));
		controller.updateInfo(new GuiDataOpenInExplorer(m_params.outputPath));
	}

	ControlType ConvertImageToPointCloud::getType() const
	{
		return ControlType::convertImage;
	}
}