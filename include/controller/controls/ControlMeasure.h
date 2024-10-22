#ifndef CONTROL_MEASURE_H_
#define CONTROL_MEASURE_H_

#include "controller/controls/IControl.h"
#include "controller/functionSystem/PipeDetectionOptions.h"

#include "models/data/PolylineMeasure/PolyLineTypes.h"
#include "controller/functionSystem/PlaneDetectionOptions.h"

namespace control
{
	namespace measure
	{

		class ActivateSimpleMeasure : public AControl
		{
		public:
			ActivateSimpleMeasure();
			~ActivateSimpleMeasure();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class ActivatePolylineMeasure : public AControl
		{
		public:
			ActivatePolylineMeasure();
			~ActivatePolylineMeasure();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class SendMeasuresOptions : public AControl
		{
		public:
			SendMeasuresOptions(const PolyLineOptions& options);
			~SendMeasuresOptions();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;

		private:
			PolyLineOptions m_options;
		};

		class SendPipeDetectionOptions : public AControl
		{
		public:
			SendPipeDetectionOptions(const PipeDetectionOptions& options);
			~SendPipeDetectionOptions();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;

		private:
			PipeDetectionOptions m_options;

		};

		class ActivateFitCylinder : public AControl 
		{
		public:
			ActivateFitCylinder();
			~ActivateFitCylinder();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;

		};

		class ActivateBigCylinderFit : public AControl
		{
		public:
			ActivateBigCylinderFit();
			~ActivateBigCylinderFit();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		};
		class ActivateBeamBending : public AControl
		{
		public:
			ActivateBeamBending(const BeamBendingOptions& options);
			~ActivateBeamBending();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			BeamBendingOptions m_options;
		};

		class SetBendingTolerance : public AControl
		{
		public:
			SetBendingTolerance(const double& tolerance);
			~SetBendingTolerance();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const double m_newTolerance;
			double m_oldTolerance;
		};

		class ActivateColumnTilt : public AControl
		{
		public:
			ActivateColumnTilt();
			~ActivateColumnTilt();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class SetTiltTolerance : public AControl
		{
		public:
			SetTiltTolerance(const double& tolerance);
			~SetTiltTolerance();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const double m_newTolerance;
			double m_oldTolerance;
		};

		class ActivatePointToCylinderMeasure : public AControl
		{
		public:
			ActivatePointToCylinderMeasure();
			~ActivatePointToCylinderMeasure();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class ActivatePointToPlaneMeasure : public AControl
		{
		public:
			ActivatePointToPlaneMeasure();
			~ActivatePointToPlaneMeasure();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class ActivatePointToPlane3Measure : public AControl
		{
		public:
			ActivatePointToPlane3Measure();
			~ActivatePointToPlane3Measure();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class ActivateCylinderToPlaneMeasure : public AControl
		{
		public:
			ActivateCylinderToPlaneMeasure();
			~ActivateCylinderToPlaneMeasure();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class ActivateCylinderToPlane3Measure : public AControl
		{
		public:
			ActivateCylinderToPlane3Measure();
			~ActivateCylinderToPlane3Measure();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class Switch3PlanMeasure : public AControl
		{
		public:
			Switch3PlanMeasure(bool threeplan);
			~Switch3PlanMeasure();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			bool m_threeplan;
		};

		class ActivateCylinderToCylinderMeasure : public AControl
		{
		public:
			ActivateCylinderToCylinderMeasure();
			~ActivateCylinderToCylinderMeasure();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class ActivateMultipleCylindersMeasure : public AControl
		{
		public:
			ActivateMultipleCylindersMeasure();
			~ActivateMultipleCylindersMeasure();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class ActivatePointMeasure : public AControl
		{
		public:
			ActivatePointMeasure();
			~ActivatePointMeasure();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class ActivatePointCreation : public AControl
		{
		public:
			ActivatePointCreation();
			~ActivatePointCreation();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class ActivateSphere : public AControl
		{
		public:
			ActivateSphere();
			~ActivateSphere();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class Activate4ClicsSphere : public AControl
		{
		public:
			Activate4ClicsSphere();
			~Activate4ClicsSphere();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class ActivateExtendCylinder : public AControl
		{
		public:
			ActivateExtendCylinder();
			~ActivateExtendCylinder();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class ActivatePipeDetectionConnexion : public AControl
		{
		public:
			ActivatePipeDetectionConnexion(double RonDext, bool angleConstraint, bool keepDiameter);
			~ActivatePipeDetectionConnexion();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;

		private:
			double m_RonDext;
			bool m_angleConstraint;
			bool m_keepDiameter;


		};

		class ActivatePipePostConnexion : public AControl
		{
		public:
			ActivatePipePostConnexion(double RonDext, bool angleConstraint, bool keepDiameter);
			~ActivatePipePostConnexion();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;

		private:
			double m_RonDext;
			bool m_angleConstraint;
			bool m_keepDiameter;


		};

		class ActivateDetectBeam : public AControl
		{
		public:
			ActivateDetectBeam();
			~ActivateDetectBeam();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;

		};

		class ActivateSlabDetection : public AControl
		{
		public:
			ActivateSlabDetection();
			~ActivateSlabDetection();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;

		};

		class ActivatePointToMeshMeasure : public AControl
		{
		public:
			ActivatePointToMeshMeasure();
			~ActivatePointToMeshMeasure();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class ActivateSlabMeasure : public AControl
		{
		public:
			ActivateSlabMeasure(bool isExtend, bool is3Click);
			~ActivateSlabMeasure();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			bool m_isExtend;
			bool m_is3Click;
		};

		class ActivatePlaneConnexion : public AControl
		{
		public:
			ActivatePlaneConnexion();
			~ActivatePlaneConnexion();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class ActivatePlaneDetection : public AControl
		{
		public:
			ActivatePlaneDetection(const PlaneDetectionOptions& options);
			~ActivatePlaneDetection();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			PlaneDetectionOptions m_options;
		};

		class ActivatePeopleRemover : public AControl
		{
		public:
			ActivatePeopleRemover();
			~ActivatePeopleRemover();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class ActivateSetOfPoints : public AControl
		{
		public:
			ActivateSetOfPoints(double step, double threshold, SetOfPointsOptions options, bool userAxes, bool createMeasures, bool fromTop, bool horizontal);
			~ActivateSetOfPoints();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			double m_step;
			double m_threshold;
			SetOfPointsOptions m_options;
			bool m_userAxes, m_createMeasures, m_fromTop, m_horizontal;
		};

		class ActivateTorus : public AControl
		{
		public:
			ActivateTorus();
			~ActivateTorus();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class ActivateTrajectory : public AControl
		{
		public:
			ActivateTrajectory();
			~ActivateTrajectory();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		};

	}
}
#endif //CONTROLMEASURE_H_