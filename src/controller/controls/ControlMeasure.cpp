#include "controller/Controls/ControlMeasure.h"
#include "controller/Controller.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/ControllerContext.h"
#include "controller/messages/PipeMessage.h"
#include "controller/messages/SimpleNumberMessage.h"
#include "controller/messages/PlaneMessage.h"
#include "utils/Logger.h"


namespace control::measure
{
	ActivateSimpleMeasure::ActivateSimpleMeasure()
	{}

	ActivateSimpleMeasure::~ActivateSimpleMeasure()
	{}

	void ActivateSimpleMeasure::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::simpleMeasure);
	}

	bool  ActivateSimpleMeasure::canUndo() const
	{
		return (false);
	}

	void ActivateSimpleMeasure::undoFunction(Controller& controller)
	{
		controller.getFunctionManager().abort(controller);
	}

	ControlType ActivateSimpleMeasure::getType() const
	{
		return ControlType::ActivateSimpleMeasure;
	}

	ActivatePolylineMeasure::ActivatePolylineMeasure()
	{}

	ActivatePolylineMeasure::~ActivatePolylineMeasure()
	{}

	void ActivatePolylineMeasure::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::pointsMeasure);
	}

	bool ActivatePolylineMeasure::canUndo() const
	{
		return false;
	}

	void ActivatePolylineMeasure::undoFunction(Controller& controller)
	{}

	ControlType ActivatePolylineMeasure::getType() const
	{
		return ControlType::ActivatePolylineMeasure;
	}

	/*
	** SendMeasuresOptions
	*/

	SendMeasuresOptions::SendMeasuresOptions(const PolyLineOptions& options)
		: m_options(options)
	{}

	SendMeasuresOptions::~SendMeasuresOptions()
	{}

	void SendMeasuresOptions::doFunction(Controller& controller)
	{
		controller.getContext().setPolyLineOptions(m_options);
	}

	bool SendMeasuresOptions::canUndo() const
	{
		return false;
	}
	void SendMeasuresOptions::undoFunction(Controller& controller)
	{}

	ControlType SendMeasuresOptions::getType() const
	{
		return ControlType::setPolylineOptions;
	}

	/*
	** SendPipeDetectionOptions
	*/

	SendPipeDetectionOptions::SendPipeDetectionOptions(const PipeDetectionOptions& options)
		: m_options(options)
	{
	}

	SendPipeDetectionOptions::~SendPipeDetectionOptions()
	{}

	void SendPipeDetectionOptions::doFunction(Controller& controller)
	{
		controller.getContext().setPipeDetectionOptions(m_options);
	}

	bool SendPipeDetectionOptions::canUndo() const
	{
		return false;
	}

	void SendPipeDetectionOptions::undoFunction(Controller& controller)
	{}

	ControlType SendPipeDetectionOptions::getType() const
	{
		return ControlType::SendPipeDetectionOptions;
	}

	/*
	** ActivateFitCylinder
	*/

	ActivateFitCylinder::ActivateFitCylinder()
	{}

	ActivateFitCylinder::~ActivateFitCylinder()
	{}

	void ActivateFitCylinder::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::fitCylinder);
	}

	ControlType ActivateFitCylinder::getType() const
	{
		return ControlType::ActivateFastCylinder;
	}

	////////////////////////////////////////experiment///////////////////////////////////

	ActivateBigCylinderFit::ActivateBigCylinderFit()
	{}

	ActivateBigCylinderFit::~ActivateBigCylinderFit()
	{}

	void ActivateBigCylinderFit::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::bigCylinderFit);
	}

	ControlType ActivateBigCylinderFit::getType() const
	{
		return ControlType::ActivateBigCylinderFit;
	}

	ActivateBeamBending::ActivateBeamBending(const BeamBendingOptions& options):m_options(options)
	{}

	ActivateBeamBending::~ActivateBeamBending()
	{}

	void ActivateBeamBending::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::beamBending);
		BeamBendingMessage* msg = new BeamBendingMessage(m_options);
		controller.getFunctionManager().feedMessage(controller, msg);
	}

	bool  ActivateBeamBending::canUndo() const
	{
		return (false);
	}

	void ActivateBeamBending::undoFunction(Controller& controller)
	{
		controller.getFunctionManager().abort(controller);
	}

	ControlType ActivateBeamBending::getType() const
	{
		return ControlType::ActivateBeamBending;
	}

	/*
	* SetBendingTolerance
	*/

	SetBendingTolerance::SetBendingTolerance(const double& tolerance)
		: m_newTolerance(tolerance)
	{}

	SetBendingTolerance::~SetBendingTolerance()
	{}

	void SetBendingTolerance::doFunction(Controller& controller)
	{
		ProjectInfos& info = controller.getContext().getProjectInfo();
		m_oldTolerance = info.m_beamBendingTolerance;
		info.m_beamBendingTolerance = m_newTolerance;
		CONTROLLOG << "control::measure::SetBendingTolerance do : set" << m_newTolerance << "from" << m_oldTolerance << LOGENDL;
	}

	bool SetBendingTolerance::canUndo() const
	{
		return (true);
	}

	void SetBendingTolerance::undoFunction(Controller& controller)
	{
		ProjectInfos& info = controller.getContext().getProjectInfo();
		info.m_beamBendingTolerance = m_oldTolerance;
		CONTROLLOG << "control::measure::SetBendingTolerance undo : set" << m_oldTolerance << "from" << m_newTolerance << LOGENDL;
	}
			
	ControlType SetBendingTolerance::getType() const
	{
		return (ControlType::SetBeamBendingTolerance);
	}

	/*
	* SetTiltTolerance
	*/

	SetTiltTolerance::SetTiltTolerance(const double& tolerance)
		: m_newTolerance(tolerance)
	{}

	SetTiltTolerance::~SetTiltTolerance()
	{}

	void SetTiltTolerance::doFunction(Controller& controller)
	{
		ProjectInfos& info = controller.getContext().getProjectInfo();
		m_oldTolerance = info.m_columnTiltTolerance;
		info.m_columnTiltTolerance = m_newTolerance;
		CONTROLLOG << "control::measure::SetTiltTolerance do : set" << m_oldTolerance << "from" << m_newTolerance << LOGENDL;
	}

	bool SetTiltTolerance::canUndo() const
	{
		return (true);
	}

	void SetTiltTolerance::undoFunction(Controller& controller)
	{
		ProjectInfos& info = controller.getContext().getProjectInfo();
		info.m_columnTiltTolerance = m_oldTolerance;
		CONTROLLOG << "control::measure::SetTiltTolerance undo : set" << m_oldTolerance << "from" << m_newTolerance << LOGENDL;
	}

	ControlType SetTiltTolerance::getType() const
	{
		return (ControlType::SetColumnTiltTolerance);
	}


	ActivateColumnTilt::ActivateColumnTilt()
	{}

	ActivateColumnTilt::~ActivateColumnTilt()
	{}

	void ActivateColumnTilt::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::columnTilt);
	}

	bool  ActivateColumnTilt::canUndo() const
	{
		return (false);
	}

	void ActivateColumnTilt::undoFunction(Controller& controller)
	{
		controller.getFunctionManager().abort(controller);
	}

	ControlType ActivateColumnTilt::getType() const
	{
		return ControlType::ActivateColumnTilt;
	}

	///////////////////////////////////////////PointToCylinderMeasure//////////////////////////////////
	ActivatePointToCylinderMeasure::ActivatePointToCylinderMeasure()
	{}

	ActivatePointToCylinderMeasure::~ActivatePointToCylinderMeasure()
	{}

	void ActivatePointToCylinderMeasure::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::pointToCylinder);
	}

	bool  ActivatePointToCylinderMeasure::canUndo() const
	{
		return (false);
	}

	void ActivatePointToCylinderMeasure::undoFunction(Controller& controller)
	{
		controller.getFunctionManager().abort(controller);
	}

	ControlType ActivatePointToCylinderMeasure::getType() const
	{
		return ControlType::ActivatePointToCylinderMeasure;
	}

	///////////////////////////////////////////PointToPlaneMeasure//////////////////////////////////
	ActivatePointToPlaneMeasure::ActivatePointToPlaneMeasure()
	{}

	ActivatePointToPlaneMeasure::~ActivatePointToPlaneMeasure()
	{}

	void ActivatePointToPlaneMeasure::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::pointToPlane);
	}

	bool  ActivatePointToPlaneMeasure::canUndo() const
	{
		return (false);
	}

	void ActivatePointToPlaneMeasure::undoFunction(Controller& controller)
	{
		controller.getFunctionManager().abort(controller);
	}

	ControlType ActivatePointToPlaneMeasure::getType() const
	{
		return ControlType::ActivatePointToPlaneMeasure;
	}

	ActivatePointToPlane3Measure::ActivatePointToPlane3Measure()
	{}

	ActivatePointToPlane3Measure::~ActivatePointToPlane3Measure()
	{}

	void ActivatePointToPlane3Measure::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::pointToPlane3);
	}

	bool  ActivatePointToPlane3Measure::canUndo() const
	{
		return false;
	}

	void ActivatePointToPlane3Measure::undoFunction(Controller& controller)
	{
		controller.getFunctionManager().abort(controller);
	}

	ControlType ActivatePointToPlane3Measure::getType() const
	{
		return ControlType::ActivatePointToPlane3Measure;
	}

	/////////////////////////////////////////////CylinderToPlaneMeasure////////////////////////////////////
	ActivateCylinderToPlaneMeasure::ActivateCylinderToPlaneMeasure()
	{}

	ActivateCylinderToPlaneMeasure::~ActivateCylinderToPlaneMeasure()
	{}

	void ActivateCylinderToPlaneMeasure::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::cylinderToPlane);
	}

	bool  ActivateCylinderToPlaneMeasure::canUndo() const
	{
		return (false);
	}

	void ActivateCylinderToPlaneMeasure::undoFunction(Controller& controller)
	{
		controller.getFunctionManager().abort(controller);
	}

	ControlType ActivateCylinderToPlaneMeasure::getType() const
	{
		return ControlType::ActivateCylinderToPlaneMeasure;
	}

	ActivateCylinderToPlane3Measure::ActivateCylinderToPlane3Measure()
	{}

	ActivateCylinderToPlane3Measure::~ActivateCylinderToPlane3Measure()
	{}

	void ActivateCylinderToPlane3Measure::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::cylinderToPlane3);
	}

	bool  ActivateCylinderToPlane3Measure::canUndo() const
	{
		return false;
	}

	void ActivateCylinderToPlane3Measure::undoFunction(Controller& controller)
	{
		controller.getFunctionManager().abort(controller);
	}

	ControlType ActivateCylinderToPlane3Measure::getType() const
	{
		return ControlType::ActivateCylinderToPlane3Measure;
	}

	/////////////////////////////////////////////Switch3PlanMeasure/////////////////////////////////////////

	Switch3PlanMeasure::Switch3PlanMeasure(bool threeplan) : m_threeplan(threeplan)
	{}

	Switch3PlanMeasure::~Switch3PlanMeasure()
	{}

	void Switch3PlanMeasure::doFunction(Controller & controller)
	{
		ContextType active = controller.getFunctionManager().isActiveContext();
			
		if (active == ContextType::none)
			return;
			
		ContextType functionToLaunch = ContextType::none;
		if(m_threeplan) switch (active) {
		case ContextType::cylinderToPlane:
			functionToLaunch = ContextType::cylinderToPlane3;
			break;
		case ContextType::pointToPlane:
			functionToLaunch = ContextType::pointToPlane3;
			break;
		}
		else switch (active) {
		case ContextType::cylinderToPlane3:
			functionToLaunch = ContextType::cylinderToPlane;
			break;
		case ContextType::pointToPlane3:
			functionToLaunch = ContextType::pointToPlane;
			break;
		}

		if (functionToLaunch != ContextType::none) {
			controller.getFunctionManager().abort(controller);
			controller.getFunctionManager().launchFunction(controller, functionToLaunch);
		}
	}

	bool Switch3PlanMeasure::canUndo() const
	{
		return false;
	}

	void Switch3PlanMeasure::undoFunction(Controller & controller)
	{}

	ControlType Switch3PlanMeasure::getType() const
	{
		return ControlType::Switch3PlanMeasure;
	}




	/////////////////////////////////////////////CylinderToCylinderMeasure////////////////////////////////////
	ActivateCylinderToCylinderMeasure::ActivateCylinderToCylinderMeasure()
	{}

	ActivateCylinderToCylinderMeasure::~ActivateCylinderToCylinderMeasure()
	{}

	void ActivateCylinderToCylinderMeasure::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::cylinderToCylinder);
	}

	bool  ActivateCylinderToCylinderMeasure::canUndo() const
	{
		return (false);
	}

	void ActivateCylinderToCylinderMeasure::undoFunction(Controller& controller)
	{
		controller.getFunctionManager().abort(controller);
	}

	ControlType ActivateCylinderToCylinderMeasure::getType() const
	{
		return ControlType::ActivateCylinderToCylinderMeasure;
	}

	///////////////////////////////////////////MultipleCylindersMeasure///////////////////////////////////
	ActivateMultipleCylindersMeasure::ActivateMultipleCylindersMeasure()
	{}

	ActivateMultipleCylindersMeasure::~ActivateMultipleCylindersMeasure()
	{}

	void ActivateMultipleCylindersMeasure::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::multipleCylinders);
	}

	bool  ActivateMultipleCylindersMeasure::canUndo() const
	{
		return (false);
	}

	void ActivateMultipleCylindersMeasure::undoFunction(Controller& controller)
	{
		controller.getFunctionManager().abort(controller);
	}

	ControlType ActivateMultipleCylindersMeasure::getType() const
	{
		return ControlType::ActivateMultipleCylindersMeasure;
	}

	///////////////////////////////////////////ActivatePointMeasure///////////////////////////////////

	ActivatePointMeasure::ActivatePointMeasure()
	{}

	ActivatePointMeasure::~ActivatePointMeasure()
	{}

	void ActivatePointMeasure::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::pointMeasure);
	}

	bool ActivatePointMeasure::canUndo() const
	{
		return (true);
	}

	void ActivatePointMeasure::undoFunction(Controller& controller)
	{
		controller.getFunctionManager().abort(controller);
	}

	ControlType ActivatePointMeasure::getType() const 
	{
		return ControlType::ActivatePointMeasure;
	}

	///////////////////////////////////////////ActivatePointCreation///////////////////////////////////

	ActivatePointCreation::ActivatePointCreation()
	{}

	ActivatePointCreation::~ActivatePointCreation()
	{}

	void ActivatePointCreation::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::pointCreation);
	}

	bool ActivatePointCreation::canUndo() const
	{
		return (true);
	}

	void ActivatePointCreation::undoFunction(Controller& controller)
	{
		controller.getFunctionManager().abort(controller);
	}

	ControlType ActivatePointCreation::getType() const
	{
		return ControlType::ActivatePointCreation;
	}

	///////////////////////////////////////////SphereDetection//////////////////////////////////
	ActivateSphere::ActivateSphere()
	{}

	ActivateSphere::~ActivateSphere()
	{}

	void ActivateSphere::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::Sphere);
	}

	bool  ActivateSphere::canUndo() const
	{
		return (false);
	}

	void ActivateSphere::undoFunction(Controller& controller)
	{
		controller.getFunctionManager().abort(controller);
	}

	ControlType ActivateSphere::getType() const
	{
		return ControlType::ActivateSphere;
	}
		
	///////////////////////////////////////////SphereIn4Clics//////////////////////////////////
	Activate4ClicsSphere::Activate4ClicsSphere()
	{}

	Activate4ClicsSphere::~Activate4ClicsSphere()
	{}

	void Activate4ClicsSphere::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::ClicsSphere4);
	}

	bool  Activate4ClicsSphere::canUndo() const
	{
		return (false);
	}

	void Activate4ClicsSphere::undoFunction(Controller& controller)
	{
		controller.getFunctionManager().abort(controller);
	}

	ControlType Activate4ClicsSphere::getType() const
	{
		return ControlType::Activate4ClicsSphere;
	}
	///////////////////////////////ExtendCylinder///////////////////////////////


	ActivateExtendCylinder::ActivateExtendCylinder()
	{}

	ActivateExtendCylinder::~ActivateExtendCylinder()
	{}

	void ActivateExtendCylinder::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::cylinder2ClickExtend);
	}

	bool ActivateExtendCylinder::canUndo() const
	{
		return (true);
	}

	void ActivateExtendCylinder::undoFunction(Controller& controller)
	{
		controller.getFunctionManager().abort(controller);
	}

	ControlType ActivateExtendCylinder::getType() const
	{
		return ControlType::ActivateExtendCylinder;
	}

	///PipeDetectionConnexion///

	ActivatePipeDetectionConnexion::ActivatePipeDetectionConnexion(double RonDext, bool angleConstraint, bool keepDiameter)
		: m_RonDext(RonDext), m_angleConstraint(angleConstraint), m_keepDiameter(keepDiameter)
	{}

	ActivatePipeDetectionConnexion::~ActivatePipeDetectionConnexion()
	{}

	void ActivatePipeDetectionConnexion::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::pipeDetectionConnexion);
		PipeConnetionMessage* msg = new PipeConnetionMessage(m_RonDext, m_angleConstraint,m_keepDiameter);
		controller.getFunctionManager().feedMessage(controller, msg);
	}

	bool  ActivatePipeDetectionConnexion::canUndo() const
	{
		return (false);
	}

	void ActivatePipeDetectionConnexion::undoFunction(Controller& controller)
	{
		controller.getFunctionManager().abort(controller);
	}

	ControlType ActivatePipeDetectionConnexion::getType() const
	{
		return ControlType::ActivatePipeDetectionConnexion;
	}

	///PipePostConnexion///

	ActivatePipePostConnexion::ActivatePipePostConnexion(double RonDext, bool angleConstraint,bool keepDiameter) 
		: m_RonDext(RonDext), m_angleConstraint(angleConstraint), m_keepDiameter(keepDiameter)
	{}

	ActivatePipePostConnexion::~ActivatePipePostConnexion()
	{}

	void ActivatePipePostConnexion::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::pipePostConnexion);
		PipeConnetionMessage* msg = new PipeConnetionMessage(m_RonDext, m_angleConstraint,m_keepDiameter);
		controller.getFunctionManager().feedMessage(controller, msg);
	}

	bool  ActivatePipePostConnexion::canUndo() const
	{
		return (false);
	}

	void ActivatePipePostConnexion::undoFunction(Controller& controller)
	{
		controller.getFunctionManager().abort(controller);
	}

	ControlType ActivatePipePostConnexion::getType() const
	{
		return ControlType::ActivatePipePostConnexion;
	}

	///BeamDetection///

	ActivateDetectBeam::ActivateDetectBeam()
	{}

	ActivateDetectBeam::~ActivateDetectBeam()
	{}

	void ActivateDetectBeam::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::beamDetection);
	}

	bool  ActivateDetectBeam::canUndo() const
	{
		return (false);
	}

	void ActivateDetectBeam::undoFunction(Controller& controller)
	{
		controller.getFunctionManager().abort(controller);
	}

	ControlType ActivateDetectBeam::getType() const
	{
		return ControlType::ActivateDetectBeam;
	}

	///SlabDetection///

	ActivateSlabDetection::ActivateSlabDetection()
	{}

	ActivateSlabDetection::~ActivateSlabDetection()
	{}

	void ActivateSlabDetection::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::Slab2Click);
	}

	bool  ActivateSlabDetection::canUndo() const
	{
		return (false);
	}

	void ActivateSlabDetection::undoFunction(Controller& controller)
	{
		controller.getFunctionManager().abort(controller);
	}

	ControlType ActivateSlabDetection::getType() const
	{
		return ControlType::ActivateSlabDetection;
	}

	/// Mesure Point to Mesh ///
	ActivatePointToMeshMeasure::ActivatePointToMeshMeasure()
	{}

	ActivatePointToMeshMeasure::~ActivatePointToMeshMeasure()
	{}

	void ActivatePointToMeshMeasure::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::meshDistance);
	}

	bool  ActivatePointToMeshMeasure::canUndo() const
	{
		return (false);
	}

	void ActivatePointToMeshMeasure::undoFunction(Controller& controller)
	{
		controller.getFunctionManager().abort(controller);
	}

	ControlType ActivatePointToMeshMeasure::getType() const
	{
		return ControlType::ActivatePointToMeshMeasure;
	}

	/// Mesure Slab ///
	ActivateSlabMeasure::ActivateSlabMeasure(bool isExtend, bool is3Click)
		: m_isExtend(isExtend)
		, m_is3Click(is3Click)
	{}

	ActivateSlabMeasure::~ActivateSlabMeasure()
	{}

	void ActivateSlabMeasure::doFunction(Controller& controller)
	{
		if(m_is3Click)
			controller.getFunctionManager().launchFunction(controller, ContextType::Slab2Click);
		else
			controller.getFunctionManager().launchFunction(controller, ContextType::Slab1Click);

		SimpleNumberMessage* msg = new SimpleNumberMessage(m_isExtend); //0 if no extend, 1 if auto extend
		controller.getFunctionManager().feedMessage(controller, msg);
	}

	bool ActivateSlabMeasure::canUndo() const
	{
		return false;
	}

	void ActivateSlabMeasure::undoFunction(Controller& controller)
	{}

	ControlType ActivateSlabMeasure::getType() const
	{
		return ControlType::ActivateSlabMeasure;
	}

	/// Plane Connections ///
	ActivatePlaneConnexion::ActivatePlaneConnexion()
	{}

	ActivatePlaneConnexion::~ActivatePlaneConnexion()
	{}

	void ActivatePlaneConnexion::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::planeConnexion);
	}

	bool ActivatePlaneConnexion::canUndo() const
	{
		return false;
	}

	void ActivatePlaneConnexion::undoFunction(Controller& controller)
	{}

	ControlType ActivatePlaneConnexion::getType() const
	{
		return ControlType::ActivatePlaneConnexion;
	}


	/// Plane Detections ///
	ActivatePlaneDetection::ActivatePlaneDetection(const PlaneDetectionOptions& options):m_options(options)
	{}

	ActivatePlaneDetection::~ActivatePlaneDetection()
	{}

	void ActivatePlaneDetection::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::planeDetection);
		PlaneMessage* msg = new PlaneMessage(m_options);
		controller.getFunctionManager().feedMessage(controller, msg);
	}

	bool ActivatePlaneDetection::canUndo() const
	{
		return false;
	}

	void ActivatePlaneDetection::undoFunction(Controller& controller)
	{}

	ControlType ActivatePlaneDetection::getType() const
	{
		return ControlType::ActivatePlaneDetection;
	}


	/// People Remover ///

	ActivatePeopleRemover::ActivatePeopleRemover()
	{}

	ActivatePeopleRemover::~ActivatePeopleRemover()
	{}

	void ActivatePeopleRemover::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::peopleRemover);
	}

	bool ActivatePeopleRemover::canUndo() const
	{
		return false;
	}

	void ActivatePeopleRemover::undoFunction(Controller& controller)
	{}

	ControlType ActivatePeopleRemover::getType() const
	{
		return ControlType::ActivatePeopleRemover;
	}
	
	/// SetOfPoints ///

	ActivateSetOfPoints::ActivateSetOfPoints(double step, double threshold, SetOfPointsOptions options, bool userAxes, bool createMeasures, bool fromTop, bool horizontal)
		: m_step(step), m_threshold(threshold), m_options(options),m_userAxes(userAxes),m_createMeasures(createMeasures),m_fromTop(fromTop), m_horizontal(horizontal)
	{}

	ActivateSetOfPoints::~ActivateSetOfPoints()
	{}

	void ActivateSetOfPoints::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::setOfPoints);

		SetOfPointsMessage* msg = new SetOfPointsMessage(m_options, m_userAxes,m_createMeasures,m_fromTop,m_step,m_threshold, m_horizontal);
		controller.getFunctionManager().feedMessage(controller, msg);
	}

	bool ActivateSetOfPoints::canUndo() const
	{
		return false;
	}

	void ActivateSetOfPoints::undoFunction(Controller& controller)
	{}

	ControlType ActivateSetOfPoints::getType() const
	{
		return ControlType::ActivateSetOfPoints;
	}

	/// Torus ///

	ActivateTorus::ActivateTorus()
	{}

	ActivateTorus::~ActivateTorus()
	{}

	void ActivateTorus::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::fitTorus);

	}

	bool ActivateTorus::canUndo() const
	{
		return false;
	}

	void ActivateTorus::undoFunction(Controller& controller)
	{}

	ControlType ActivateTorus::getType() const
	{
		return ControlType::ActivateTorus;
	}

	/// Trajectory ///

	ActivateTrajectory::ActivateTrajectory()
	{}

	ActivateTrajectory::~ActivateTrajectory()
	{}

	void ActivateTrajectory::doFunction(Controller& controller)
	{
		controller.getFunctionManager().launchFunction(controller, ContextType::trajectory);

	}

	bool ActivateTrajectory::canUndo() const
	{
		return false;
	}

	void ActivateTrajectory::undoFunction(Controller& controller)
	{}

	ControlType ActivateTrajectory::getType() const
	{
		return ControlType::ActivateTrajectory;
	}
}
