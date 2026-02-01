#define FILTERED_FONCTIONNALITIES_H_	//Reverse line to activate file, currently desactivate because we switch open source
#ifndef FILTERED_FONCTIONNALITIES_H_  


#include "controller/controls/IControl.h"
#include "models/LicenceTypes.h"
#include <unordered_map>
#include <unordered_set>
#include <stdint.h>

static const std::unordered_map<ControlType, std::unordered_set<LicenceVersion>> FilteredFonctionnalitesMap =
{
	// control::application
	{ControlType::quitApplication, {LicenceVersion::Free}}, 
	{ControlType::quitFromLicenceApplication, {LicenceVersion::Free}}, 
	{ControlType::resetLicenceApplication, {LicenceVersion::Free}}, 
	{ControlType::redoApplication, {LicenceVersion::Free}}, 
	{ControlType::undoApplication, {LicenceVersion::Free}}, 

	// control::meta
	{ControlType::metaControl, {LicenceVersion::Free}},
	{ControlType::startMetaControl, {LicenceVersion::Free}},
	{ControlType::stopMetaControl, {LicenceVersion::Free}},

	// control::application::author
	{ControlType::temporaryPath, {LicenceVersion::Free}}, 

	// control::application
	{ControlType::saveAuthors, {LicenceVersion::Standard}},
	{ControlType::selectAuthor, {LicenceVersion::Free}},
	{ControlType::createAuthor, {LicenceVersion::Free}},
	{ControlType::deleteAuthor, {LicenceVersion::Free}},
	{ControlType::sendAuthorList, {LicenceVersion::Free}},
	{ControlType::setLangage, {LicenceVersion::Free}},
	{ControlType::setMouseWheel, {LicenceVersion::Free}},
	{ControlType::setDigitsDisplay, {LicenceVersion::Free}},
	{ControlType::setProjectsFolder, {LicenceVersion::Free}},
	{ControlType::setTemporaryFolder, {LicenceVersion::Free}},
	{ControlType::setUserColor, {LicenceVersion::Free}},
	{ControlType::setDecimationOptions, {LicenceVersion::Free}},
	{ControlType::setOctreePrecision, {LicenceVersion::Free}},
	{ControlType::setRenderPointSize, {LicenceVersion::Free}},
	{ControlType::setExamineOptions, {LicenceVersion::Free}},
	{ControlType::setFramelessMode, {LicenceVersion::Free}},
	{ControlType::setGizmoParameters, {LicenceVersion::Free}},
	{ControlType::setManipulatorSize, {LicenceVersion::Free}},
	{ControlType::setExamineDisplayMode, {LicenceVersion::Free}},
	{ControlType::setRecentProjects, {LicenceVersion::Free}},
	{ControlType::sendRecentProjects, {LicenceVersion::Free}},
	{ControlType::setAutosaveParameters, {LicenceVersion::Free}},
	{ControlType::setIndexationMethod, {LicenceVersion::Free}},
	{ControlType::setMultithreadingOptions, {LicenceVersion::Free}},
	{ControlType::setValueSettingsDisplay, {LicenceVersion::Free}},
	{ControlType::setNavigationParameters, {LicenceVersion::Free}},
	{ControlType::setPerspectiveZBounds, {LicenceVersion::Free}},
	{ControlType::setOrthoGridParams, {LicenceVersion::Free}},
	{ControlType::unlockScanManipulation, {LicenceVersion::Standard}},

	// control::project
	{ControlType::createProject, {LicenceVersion::Standard}},
	{ControlType::dropLoadProject, {LicenceVersion::Free}},
	{ControlType::loadProject, {LicenceVersion::Free}},
	{ControlType::saveProjectContext, {LicenceVersion::Free}},
	{ControlType::saveProject, {LicenceVersion::Free}},
	{ControlType::autosaveProject, {LicenceVersion::Free}},
	{ControlType::closeProject, {LicenceVersion::Free}},
	{ControlType::editProject, {LicenceVersion::Free}},
	{ControlType::saveCreateProject, {LicenceVersion::Standard}},
	{ControlType::saveCloseProject, {LicenceVersion::Free}},
	{ControlType::saveCloseLoadProject, {LicenceVersion::Free}},
	{ControlType::saveCloseLoadProjectCentral, {LicenceVersion::Free}},
	{ControlType::saveQuitProject, {LicenceVersion::Free}},
	{ControlType::functionImportScanProject, {LicenceVersion::Standard}},
	{ControlType::importScanProject, {LicenceVersion::Standard}},
	{ControlType::applyProjectTransformation, {LicenceVersion::Free}},
	{ControlType::applyUserTransformation, {LicenceVersion::Free}},
	{ControlType::deleteScanProject, {LicenceVersion::Standard}},
	{ControlType::showPropertiesProject, {LicenceVersion::Free}},
	{ControlType::importTemperatureScale, {LicenceVersion::Free}},

	// control::projectTemplate
	{ControlType::createProjectTemplate, {LicenceVersion::Standard}},
	{ControlType::loadProjectTemplate, {LicenceVersion::Standard}},
	{ControlType::saveProjectTemplate, {LicenceVersion::Standard}},
	{ControlType::renameProjectTemplate, {LicenceVersion::Standard}},
	{ControlType::deleteProjectTemplate, {LicenceVersion::Standard}},
	{ControlType::sendListProjectTemplate, {LicenceVersion::Standard}},
	{ControlType::stopEditionProjectTemplate, {LicenceVersion::Standard}},

	// control::modal
	{ControlType::ModalReturnValue, {LicenceVersion::Free}},
	{ControlType::ModalReturnFiles, {LicenceVersion::Free}}, 

	// control::io
	{ControlType::exportSubProject, {LicenceVersion::Standard}}, 
	{ControlType::itemsToDXFIO, {LicenceVersion::Standard}},
	{ControlType::itemsToCSVIO, {LicenceVersion::Standard}},
	{ControlType::itemsToStepIO, {LicenceVersion::Standard}},
	{ControlType::itemsToOSToolsIO, {LicenceVersion::Standard}},
	{ControlType::itemsToObjIO, {LicenceVersion::Standard}},
	{ControlType::itemsToFbxIO, {LicenceVersion::Standard}},
	{ControlType::importOSTObjects, {LicenceVersion::Standard}},
	{ControlType::linkOSTObjectsContext, {LicenceVersion::Standard}},
	{ControlType::refreshScanLink, {LicenceVersion::Free}},
	{ControlType::setupImageHD, {LicenceVersion::Standard}},
	{ControlType::generateVideoHD, {LicenceVersion::Standard}},
	{ControlType::importScanTraModifications, {LicenceVersion::Standard}},

	// control::exportPC
	{ControlType::startExport, {LicenceVersion::Standard}}, 
	{ControlType::startExportPCO, {LicenceVersion::Standard}},
	{ControlType::startSingleExport, {LicenceVersion::Standard}}, 
	{ControlType::startGridExport, {LicenceVersion::Standard}},
	{ControlType::startDeletePoints, {LicenceVersion::Standard}},
	{ControlType::startStatisticalOutlierFilter, {LicenceVersion::Standard}},

	// control::function
	{ControlType::functionAbort, {LicenceVersion::Free}}, 
	{ControlType::abortSelection, {LicenceVersion::Free}}, 
	{ControlType::functionCancel, {LicenceVersion::Free}}, 
	{ControlType::forwardMessage, {LicenceVersion::Free}}, 
	{ControlType::addNode, {LicenceVersion::Free}}, 

	// control::userlist
	{ControlType::sendUserLists, {LicenceVersion::Free} },
	{ControlType::createUserList, {LicenceVersion::Standard}},
	{ControlType::deleteUserList, {LicenceVersion::Standard}},
	{ControlType::duplicateUserList, {LicenceVersion::Standard} },
	{ControlType::renameUserList, {LicenceVersion::Standard} },
	{ControlType::sendInfoUserList, {LicenceVersion::Standard} },
	{ControlType::addItemUserList, {LicenceVersion::Standard} },
	{ControlType::removeItemUserList, {LicenceVersion::Standard} },
	{ControlType::renameItemUserList, {LicenceVersion::Standard} },
	{ControlType::clearUserList, {LicenceVersion::Standard} },
	{ControlType::saveLists, {LicenceVersion::Standard} },
	{ControlType::reloadLists, {LicenceVersion::Standard} },
	{ControlType::importList, {LicenceVersion::Standard} },
	{ControlType::exportList, {LicenceVersion::Standard} },

	// control::standards
	{ControlType::sendStandards, {LicenceVersion::Free} },
	{ControlType::selectStandard, {LicenceVersion::Standard} },
	{ControlType::createStandard, {LicenceVersion::Standard} },
	{ControlType::deleteStandard, {LicenceVersion::Standard} },
	{ControlType::duplicateStandard, {LicenceVersion::Standard} },
	{ControlType::renameStandard, {LicenceVersion::Standard} },
	{ControlType::sendInfoStandard, {LicenceVersion::Standard} },
	{ControlType::addItemStandard, {LicenceVersion::Standard} },
	{ControlType::removeItemStandard, {LicenceVersion::Standard} },
	{ControlType::renameItemStandard, {LicenceVersion::Standard} },
	{ControlType::clearStandard, {LicenceVersion::Standard} },
	{ControlType::saveStandards, {LicenceVersion::Standard} },
	{ControlType::reloadStandards, {LicenceVersion::Standard} },
	{ControlType::importStandard, {LicenceVersion::Standard} },
	{ControlType::exportStandard, {LicenceVersion::Standard} },

	// control::tagTemplate
	{ControlType::sendTemplateList, {LicenceVersion::Free} },
	{ControlType::sendTagTemplate, {LicenceVersion::Free} },
	{ControlType::createTagTemplate, {LicenceVersion::Standard} },
	{ControlType::deleteTagTemplate, {LicenceVersion::Standard} },
	{ControlType::duplicateTagTemplate, {LicenceVersion::Standard} },
	{ControlType::renameTagTemplate, {LicenceVersion::Standard} },
	{ControlType::exportTagTemplate, {LicenceVersion::Standard} },
	{ControlType::importTagTemplate, {LicenceVersion::Standard} },
	{ControlType::saveTemplate, {LicenceVersion::Standard} },
	{ControlType::createFieldTemplate, {LicenceVersion::Standard} },
	{ControlType::deleteFieldTemplate, {LicenceVersion::Standard} },
	{ControlType::renameFieldTemplate, {LicenceVersion::Standard} },
	{ControlType::changeTypeFieldTemplate, {LicenceVersion::Standard} },
	{ControlType::changeRefFieldTemplate, {LicenceVersion::Standard} },
	{ControlType::changeDefaultValueFieldTemplate, {LicenceVersion::Standard} },

	// control::filter
	/*{ControlType::keywordFilter, {LicenceVersion::Standard}},
	{ControlType::showHideMarkers, {LicenceVersion::Free} },
	{ControlType::MarkersSoftFilter, {LicenceVersion::Standard} },
	{ControlType::addColorFilter, {LicenceVersion::Standard} },
	{ControlType::removeColorFilter, {LicenceVersion::Standard} },
	{ControlType::clearcolorFilter, {LicenceVersion::Standard} },
	{ControlType::setTimeMinFilter, {LicenceVersion::Standard} },
	{ControlType::setTimeMaxFilter, {LicenceVersion::Standard} },
	{ControlType::setIconFilter, {LicenceVersion::Standard} },
	{ControlType::setIconStatusFilter, {LicenceVersion::Standard} },
	{ControlType::setKeyworStatusFilter, {LicenceVersion::Standard} },
	{ControlType::setUserStatusFilter, {LicenceVersion::Standard} },
	{ControlType::setDisciplineStatusFilter, {LicenceVersion::Standard} },
	{ControlType::setPhaseStatusFilter, {LicenceVersion::Standard} },
	{ControlType::setTimeStatusFilter, {LicenceVersion::Standard} },
	{ControlType::setUserFilter, {LicenceVersion::Standard} },
	{ControlType::setDisciplineFilter, {LicenceVersion::Standard} },
	{ControlType::setPhaseFilter, {LicenceVersion::Standard} },*/

	// control::function::tag
	{ControlType::activateCreateFunctionTag, {LicenceVersion::Free} },
	{ControlType::activateMoveFunctionTag, {LicenceVersion::Standard} },
	{ControlType::activateDuplicateFunctionTag, {LicenceVersion::Standard} },
	{ControlType::setDefaultColorFunctionTag, {LicenceVersion::Standard} },
	{ControlType::setCurrentTagTemplate, {LicenceVersion::Standard} },
	{ControlType::setDefaultActionFunctionTag, {LicenceVersion::Standard} },
	{ControlType::setDefaultCategoryFunctionTag, {LicenceVersion::Standard} },
	{ControlType::setDefaultIconFunctionTag, {LicenceVersion::Standard} },
	{ControlType::duplicateTag, {LicenceVersion::Standard} },

	// control::function::clipping
	{ControlType::activateCreateLocalFunctionClipping, {LicenceVersion::Standard} },
	{ControlType::activateCreateAttachedBox3Points, {LicenceVersion::Standard} },
	{ControlType::activateCreateAttachedBox2Points, {LicenceVersion::Standard} },
	{ControlType::createGlobalFunctionClipping, {LicenceVersion::Standard} },
	{ControlType::setDefaultSizeFunctionClipping, {LicenceVersion::Standard} },
	{ControlType::setDefaultOffsetFunctionClipping, {LicenceVersion::Standard} },
	{ControlType::activateDuplicateFunctionClipping, {LicenceVersion::Standard} },
	{ControlType::setClippingAlignementValue, {LicenceVersion::Standard} },

	// control::duplication
	{ControlType::setDefaultDuplicationModeFunction, {LicenceVersion::Standard} },
	{ControlType::setDefaultDuplicationStepSizeFunction, {LicenceVersion::Standard} },
	{ControlType::setDefaultDuplicationOffsetFunction, {LicenceVersion::Standard} },
	{ControlType::setDefaultDuplicationIsLocalFunction, {LicenceVersion::Standard} },

	// control::function::torus
	{ControlType::disconnectPiping, {LicenceVersion::Standard} },

	// control::function::measure
	{ControlType::addMeasureToPolylineMeasure, {LicenceVersion::Free} },
	{ ControlType::setPolylineOptions, {LicenceVersion::Standard} },

	// control::special
	{ControlType::deleteElementSpecial, {LicenceVersion::Free} },
	{ControlType::deleteSelectedElementSpecial, {LicenceVersion::Free} },
	{ControlType::deleteTotal, {LicenceVersion::Free} },
	{ControlType::showhideCurrentMarkers, {LicenceVersion::Free} },
	{ControlType::showhideUncurrentMarkers, {LicenceVersion::Free} },
	{ControlType::showhideAll, {LicenceVersion::Free} },
	{ControlType::showhideDatas, {LicenceVersion::Free} },
	{ControlType::showHideObjects, {LicenceVersion::Free} },

	// control::picking
	{ControlType::clickPicking, {LicenceVersion::Free} },
	{ControlType::findScanFromPicking, {LicenceVersion::Standard} },
	{ControlType::pickTemperatureFromPicking, {LicenceVersion::Standard} },

	// control::viewport
	{ControlType::multiSelect, {LicenceVersion::Free} },
	{ControlType::examine, {LicenceVersion::Free} },
	{ControlType::changeBackgroundColor, {LicenceVersion::Free} },
	{ControlType::alignViewSide, {LicenceVersion::Free} },
	{ControlType::alignView2PointsFunction, {LicenceVersion::Standard} },
	{ControlType::alignView3PointsFunction, {LicenceVersion::Standard} },
	{ControlType::alignViewBoxFunction, {LicenceVersion::Standard} },
	{ControlType::moveManipFunction, {LicenceVersion::Standard} },
	{ControlType::renderModeUpdate, {LicenceVersion::Free} },
	{ControlType::changeScanVisibility, {LicenceVersion::Free} },
	{ControlType::quickScreenshot, {LicenceVersion::Free} },
	{ControlType::quickVideo, {LicenceVersion::Standard} },
	{ControlType::recordPerformance, {LicenceVersion::Free} },

	// control::tree
	{ControlType::createClusterTree, {LicenceVersion::Standard} },
	{ControlType::deleteTree, {LicenceVersion::Standard} },
	{ControlType::clearAndDeleteCluster, {LicenceVersion::Standard} },
	{ControlType::removeFromHierarchy, {LicenceVersion::Standard} },
	{ControlType::childrenSelection, {LicenceVersion::Free} },
	{ControlType::changeHideData, {LicenceVersion::Free} },
	{ControlType::DropElement, {LicenceVersion::Standard} },
	{ControlType::CopySelectedTo, {LicenceVersion::Standard} },

	// control::dataEdition
	{ControlType::setColorEdit, {LicenceVersion::Standard} },
	{ControlType::setUserIdEdit, {LicenceVersion::Standard} },
	{ControlType::setDescriptionEdit, {LicenceVersion::Standard} },
	{ControlType::setDisciplineEdit, {LicenceVersion::Standard} },
	{ControlType::setHyperLinks, {LicenceVersion::Standard}},
	{ControlType::setNameEdit, {LicenceVersion::Standard} },
	{ControlType::setPhaseEdit, {LicenceVersion::Standard} },
	{ControlType::setIdentifierEdit, {LicenceVersion::Standard} },
	{ControlType::addHyperLink, {LicenceVersion::Standard} },
	{ControlType::removeHyperLink, {LicenceVersion::Standard} },

	// control::attributesEdition
	{ControlType::setColorAttribute, {LicenceVersion::Free} },
	{ControlType::setDisciplineAttribute, {LicenceVersion::Free} },
	{ControlType::setNameAttribute, {LicenceVersion::Free} },
	{ControlType::setPhaseAttribute, {LicenceVersion::Free} },
	{ControlType::setIdentifierAttribute, {LicenceVersion::Free} },

	// control::tagEdition
	{ControlType::setFieldDatatagEdit, {LicenceVersion::Standard} },
	{ControlType::setShapeTagEdit, {LicenceVersion::Standard} },

	// control::clippingEdition
	{ControlType::setClippingTypeEdit, {LicenceVersion::Standard} },
	{ControlType::setClippingModeEdit, {LicenceVersion::Standard} },
	{ControlType::setMinClipDistance, {LicenceVersion::Standard} },
	{ControlType::setMaxClipDistance, {LicenceVersion::Standard} },
	{ControlType::setClippingActiveEdit, {LicenceVersion::Standard} },
	{ControlType::disableAllActiveClipping, {LicenceVersion::Standard} },
	{ControlType::setDefaultMinClipDistance, {LicenceVersion::Standard} },
	{ControlType::setDefaultMaxClipDistance, {LicenceVersion::Standard} },
	{ControlType::setClippingUserIdEdit, {LicenceVersion::Standard} },
	{ControlType::setClippingBoxExtend, {LicenceVersion::Standard} },
	{ControlType::setGridType, {LicenceVersion::Standard} },
	{ControlType::setGridValue, {LicenceVersion::Standard} },
	{ControlType::setDefaultClipMode, {LicenceVersion::Standard} },
	{ControlType::setDefaultRampValues, {LicenceVersion::Standard} },

	{ControlType::activeRamp, {LicenceVersion::Standard} },
	{ControlType::setMaxRamp, {LicenceVersion::Standard} },
	{ControlType::setMinRamp, {LicenceVersion::Standard} },
	{ControlType::setRampSteps, {LicenceVersion::Standard} },
	{ControlType::setRampClamped, {LicenceVersion::Standard} },

	// control::pointEdition
	{ControlType::setPointPositionEdit, {LicenceVersion::Standard} },

	// control::objectEdition
	{ControlType::setObject3DCenterEdit, {LicenceVersion::Standard} },
	{ControlType::addObject3DCenterEdit, {LicenceVersion::Standard} },
	{ControlType::setObject3DSizeEdit, {LicenceVersion::Standard} },
	{ControlType::setObject3DRotationEdit, {LicenceVersion::Standard} },
	{ControlType::addObject3DRotationEdit, {LicenceVersion::Standard} },
	{ControlType::setObject3DTransformationEdit, {LicenceVersion::Standard} },
	{ControlType::addObject3DTransformationEnd, {LicenceVersion::Standard} },
	{ControlType::addObject3DExtrudeEdit, {LicenceVersion::Standard} },
	{ControlType::addObject3DExtrudeEditEnd, {LicenceVersion::Standard} },
	{ControlType::setSphereRadius, {LicenceVersion::Standard} },
	{ControlType::launchManipulateContext, {LicenceVersion::Standard} },

	// control::cylinderEdition
	{ControlType::setForcedRadiusEdit, {LicenceVersion::Standard} },
	{ControlType::setDetectedRadiusEdit, {LicenceVersion::Standard} },
	{ControlType::setStandardEdit, {LicenceVersion::Free} },
	{ControlType::setCylinderLengthEdit, {LicenceVersion::Standard} },

	// control::folderEdition
	{ControlType::update3DCluster, {LicenceVersion::Standard} },

	// control::ScanEdition
	{ControlType::setScanClippable, {LicenceVersion::Standard} }, // TODO - Free or Premium control ?
	{ControlType::randomScansColors, {LicenceVersion::Free} },
	{ControlType::changeScanGuid, {LicenceVersion::Free} },

	// control::pcObject
	{ControlType::createPCObjectFromFile, {LicenceVersion::Standard} },
	{ControlType::createPCObjectFromBox, {LicenceVersion::Standard} },
	{ControlType::activateDuplicateFunctionPCO, {LicenceVersion::Standard} },

	// control::meshObject
	{ControlType::createWavefrontFromFile, {LicenceVersion::Standard} },
	{ControlType::activateDuplicateFunctionWavefront, {LicenceVersion::Standard} },
	{ControlType::stepSimplification, {LicenceVersion::Standard} },

	// control::ScanTools
	{ControlType::showConvertionOptions, {LicenceVersion::Standard} },

	// control::SignalHandling
	{ControlType::SIGINTSignalHandling, {LicenceVersion::Free} },
	{ControlType::SIGTERMSignalHandling, {LicenceVersion::Free} },
	{ControlType::SIGILLSignalHandling, {LicenceVersion::Free} },
	{ControlType::SIGABRTSignalHandling, {LicenceVersion::Free} },
	{ControlType::SIGSEGVSignalHandling, {LicenceVersion::Free} },
	{ControlType::SIGFPESignalHandling, {LicenceVersion::Free} },

	// control::animation
	{ControlType::addAnimationKeyPoint, {LicenceVersion::Standard} },
	{ControlType::addScansAnimationKeyPoint, {LicenceVersion::Standard} },

	// control::Measure
	{ControlType::SendPipeDetectionOptions, {LicenceVersion::Free} },
	{ControlType::ActivateFastCylinder, {LicenceVersion::Free} },
	{ControlType::ActivateRobustCylinder, {LicenceVersion::Free} },
	{ControlType::ActivateBeamBending, {LicenceVersion::Standard} },
	{ControlType::ActivateColumnTilt, {LicenceVersion::Standard} },
	{ControlType::SetBeamBendingTolerance, {LicenceVersion::Standard} },
	{ControlType::SetColumnTiltTolerance, {LicenceVersion::Standard} },
	{ControlType::ActivateSimpleMeasure, {LicenceVersion::Free} },
	{ControlType::ActivatePolylineMeasure, {LicenceVersion::Free} },
	{ControlType::ActivateBigCylinderFit, {LicenceVersion::Standard} },
	{ControlType::ActivatePointToCylinderMeasure, {LicenceVersion::Standard} },
	{ControlType::ActivatePointToPlaneMeasure, {LicenceVersion::Standard} },
	{ControlType::ActivatePointToPlane3Measure, {LicenceVersion::Standard} },
	{ControlType::ActivateCylinderToPlaneMeasure, {LicenceVersion::Standard} },
	{ControlType::ActivateCylinderToPlane3Measure, {LicenceVersion::Standard} },
	{ControlType::Switch3PlanMeasure, {LicenceVersion::Standard} },
	{ControlType::ActivateCylinderToCylinderMeasure, {LicenceVersion::Standard} },
	{ControlType::ActivateMultipleCylindersMeasure, {LicenceVersion::Standard} },
	{ControlType::ActivatePointMeasure, {LicenceVersion::Free} },
	{ControlType::ActivatePointCreation, {LicenceVersion::Free} },
	{ControlType::ActivateSphere, {LicenceVersion::Standard} },
	{ControlType::Activate4ClicsSphere, {LicenceVersion::Standard} },
	{ControlType::ActivateExtendCylinder, {LicenceVersion::Standard} },
	{ControlType::ActivatePipeDetectionConnexion, {LicenceVersion::Standard} },
	{ControlType::ActivatePipePostConnexion, {LicenceVersion::Standard} },
	{ControlType::ActivateDetectBeam, {LicenceVersion::Standard} },
	{ControlType::ActivateSlabDetection, {LicenceVersion::Standard} },
	{ControlType::ActivatePointToMeshMeasure, {LicenceVersion::Standard} },
	{ControlType::ActivateSlabMeasure, {LicenceVersion::Standard} },
	{ ControlType::ActivatePlaneConnexion, {LicenceVersion::Standard} },
	{ ControlType::ActivatePlaneDetection, {LicenceVersion::Standard} },
	{ ControlType::ActivatePeopleRemover, {LicenceVersion::Standard} },
	{ ControlType::ActivateSetOfPoints, {LicenceVersion::Standard} },
	{ ControlType::ActivateTorus, {LicenceVersion::Standard} },
	{ ControlType::ActivateTrajectory, {LicenceVersion::Standard} },

	//Control::userOrientation
	{ControlType::userOrientationProperties, {LicenceVersion::Standard} },
	{ControlType::sendUserOrientations, {LicenceVersion::Standard} },
	{ControlType::setUserOrientation, {LicenceVersion::Standard} },
	{ControlType::unsetUserOrientation, {LicenceVersion::Standard} },
	{ControlType::createEditUserOrientation, {LicenceVersion::Standard} },
	{ControlType::deleteUserOrientation, {LicenceVersion::Standard} },

	//Control::viewpoint
	{ControlType::contextViewPointCreation, {LicenceVersion::Standard} },
	{ControlType::contextViewPointUpdate, {LicenceVersion::Standard} },
	{ControlType::updateViewPoint, {LicenceVersion::Standard} },
	{ControlType::updateStateFromViewPoint, {LicenceVersion::Free} },

	//Control::test
	{ControlType::test_autoGenerateData, {LicenceVersion::Free}}
};

enum class FunctionFamily
{
	Clipping,
	Measure,
	Model,
	Import,
	Export,
	Tag,
	Viewpoint,
	UserOrientation
};

static const std::unordered_map<ControlType, FunctionFamily> ControlTypeToFunctionFamily =
{
	{ControlType::activateCreateLocalFunctionClipping, FunctionFamily::Clipping},
	{ControlType::createGlobalFunctionClipping, FunctionFamily::Clipping},

	{ControlType::ActivateSimpleMeasure, FunctionFamily::Measure},
	{ControlType::ActivatePolylineMeasure, FunctionFamily::Measure},
	{ControlType::ActivatePointToPlaneMeasure, FunctionFamily::Measure},
	{ControlType::ActivatePointToPlane3Measure, FunctionFamily::Measure},
	{ControlType::ActivatePointToCylinderMeasure, FunctionFamily::Measure},
	{ControlType::ActivateCylinderToPlaneMeasure, FunctionFamily::Measure},
	{ControlType::ActivateCylinderToPlane3Measure, FunctionFamily::Measure},
	{ControlType::ActivateCylinderToCylinderMeasure, FunctionFamily::Measure},
	{ControlType::ActivateColumnTilt, FunctionFamily::Measure},
	{ControlType::ActivateBeamBending, FunctionFamily::Measure},

	{ControlType::ActivateBigCylinderFit, FunctionFamily::Model},
	{ControlType::ActivateExtendCylinder, FunctionFamily::Model},
	{ControlType::ActivateFastCylinder, FunctionFamily::Model},
	{ControlType::ActivateRobustCylinder, FunctionFamily::Model},
	{ControlType::ActivatePointCreation, FunctionFamily::Model},
	{ControlType::ActivatePipeDetectionConnexion, FunctionFamily::Model},
	{ControlType::ActivatePipePostConnexion, FunctionFamily::Model},
	{ControlType::ActivateSphere, FunctionFamily::Model},
	{ControlType::Activate4ClicsSphere, FunctionFamily::Model},

	{ControlType::createPCObjectFromFile, FunctionFamily::Import},
	{ControlType::createWavefrontFromFile, FunctionFamily::Import},

	{ControlType::itemsToStepIO, FunctionFamily::Export},
	{ControlType::itemsToCSVIO, FunctionFamily::Export},
	{ControlType::itemsToDXFIO, FunctionFamily::Export},
	{ControlType::itemsToOSToolsIO, FunctionFamily::Export},
	{ControlType::itemsToObjIO, FunctionFamily::Export},
	{ControlType::itemsToFbxIO, FunctionFamily::Export},

	{ControlType::activateCreateFunctionTag, FunctionFamily::Tag},
	{ControlType::activateDuplicateFunctionTag, FunctionFamily::Tag},

	{ControlType::contextViewPointCreation, FunctionFamily::Viewpoint},
	{ControlType::contextViewPointUpdate, FunctionFamily::Viewpoint},

	{ControlType::createEditUserOrientation, FunctionFamily::UserOrientation}

};
#endif // FILTERED_FONCTIONNALITIES_H_
