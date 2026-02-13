#include "controller/functionSystem/ContextFactory.h"

#include "controller/functionSystem/ContextCreateTag.h"
#include "controller/functionSystem/ContextDuplicateTag.h"
#include "controller/functionSystem/ContextMoveTag.h"
#include "controller/functionSystem/ContextImportScan.h"
#include "controller/functionSystem/ContextViewPointAnimation.h"
#include "controller/functionSystem/ContextBeamBending.h"
#include "controller/functionSystem/ContextFitCylinder.h"
#include "controller/functionSystem/ContextSaveCloseLoadProject.h"
#include "controller/functionSystem/ContextExamine.h"
#include "controller/functionSystem/ContextColumnTilt.h"
#include "controller/functionSystem/ContextSimpleMeasure.h"
#include "controller/functionSystem/ContextSimpleMeasure.h"
#include "controller/functionSystem/ContextPoint.h"
#include "controller/functionSystem/ContextBigCylinderFit.h"
#include "controller/functionSystem/ContextPointToCylinderMeasure.h"
#include "controller/functionSystem/ContextClippingBoxCreation.h"
#include "controller/functionSystem/ContextBoxDuplication.h"
#include "controller/functionSystem/ContextPCODuplication.h"
#include "controller/functionSystem/ContextStepSimplification.h"
#include "controller/functionSystem/ContextMeshObjectDuplication.h"
#include "controller/functionSystem/ContextPointToPlaneMeasure.h"
#include "controller/functionSystem/ContextPointToPlane3Measure.h"
#include "controller/functionSystem/ContextCylinderToPlaneMeasure.h"
#include "controller/functionSystem/ContextCylinderToPlane3Measure.h"
#include "controller/functionSystem/ContextCylinderToCylinderMeasure.h"
#include "controller/functionSystem/ContextExtendCylinderMeasure.h"
#include "controller/functionSystem/ContextDeletePoints.h"
#include "controller/functionSystem/ContextStatisticalOutlierFilter.h"
#include "controller/functionSystem/ContextColorBalanceFilter.h"
#include "controller/functionSystem/ContextColorimetricFilterExport.h"
#include "controller/functionSystem/ContextMultipleCylindersMeasure.h"
#include "controller/functionSystem/ContextPointCreation.h"
#include "controller/functionSystem/Context4ClicsSphere.h"
#include "controller/functionSystem/ContextImportOSTObjects.h"
#include "controller/functionSystem/ContextAlignView2P.h"
#include "controller/functionSystem/ContextAlignView3P.h"
#include "controller/functionSystem/ContextAlignViewBox.h"
#include "controller/functionSystem/ContextBoxAttachedCreation.h"
#include "controller/functionSystem/ContextDeleteTag.h"
#include "controller/functionSystem/ContextTemplateModification.h"
#include "controller/functionSystem/ContextTemplateListModification.h"
#include "controller/functionSystem/ContextBeamDetection.h"
#include "controller/functionSystem/ContextSphere.h"
#include "controller/functionSystem/ContextSlabDetection.h"
#include "controller/functionSystem/ContextSlab1Click.h"
#include "controller/functionSystem/ContextPipeDetectionConnexion.h"
#include "controller/functionSystem/ContextPipePostConnexion.h"
#include "controller/functionSystem/ContextViewPoint.h"
#include "controller/functionSystem/mesh/ContextMeshDistance.h"
#include "controller/functionSystem/ContextPlaneConnexion.h"
#include "controller/functionSystem/ContextDataGeneration.h"
#include "controller/functionSystem/ContextPeopleRemover.h"
#include "controller/functionSystem/ContextFindScan.h"
#include "controller/functionSystem/ContextPickTemperature.h"
#include "controller/functionSystem/ContextPickColorimetric.h"
#include "controller/functionSystem/ContextSetOfPoints.h"
#include "controller/functionSystem/ContextPlaneDetection.h"
#include "controller/functionSystem/ContextFitTorus.h"
#include "controller/functionSystem/ContextExportVideoHD.h"
#include "controller/functionSystem/ContextTrajectory.h"
#include "controller/functionSystem/ContextMoveManip.h"
#include "controller/functionSystem/ContextManipulateObjects.h"
#include "controller/functionSystem/ContextPolygonalSelector.h"


#ifndef PORTABLE
#include "controller/functionSystem/ContextMeshObjectCreation.h"
#include "controller/functionSystem/ContextConvertionScan.h"
#include "controller/functionSystem/ContextExportDxf.h"
#include "controller/functionSystem/ContextExportCSV.h"
#include "controller/functionSystem/ContextExportStep.h"
#include "controller/functionSystem/ContextExportOSTObjects.h"
#include "controller/functionSystem/ContextExportObj.h"
#include "controller/functionSystem/ContextExportFbx.h"
#include "controller/functionSystem/ContextExportPC.h"

#include "controller/functionSystem/ContextPCOCreation.h"
#endif

ContextFactory::ContextFactory()
{}

ContextFactory::~ContextFactory()
{}

AContext* ContextFactory::createContext(const ContextType& type, ContextId& id, const ContextId& parent)
{
	switch (type)
	{
#ifndef PORTABLE
		case ContextType::pointCloudObjectCreation:
			return new ContextPCOCreation(id);
		case ContextType::exportPC:
			return new ContextExportPC(id);
		case ContextType::exportSubProject:
			return new ContextExportSubProject(id);
		case ContextType::exportDxf:
			return new ContextExportDxf(id);
		case ContextType::exportCSV:
			return new ContextExportCSV(id);
		case ContextType::exportStep:
			return new ContextExportStep(id);
		case ContextType::exportOpenScanTools:
			return new ContextExportOSTObjects(id);
		case ContextType::exportObj:
			return new ContextExportObj(id);
		case ContextType::exportFbx:
			return new ContextExportFbx(id);
		case ContextType::scanConversion:
			return new ContextConvertionScan(id);
		case ContextType::meshObjectCreation:
			return new ContextMeshObjectCreation(id);
#endif
		case ContextType::tagCreation:
			return new ContextCreateTag(id);
		case ContextType::tagMove:
			return new ContextMoveTag(id);
		case ContextType::tagDuplication:
			return new ContextDuplicateTag(id);
		case ContextType::tagDeletion:
			return new ContextDeleteTag(id);
		case ContextType::templateModification:
			return new ContextTemplateModification(id);
		case ContextType::templateListModification:
			return new ContextTemplateListModification(id);
		case ContextType::scanImport:
			return new ContextImportScan(id);
		case ContextType::viewPointAnimation:
			return new ContextViewPointAnimation(id);
		case ContextType::saveCloseCreateProject:
			return new ContextSaveCloseCreateProject(id);
		case ContextType::saveCloseProject:
			return new ContextSaveCloseProject(id);
		case ContextType::saveCloseLoadProject:
			return new ContextSaveCloseLoadProject(id);

		//new
		//case ContextType::saveCloseLoadProjectCentral:
			//return new ContextSaveCloseLoadProjectCentral(id);

		case ContextType::saveQuitProject:
			return new ContextSaveQuitProject(id);
		case ContextType::saveProject:
			return new ContextSaveProject(id);
		case ContextType::examine:
			return new ContextExamine(id);
		case ContextType::beamBending:
			return new ContextBeamBending(id);
		case ContextType::columnTilt:
			return new ContextColumnTilt(id);
		case ContextType::fitCylinder:
			return new ContextFitCylinder(id);
		case ContextType::simpleMeasure:
			return new ContextSimpleMeasure(id);
		case ContextType::pointMeasure:
			return new ContextPoint(id);
		case ContextType::pointsMeasure:
			return new ContextPointsMeasure(id);
		case ContextType::bigCylinderFit:
			return new ContextBigCylinderFit(id);
		case ContextType::pointToCylinder:
			return new ContextPointToCylinderMeasure(id);
		case ContextType::clippingBoxCreation:
			return new ContextClippingBoxCreation(id);
		case ContextType::clippingBoxAttached2Points:
			return new ContextCreateBoxAttached2Points(id);
		case ContextType::clippingBoxAttached3Points:
			return new ContextCreateBoxAttached3Points(id);
		case ContextType::polygonalSelector:
			return new ContextPolygonalSelector(id);
		case ContextType::boxDuplication:
			return new ContextBoxDuplication(id);
		case ContextType::pointCloudObjectDuplication:
			return new ContextPCODuplication(id);
		case ContextType::stepSimplification:
			return new ContextStepSimplification(id);
		case ContextType::meshObjectDuplication:
			return new ContextMeshObjectDuplication(id);
		case ContextType::meshDistance:
			return new ContextMeshDistance(id);
		case ContextType::pointToPlane:
			return new ContextPointToPlaneMeasure(id);
		case ContextType::pointToPlane3:
			return new ContextPointToPlane3Measure(id);
		case ContextType::cylinderToPlane:
			return new ContextCylinderToPlaneMeasure(id);
		case ContextType::cylinderToPlane3:
			return new ContextCylinderToPlane3Measure(id);
		case ContextType::cylinderToCylinder:
			return new ContextCylinderToCylinderMeasure(id);
		case ContextType::multipleCylinders:
			return new ContextMultipleCylinders(id);
		case ContextType::cylinder2ClickExtend:
			return new ContextExtendCylinder(id);
		case ContextType::deletePoints:
			return new ContextDeletePoints(id);
		case ContextType::statisticalOutlierFilter:
			return new ContextStatisticalOutlierFilter(id);
		case ContextType::colorBalanceFilter:
			return new ContextColorBalanceFilter(id);
		case ContextType::colorimetricFilterExport:
			return new ContextColorimetricFilterExport(id);
		case ContextType::pointCreation:
			return new ContextPointCreation(id);
		case ContextType::Sphere:
			return new ContextSphere(id);
		case ContextType::ClicsSphere4:
			return new Context4ClicsSphere(id);
		case ContextType::importOSTObjects:
			return new ContextImportOSTObjects(id);
		case ContextType::linkFileOSTObjects:
			return new ContextLinkOSTObjects(id);
		case ContextType::alignView2P:
			return new ContextAlignView2P(id);
		case ContextType::alignView3P:
			return new ContextAlignView3P(id);
		case ContextType::alignViewBox:
			return new ContextAlignViewBox(id);
		case ContextType::pipeDetectionConnexion:
			return new ContextPipeDetectionConnexion(id);
		case ContextType::pipePostConnexion:
			return new ContextPipePostConnexion(id);
		case ContextType::beamDetection:
			return new ContextBeamDetection(id);
		case ContextType::Slab2Click:
			return new ContextSlabDetection(id);
		case ContextType::Slab1Click:
			return new ContextSlab1Click(id);
		case ContextType::planeConnexion:
			return new ContextPlaneConnexion(id);
		case ContextType::viewpointCreation:
			return new ContextViewPointCreation(id);
		case ContextType::viewpointUpdate:
			return new ContextViewPointUpdate(id);
		case ContextType::autoGenerateData:
			return new ContextDataGeneration(id);
		case ContextType::peopleRemover:
			return new ContextPeopleRemover(id);
		case ContextType::findScan:
			return new ContextFindScan(id);
		case ContextType::pickTemperature:
			return new ContextPickTemperature(id);
		case ContextType::pickColorimetric:
			return new ContextPickColorimetric(id);
		case ContextType::setOfPoints:
			return new ContextSetOfPoints(id);
		case ContextType::planeDetection:
			return new ContextPlaneDetection(id);
		case ContextType::fitTorus:
			return new ContextFitTorus(id);
		case ContextType::exportVideoHD:
			return new ContextExportVideoHD(id);
		case ContextType::trajectory:
			return new ContextTrajectory(id);
		case ContextType::moveManip:
			return new ContextMoveManip(id);
		case ContextType::manipulateObjects:
			return new ContextManipulateObjects(id);
	}
	assert(false);
	return nullptr;
}
