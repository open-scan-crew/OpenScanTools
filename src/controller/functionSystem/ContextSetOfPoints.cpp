#include "controller/functionSystem/ContextSetOfPoints.h"

#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/IControlListener.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "pointCloudEngine/TlScanOverseer.h"
#include "utils/Logger.h"
#include "controller/messages/PlaneMessage.h"
#include "pointCloudEngine/OctreeRayTracing.h"
#include "models/graph/GraphManager.h"
#include "models/graph/PointNode.h"
#include "controller/controls/ControlFunction.h"
#include "pointCloudEngine/MeasureClass.h"
#include "models/graph/SimpleMeasureNode.h"
#include "models/graph/ClusterNode.h"

ContextSetOfPoints::ContextSetOfPoints(const ContextId& id)
	: ARayTracingContext(id)
{
	resetClickUsages();
}

ContextSetOfPoints::~ContextSetOfPoints()
{
}

void ContextSetOfPoints::resetClickUsages()
{
	m_usages.clear();
	m_clickResults.clear();

	if (m_options == SetOfPointsOptions::case1)
	{
		m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_PLANE3_FIRST });
		m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_PLANE3_FIRST });
	}
	
	
	else if (m_options == SetOfPointsOptions::case3)
	{
		m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_PLANE3_FIRST });
		m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_PLANE3_FIRST });
		m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_PLANE3_FIRST });
	}

	else
	{
		m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_PLANE3_FIRST });
		m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_PLANE3_FIRST });
		m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_PLANE3_FIRST });
		m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_PLANE3_FIRST });
	}
}

ContextState ContextSetOfPoints::start(Controller& controller)
{
	return ARayTracingContext::start(controller);
}

ContextState ContextSetOfPoints::feedMessage(IMessage* message, Controller& controller)
{
	ARayTracingContext::feedMessage(message, controller);
	switch (message->getType())
	{
	case IMessage::MessageType::SETOFPOINTSMESSAGE:
	{
		SetOfPointsMessage* setOfPointsMessage = static_cast<SetOfPointsMessage*>(message);
		m_options = setOfPointsMessage->getOptions();
		m_userAxes = setOfPointsMessage->getUserAxes();
		m_createMeasures = setOfPointsMessage->getCreateMeasures();
		m_fromTop = setOfPointsMessage->getFromTop();
		m_threshold = setOfPointsMessage->getThreshold();
		m_step = setOfPointsMessage->getStep();
		m_horizontal = setOfPointsMessage->getHorizontal();
		resetClickUsages();
		break;
	}
	default:
		break;
	}
	return (m_state);
}

ContextState ContextSetOfPoints::launch(Controller& controller)
{
	
	// --- Ray Tracing ---
	ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
	// -!- Ray Tracing -!-
	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));
	bool success = false;

	GraphManager& graphManager = controller.getGraphManager();
	ClippingAssembly clippingAssembly;
	graphManager.getClippingAssembly(clippingAssembly, true, false);

	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));

	TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));
	std::vector<glm::dvec3> createdPoints(0);
	std::vector<bool> arePointsReal(0);
	double cosAngleThreshold(0.01);
	double cosAngleThresholdBefore(0.01);

	std::unordered_set<SafePtr<AGraphNode>> pointsToCreate(0);
	std::unordered_set<SafePtr<AGraphNode>> measuresToCreate(0);
	switch (m_options)
	{
	case SetOfPointsOptions::case1:
	{
		glm::dvec3 projDirection(0.0, 0.0, 1.0);
		if (m_fromTop)
			projDirection = -projDirection;
		TlScanOverseer::getInstance().setOfPoints(projDirection, m_clickResults[0].position, m_clickResults[1].position, m_step, m_threshold, createdPoints, arePointsReal, cosAngleThreshold, clippingAssembly);
		//create points
		
		int pointToCreate(0);
		for (int i = 0; i < (int)createdPoints.size(); i++)
		{
			if (arePointsReal[i])
				pointToCreate++;
		}


		std::vector<uint32_t> indexes = controller.getMultipleUserId(ElementType::Point, pointToCreate);
		int currentIndex(0);
		for (int i = 0; i < (int)createdPoints.size(); i++)
		{
			if (arePointsReal[i])
			{
				SafePtr<PointNode> point = make_safe<PointNode>();
				WritePtr<PointNode> wPoint = point.get();
				if (!wPoint)
				{
					m_clickResults.clear();
					return waitForNextPoint(controller);
				}

				wPoint->setDefaultData(controller);
				wPoint->setPosition(createdPoints[i]);	
				wPoint->setUserIndex(indexes[currentIndex]);
				currentIndex++;
				if (!controller.getContext().getActiveName().empty())
					wPoint->setName(controller.getContext().getActiveName());
				else
					wPoint->setName(L"Inter");
				pointsToCreate.insert(point);
				//controller.getControlListener()->notifyUIControl(new control::function::AddNodes({ point }));
			}
		}
		
		break;
	}
	case SetOfPointsOptions::case2:
	{
		glm::dvec3 projDirection(0.0, 0.0, 1.0);
		if (m_fromTop)
			projDirection = -projDirection;
		glm::dvec3 horizontal = glm::cross(m_clickResults[0].position - m_clickResults[1].position, projDirection);
		horizontal /= glm::length(horizontal);
		horizontal = glm::cross(horizontal, projDirection);
		horizontal /= glm::length(horizontal);
		
		//create first set of points
		TlScanOverseer::getInstance().setOfPoints(projDirection, m_clickResults[0].position, m_clickResults[1].position, m_step, m_threshold, createdPoints, arePointsReal, cosAngleThreshold, clippingAssembly);

		//create second set of points
		std::vector<glm::dvec3> createdPoints2(0);
		std::vector<bool> arePointsReal2(0);
		//get start and end for second sets of points
		glm::dvec3 start2, end2,lineDirection;
		lineDirection = m_clickResults[3].position - m_clickResults[2].position;
		lineDirection /= glm::length(lineDirection);
		start2 = MeasureClass::intersectionLinePlane(m_clickResults[0].position, horizontal, m_clickResults[2].position, lineDirection);
		end2 = MeasureClass::intersectionLinePlane(m_clickResults[1].position, horizontal, m_clickResults[2].position, lineDirection);
		//double secondStep = m_step * abs(glm::dot(start2-end2, m_clickResults[0].position - m_clickResults[1].position) / (glm::length(m_clickResults[0].position - m_clickResults[1].position)*glm::length(start2-end2)));
		double secondStep = m_step * glm::length(start2 - end2) / glm::length(m_clickResults[0].position - m_clickResults[1].position);

		TlScanOverseer::getInstance().setOfPoints(projDirection, start2, end2, secondStep, m_threshold, createdPoints2, arePointsReal2, cosAngleThreshold, clippingAssembly);

		//create measures
		if ((int)createdPoints.size() != createdPoints2.size())
		{
			Logger::log(LoggerMode::rayTracingLog) << "not the same number of points" << Logger::endl;
			break;
		}

		int pointToCreate(0);
		for (int i = 0; i < (int)createdPoints.size(); i++)
		{
			if ((arePointsReal[i])&&(arePointsReal2[i]))
				pointToCreate++;
		}

		std::vector<uint32_t> indexesMeasure = controller.getMultipleUserId(ElementType::SimpleMeasure, pointToCreate);
		int currentIndex(0);
		for (int i = 0; i < (int)createdPoints.size(); i++)
		{
			if ((arePointsReal[i])&&(arePointsReal2[i]))
			{
				if (!m_createMeasures)
				{
					SafePtr<PointNode> point = make_safe<PointNode>();
					WritePtr<PointNode> wPoint = point.get();
					if (!wPoint)
					{
						m_clickResults.clear();
						return waitForNextPoint(controller);
					}

					wPoint->setDefaultData(controller);
					wPoint->setPosition(createdPoints2[i]);
					wPoint->setUserIndex(indexesMeasure[currentIndex]);
					currentIndex++;
					if (!controller.getContext().getActiveName().empty())
						wPoint->setName(controller.getContext().getActiveName());
					else
						wPoint->setName(L"Inter");
					pointsToCreate.insert(point);
					//controller.getControlListener()->notifyUIControl(new control::function::AddNodes({ point }));
				}

				if (m_createMeasures)
				{
					Measure newMeasure;
					newMeasure.origin = createdPoints[i];
					newMeasure.final = createdPoints2[i];

					SafePtr<SimpleMeasureNode> measure = make_safe<SimpleMeasureNode>();
					WritePtr<SimpleMeasureNode> wMeasure = measure.get();
					if (!wMeasure)
					{
						assert(false);
						m_clickResults.clear();
						return waitForNextPoint(controller);
					}

					wMeasure->setDefaultData(controller);
					wMeasure->setMeasure(newMeasure);
					wMeasure->setUserIndex(indexesMeasure[currentIndex]);
					currentIndex++;
					if (!controller.getContext().getActiveName().empty())
						wMeasure->setName(controller.getContext().getActiveName());
					else
						wMeasure->setName(L"Auto_Dist");
					measuresToCreate.insert(measure);
					//controller.getControlListener()->notifyUIControl(new control::function::AddNodes(measure));
				}
			}
		}

		//measure need to be created if option is checked

		break;
		/*
		//outdated
		glm::dvec3 vertical(0.0, 0.0, 1.0);
		if (m_fromTop)
			vertical = -vertical;
		glm::dvec3 horizontal = glm::cross(m_clickResults[0].position - m_clickResults[1].position, vertical);
		horizontal /= glm::length(horizontal);
		glm::dvec3 projDirection = glm::cross(horizontal, m_clickResults[0].position - m_clickResults[1].position);
		projDirection /= glm::length(projDirection);
		
		if (glm::dot(vertical,projDirection)<0)
			projDirection = -projDirection;
		
		TlScanOverseer::getInstance().setOfPoints(projDirection, m_clickResults[0].position, m_clickResults[1].position, m_step, m_threshold, createdPoints, arePointsReal, cosAngleThreshold, clippingAssembly);
		//create points
		int pointToCreate(0);
		for (int i = 0; i < (int)createdPoints.size(); i++)
		{
			if (arePointsReal[i])
				pointToCreate++;
		}

		std::vector<uint32_t> indexes = controller.getMultipleUserId(ElementType::Point, pointToCreate);
		int currentIndex(0);
		for (int i = 0; i < (int)createdPoints.size(); i++)
		{
			if (arePointsReal[i])
			{
				SafePtr<PointNode> point = make_safe<PointNode>();
				WritePtr<PointNode> wPoint = point.get();
				if (!wPoint)
				{
					m_clickResults.clear();
					return waitForNextPoint(controller);
				}

				wPoint->setDefaultData(controller);
				wPoint->setPosition(createdPoints[i]);
				wPoint->setUserIndex(indexes[currentIndex]);
				currentIndex++;
				if (!controller.getContext().getActiveName().empty())
					wPoint->setName(controller.getContext().getActiveName());
				else
					wPoint->setName(L"Inter");
				pointsToCreate.insert(point);
				//controller.getControlListener()->notifyUIControl(new control::function::AddNodes({ point }));
			}
		}
		break;*/
	}
	case SetOfPointsOptions::case3:
	{
		if (!m_userAxes)
		{
			//find plane containing the 3 points, no user axis mode yet
			std::vector<glm::dvec3> planePoints(0);
			planePoints.push_back(m_clickResults[0].position);
			planePoints.push_back(m_clickResults[1].position);
			planePoints.push_back(m_clickResults[2].position);
			std::vector<double> plane(0);
			OctreeRayTracing::fitPlane(planePoints, plane);
			glm::dvec3 normal(plane[0], plane[1], plane[2]);
			glm::dvec3 vertical(0.0, 0.0, 1.0);
			glm::dvec3 projDirection = glm::cross(normal, vertical);
			projDirection /= glm::length(projDirection);
			if (glm::dot(projDirection, m_clickResults[0].position - m_clickResults[2].position) < 0)
				projDirection = -projDirection;

			TlScanOverseer::getInstance().setOfPoints(projDirection, m_clickResults[0].position, m_clickResults[1].position, m_step, m_threshold, createdPoints, arePointsReal, cosAngleThreshold, clippingAssembly);
			//create points
			int pointToCreate(0);
			for (int i = 0; i < (int)createdPoints.size(); i++)
			{
				if (arePointsReal[i])
					pointToCreate++;
			}

			std::vector<uint32_t> indexes = controller.getMultipleUserId(ElementType::Point, pointToCreate);
			int currentIndex(0);
			for (int i = 0; i < (int)createdPoints.size(); i++)
			{
				if (arePointsReal[i])
				{
					SafePtr<PointNode> point = make_safe<PointNode>();
					WritePtr<PointNode> wPoint = point.get();
					if (!wPoint)
					{
						m_clickResults.clear();
						return waitForNextPoint(controller);
					}

					wPoint->setDefaultData(controller);
					wPoint->setPosition(createdPoints[i]);
					wPoint->setUserIndex(indexes[currentIndex]);
					currentIndex++;
					if (!controller.getContext().getActiveName().empty())
						wPoint->setName(controller.getContext().getActiveName());
					else
						wPoint->setName(L"Inter");
					pointsToCreate.insert(point);
					//controller.getControlListener()->notifyUIControl(new control::function::AddNodes({ point }));
				}
			}

			break;
		}
		else
		{
			//project 3 point to line (12)
			glm::dvec3 point1(m_clickResults[0].position), point2(m_clickResults[1].position), point3(m_clickResults[2].position);
			glm::dvec3 projPoint = MeasureClass::projectPointToLine(point3, (point1 - point2)/glm::length(point1-point2), point1);

			// get user axes here
			UserOrientation uo=controller.getContext().getActiveUserOrientation();
			std::array<glm::dvec3, 2> userAxesPoints = uo.getAxisPoints();
			glm::dvec3 userX, userY, userZ(glm::dvec3(0.0, 0.0, 1.0));
			if (uo.getAxisType() == UOAxisType::XAxis)
			{
				userX = glm::dvec3(userAxesPoints[1] - userAxesPoints[0]);
				userX[2] = 0;
				userX /= glm::length(userX);
				userY = glm::cross(userZ, userX);
				userY /= glm::length(userY);
			}
			else if (uo.getAxisType() == UOAxisType::YAxis)
			{
				userY = glm::dvec3(userAxesPoints[1] - userAxesPoints[0]);
				userY[2] = 0;
				userY /= glm::length(userY);
				userX = glm::cross(userY, userZ);
				userX /= glm::length(userX);
			}
			else
			{
				userAxesPoints = uo.getCustomAxis();
				userX = glm::dvec3(userAxesPoints[1] - userAxesPoints[0]);
				userX[2] = 0;
				userX /= glm::length(userX);
				userY = glm::cross(userZ, userX);
				userY /= glm::length(userY);
			}
			
			glm::dvec3 projDir = projPoint - point3;
			projDir /= glm::length(projDir);
			glm::dvec3 realAxis = userX;
			if(abs(glm::dot(projDir,userY))>(abs(glm::dot(projDir,userX))))
				realAxis=userY;
			if (abs(glm::dot(userZ, projDir)) > (abs(glm::dot(projDir, realAxis))))
				realAxis = userZ;
			if (glm::dot(realAxis, projDir) < 0)
				realAxis = -realAxis;
			TlScanOverseer::getInstance().setOfPoints(realAxis, m_clickResults[0].position, m_clickResults[1].position, m_step, m_threshold, createdPoints, arePointsReal, cosAngleThreshold, clippingAssembly);
			//create points
			int pointToCreate(0);
			for (int i = 0; i < (int)createdPoints.size(); i++)
			{
				if (arePointsReal[i])
					pointToCreate++;
			}

			std::vector<uint32_t> indexes = controller.getMultipleUserId(ElementType::Point, pointToCreate);
			int currentIndex(0);
			for (int i = 0; i < (int)createdPoints.size(); i++)
			{
				if (arePointsReal[i])
				{
					SafePtr<PointNode> point = make_safe<PointNode>();
					WritePtr<PointNode> wPoint = point.get();
					if (!wPoint)
					{
						m_clickResults.clear();
						return waitForNextPoint(controller);
					}

					wPoint->setDefaultData(controller);
					wPoint->setPosition(createdPoints[i]);
					wPoint->setUserIndex(indexes[currentIndex]);
					currentIndex++;
					std::wstring name = L"Inter";
					wPoint->setName(name);
					pointsToCreate.insert(point);
					//controller.getControlListener()->notifyUIControl(new control::function::AddNodes({ point }));
				}
			}

			break;


		}
	}
	case SetOfPointsOptions::case4:
	{
		//get real 3rd point
		glm::dvec3 thirdPoint = m_clickResults[2].position;
		if (glm::dot(m_clickResults[1].position - m_clickResults[0].position, m_clickResults[3].position - m_clickResults[2].position) < 0)
			thirdPoint = m_clickResults[3].position;

		//find plane containing the 3 points, no user axis mode yet
		std::vector<glm::dvec3> planePoints(0);
		planePoints.push_back(m_clickResults[0].position);
		planePoints.push_back(m_clickResults[1].position);
		planePoints.push_back(thirdPoint);
		std::vector<double> plane(0);
		OctreeRayTracing::fitPlane(planePoints, plane);
		glm::dvec3 normal(plane[0], plane[1], plane[2]);
		glm::dvec3 vertical(0.0, 0.0, 1.0);
		glm::dvec3 projDirection = glm::cross(normal, vertical);
		projDirection /= glm::length(projDirection);
		if (glm::dot(projDirection, thirdPoint - m_clickResults[0].position) < 0)
			projDirection = -projDirection;

		std::vector<glm::dvec3> userPoints(0);
		userPoints.push_back(m_clickResults[0].position);
		userPoints.push_back(m_clickResults[1].position);
		userPoints.push_back(m_clickResults[2].position);
		userPoints.push_back(m_clickResults[3].position);
	
		std::vector<glm::dvec3> createdPointStart(0), createdPointEnd(0);
		TlScanOverseer::getInstance().setOfPointsWith4thPoint(projDirection, userPoints, m_step, m_threshold, createdPointStart, createdPointEnd, arePointsReal, cosAngleThreshold, clippingAssembly);
		//create points
		int pointToCreate(0);
		for (int i = 0; i < (int)createdPointStart.size(); i++)
		{
			if (arePointsReal[i])
				pointToCreate++;
		}

		std::vector<uint32_t> indexes = controller.getMultipleUserId(ElementType::Point, pointToCreate);
		std::vector<uint32_t> indexesMeasure = controller.getMultipleUserId(ElementType::SimpleMeasure, pointToCreate);

		int currentIndex(0);
		for (int i = 0; i < (int)createdPointStart.size(); i++)
		{
			if (arePointsReal[i])
			{
				if (!m_createMeasures)
				{
					SafePtr<PointNode> point = make_safe<PointNode>();
					WritePtr<PointNode> wPoint = point.get();
					if (!wPoint)
					{
						m_clickResults.clear();
						return waitForNextPoint(controller);
					}

					wPoint->setDefaultData(controller);
					wPoint->setPosition(createdPointEnd[i]);
					wPoint->setUserIndex(indexes[currentIndex]);
					currentIndex++;
					if (!controller.getContext().getActiveName().empty())
						wPoint->setName(controller.getContext().getActiveName());
					else
						wPoint->setName(L"Inter");
					pointsToCreate.insert(point);
					//controller.getControlListener()->notifyUIControl(new control::function::AddNodes({ point }));
				}
				else
				{
					Measure newMeasure;
					newMeasure.origin = createdPointStart[i];
					newMeasure.final = createdPointEnd[i];

					SafePtr<SimpleMeasureNode> measure = make_safe<SimpleMeasureNode>();
					WritePtr<SimpleMeasureNode> wMeasure = measure.get();
					if (!wMeasure)
					{
						assert(false);
						m_clickResults.clear();
						return waitForNextPoint(controller);
					}

					wMeasure->setDefaultData(controller);
					wMeasure->setMeasure(newMeasure);
					wMeasure->setUserIndex(indexesMeasure[currentIndex]);
					currentIndex++;
					if (!controller.getContext().getActiveName().empty())
						wMeasure->setName(controller.getContext().getActiveName());
					else
						wMeasure->setName(L"Auto_Dist");
					measuresToCreate.insert(measure);
					//controller.getControlListener()->notifyUIControl(new control::function::AddNodes(measure));
				}
			}
		}

		break;
	}
	case SetOfPointsOptions::case5:
	{

		//get real 3rd point
		glm::dvec3 thirdPoint = m_clickResults[2].position;
		if (glm::dot(m_clickResults[1].position - m_clickResults[0].position, m_clickResults[3].position - m_clickResults[2].position) < 0)
			thirdPoint = m_clickResults[3].position;

		//find plane containing the 3 points, no user axis mode yet
		std::vector<glm::dvec3> planePoints(0);
		planePoints.push_back(m_clickResults[0].position);
		planePoints.push_back(m_clickResults[1].position);
		planePoints.push_back(thirdPoint);
		std::vector<double> plane(0);
		OctreeRayTracing::fitPlane(planePoints, plane);
		glm::dvec3 normal(plane[0], plane[1], plane[2]);
		glm::dvec3 projDirection = glm::cross(normal, m_clickResults[0].position - m_clickResults[1].position);
		if (glm::dot(projDirection, thirdPoint - m_clickResults[0].position) < 0)
			projDirection = -projDirection;
		projDirection /= glm::length(projDirection);

		std::vector<glm::dvec3> createdPointStart(0), createdPointEnd(0);
		std::vector<glm::dvec3> userPoints(0);
		userPoints.push_back(m_clickResults[0].position);
		userPoints.push_back(m_clickResults[1].position);
		userPoints.push_back(m_clickResults[2].position);
		userPoints.push_back(m_clickResults[3].position);
		TlScanOverseer::getInstance().setOfPointsWith4thPoint(projDirection, userPoints, m_step, m_threshold, createdPointStart, createdPointEnd, arePointsReal, cosAngleThreshold, clippingAssembly);
		//create points
		int pointToCreate(0);
		for (int i = 0; i < (int)createdPointStart.size(); i++)
		{
			if (arePointsReal[i])
				pointToCreate++;
		}

		std::vector<uint32_t> indexes = controller.getMultipleUserId(ElementType::Point, pointToCreate);
		std::vector<uint32_t> indexesMeasure = controller.getMultipleUserId(ElementType::SimpleMeasure, pointToCreate);
		int currentIndex(0);
		for (int i = 0; i < (int)createdPointStart.size(); i++)
		{
			if (arePointsReal[i])
			{
				if (!m_createMeasures)
				{
					SafePtr<PointNode> point = make_safe<PointNode>();
					WritePtr<PointNode> wPoint = point.get();
					if (!wPoint)
					{
						m_clickResults.clear();
						return waitForNextPoint(controller);
					}

					wPoint->setDefaultData(controller);
					wPoint->setPosition(createdPointEnd[i]);
					wPoint->setUserIndex(indexes[currentIndex]);
					currentIndex++;
					if (!controller.getContext().getActiveName().empty())
						wPoint->setName(controller.getContext().getActiveName());
					else
						wPoint->setName(L"Inter");
					pointsToCreate.insert(point);
					//controller.getControlListener()->notifyUIControl(new control::function::AddNodes({ point }));
				}
				
				else
				{
					Measure newMeasure;
					newMeasure.origin = createdPointStart[i];
					newMeasure.final = createdPointEnd[i];

					SafePtr<SimpleMeasureNode> measure = make_safe<SimpleMeasureNode>();
					WritePtr<SimpleMeasureNode> wMeasure = measure.get();
					if (!wMeasure)
					{
						assert(false);
						m_clickResults.clear();
						return waitForNextPoint(controller);
					}

					wMeasure->setDefaultData(controller);
					wMeasure->setMeasure(newMeasure);
					wMeasure->setUserIndex(indexesMeasure[currentIndex]);
					currentIndex++;
					if (!controller.getContext().getActiveName().empty())
						wMeasure->setName(controller.getContext().getActiveName());
					else
						wMeasure->setName(L"Auto_Dist");
					measuresToCreate.insert(measure);
					//controller.getControlListener()->notifyUIControl(new control::function::AddNodes(measure));
				}
			}
		}

		break;
	}
	case SetOfPointsOptions::case6:
	{
		//get real 3rd point
		glm::dvec3 thirdPoint = m_clickResults[2].position;
		if (glm::dot(m_clickResults[1].position - m_clickResults[0].position, m_clickResults[3].position - m_clickResults[2].position) < 0)
			thirdPoint = m_clickResults[3].position;

		
		//same as caseD, but vertically
		glm::dvec3 projDirection(0.0, 0.0, -1.0);
		if (glm::dot(thirdPoint - m_clickResults[0].position, projDirection) < 0)
			projDirection = -projDirection;
		std::vector<glm::dvec3> createdPointStart(0), createdPointEnd(0);
		std::vector<glm::dvec3> userPoints(0);
		userPoints.push_back(m_clickResults[0].position);
		userPoints.push_back(m_clickResults[1].position);
		userPoints.push_back(m_clickResults[2].position);
		userPoints.push_back(m_clickResults[3].position);
		TlScanOverseer::getInstance().setOfPointsWith4thPoint(projDirection, userPoints, m_step, m_threshold, createdPointStart, createdPointEnd, arePointsReal, cosAngleThreshold, clippingAssembly);
		//create points
		int pointToCreate(0);
		for (int i = 0; i < (int)createdPointStart.size(); i++)
		{
			if (arePointsReal[i])
				pointToCreate++;
		}

		std::vector<uint32_t> indexes = controller.getMultipleUserId(ElementType::Point, pointToCreate);
		std::vector<uint32_t> indexesMeasure = controller.getMultipleUserId(ElementType::SimpleMeasure, pointToCreate);
		int currentIndex(0);
		for (int i = 0; i < (int)createdPointStart.size(); i++)
		{
			if (arePointsReal[i])
			{
				if (!m_createMeasures)
				{
					SafePtr<PointNode> point = make_safe<PointNode>();
					WritePtr<PointNode> wPoint = point.get();
					if (!wPoint)
					{
						m_clickResults.clear();
						return waitForNextPoint(controller);
					}

					wPoint->setDefaultData(controller);
					wPoint->setPosition(createdPointEnd[i]);
					wPoint->setUserIndex(indexes[currentIndex]);
					currentIndex++;
					if (!controller.getContext().getActiveName().empty())
						wPoint->setName(controller.getContext().getActiveName());
					else
						wPoint->setName(L"Inter");
					pointsToCreate.insert(point);
					//controller.getControlListener()->notifyUIControl(new control::function::AddNodes({ point }));
				}
				
				if (m_createMeasures)
				{
					Measure newMeasure;
					newMeasure.origin = createdPointStart[i];
					newMeasure.final = createdPointEnd[i];

					SafePtr<SimpleMeasureNode> measure = make_safe<SimpleMeasureNode>();
					WritePtr<SimpleMeasureNode> wMeasure = measure.get();
					if (!wMeasure)
					{
						assert(false);
						m_clickResults.clear();
						return waitForNextPoint(controller);
					}

					wMeasure->setDefaultData(controller);
					wMeasure->setMeasure(newMeasure);
					wMeasure->setUserIndex(indexesMeasure[currentIndex]);
					currentIndex++;
					if (!controller.getContext().getActiveName().empty())
						wMeasure->setName(controller.getContext().getActiveName());
					else
						wMeasure->setName(L"Auto_Dist");
					measuresToCreate.insert(measure);
					//controller.getControlListener()->notifyUIControl(new control::function::AddNodes(measure));
				}
			}
		}

		//measure need to be created if option is checked

		break;
	}
	case SetOfPointsOptions::case7:
	{
		//get real 3rd point
		glm::dvec3 thirdPoint = m_clickResults[2].position;
		if (glm::dot(m_clickResults[1].position - m_clickResults[0].position, m_clickResults[3].position - m_clickResults[2].position) < 0)
			thirdPoint = m_clickResults[3].position;

		
		//find plane containing the 3 points, no user axis mode yet
		std::vector<glm::dvec3> planePoints(0);
		planePoints.push_back(m_clickResults[0].position);
		planePoints.push_back(m_clickResults[1].position);
		planePoints.push_back(m_clickResults[2].position);
		std::vector<double> plane(0);
		OctreeRayTracing::fitPlane(planePoints, plane);
		glm::dvec3 normal(plane[0], plane[1], plane[2]);
		glm::dvec3 vertical(0.0, 0.0, 1.0);
		//glm::dvec3 horizontal = glm::cross(normal, vertical);
		glm::dvec3 projDirection = glm::cross(normal, m_clickResults[1].position- m_clickResults[0].position);
		projDirection /= glm::length(projDirection);
		if (glm::dot(thirdPoint - m_clickResults[0].position, projDirection) < 0)
			projDirection = -projDirection;
		
		std::vector<glm::dvec3> createdPointStart(0), createdPointEnd(0);
		std::vector<glm::dvec3> userPoints(0);
		userPoints.push_back(m_clickResults[0].position);
		userPoints.push_back(m_clickResults[1].position);
		userPoints.push_back(m_clickResults[2].position);
		userPoints.push_back(m_clickResults[3].position);
		TlScanOverseer::getInstance().setOfPointsWith4thPoint(projDirection, userPoints, m_step, m_threshold, createdPointStart, createdPointEnd, arePointsReal, cosAngleThreshold, clippingAssembly);
		//create points
		int pointToCreate(0);
		for (int i = 0; i < (int)createdPointStart.size(); i++)
		{
			if (arePointsReal[i])
				pointToCreate++;
		}

		std::vector<uint32_t> indexes = controller.getMultipleUserId(ElementType::Point, pointToCreate);
		std::vector<uint32_t> indexesMeasure = controller.getMultipleUserId(ElementType::SimpleMeasure, pointToCreate);
		int currentIndex(0);

		for (int i = 0; i < (int)createdPointStart.size(); i++)
		{
			if (arePointsReal[i])
			{
				if (!m_createMeasures)
				{
					SafePtr<PointNode> point = make_safe<PointNode>();
					WritePtr<PointNode> wPoint = point.get();
					if (!wPoint)
					{
						m_clickResults.clear();
						return waitForNextPoint(controller);
					}

					wPoint->setDefaultData(controller);
					wPoint->setPosition(createdPointEnd[i]);
					wPoint->setUserIndex(indexes[currentIndex]);
					currentIndex++;
					if (!controller.getContext().getActiveName().empty())
						wPoint->setName(controller.getContext().getActiveName());
					else
						wPoint->setName(L"Inter");
					pointsToCreate.insert(point);
					//controller.getControlListener()->notifyUIControl(new control::function::AddNodes({ point }));
				}
				

				else
				{
					Measure newMeasure;
					newMeasure.origin = createdPointStart[i];
					newMeasure.final = createdPointEnd[i];

					SafePtr<SimpleMeasureNode> measure = make_safe<SimpleMeasureNode>();
					WritePtr<SimpleMeasureNode> wMeasure = measure.get();
					if (!wMeasure)
					{
						assert(false);
						m_clickResults.clear();
						return waitForNextPoint(controller);
					}

					wMeasure->setDefaultData(controller);
					wMeasure->setMeasure(newMeasure);
					wMeasure->setUserIndex(indexesMeasure[currentIndex]);
					currentIndex++;
					if (!controller.getContext().getActiveName().empty())
						wMeasure->setName(controller.getContext().getActiveName());
					else
						wMeasure->setName(L"Auto_Dist");
					measuresToCreate.insert(measure);
					//controller.getControlListener()->notifyUIControl(new control::function::AddNodes(measure));
				}
			}
		}

		break;
	}
	case SetOfPointsOptions::case8:
	{

		//get real 3rd point
		glm::dvec3 thirdPoint = m_clickResults[2].position;
		if (glm::dot(m_clickResults[1].position - m_clickResults[0].position, m_clickResults[3].position - m_clickResults[2].position) < 0)
			thirdPoint = m_clickResults[3].position;

		glm::dvec3 projDirection;
		if (!m_horizontal)
		{
			//find plane containing the 3 points, no user axis mode yet
			std::vector<glm::dvec3> planePoints(0);
			planePoints.push_back(m_clickResults[0].position);
			planePoints.push_back(m_clickResults[1].position);
			planePoints.push_back(thirdPoint);
			std::vector<double> plane(0);
			OctreeRayTracing::fitPlane(planePoints, plane);
			glm::dvec3 normal(plane[0], plane[1], plane[2]);
			projDirection = glm::cross(normal, m_clickResults[0].position - m_clickResults[1].position);
			projDirection /= glm::length(projDirection);

			if (glm::dot(projDirection, m_clickResults[2].position - m_clickResults[0].position) < 0)
				projDirection = -projDirection;

		}
		
		else
		{
			//projDirection should be horizontal, orthogonal to the first two points
			projDirection = glm::cross(glm::dvec3(0.0, 0.0, 1.0), m_clickResults[0].position - m_clickResults[1].position);
			projDirection /= glm::length(projDirection);
			if (glm::dot(projDirection, m_clickResults[2].position - m_clickResults[0].position) < 0)
				projDirection = -projDirection;
		}
		std::vector<glm::dvec3> createdPointStart(0), createdPointEnd(0);
		std::vector<glm::dvec3> userPoints(0);
		userPoints.push_back(m_clickResults[0].position);
		userPoints.push_back(m_clickResults[1].position);
		userPoints.push_back(m_clickResults[2].position);
		userPoints.push_back(m_clickResults[3].position);
		TlScanOverseer::getInstance().setOfPointsWith4thPoint(projDirection, userPoints, m_step, m_threshold, createdPointStart, createdPointEnd, arePointsReal, cosAngleThreshold, clippingAssembly);
		//create points
		int pointToCreate(0);
		for (int i = 0; i < (int)createdPointStart.size(); i++)
		{
			if (arePointsReal[i])
				pointToCreate++;
		}

		std::vector<uint32_t> indexes = controller.getMultipleUserId(ElementType::Point, pointToCreate);
		std::vector<uint32_t> indexesMeasure = controller.getMultipleUserId(ElementType::SimpleMeasure, pointToCreate);
		int currentIndex(0);

		for (int i = 0; i < (int)createdPointStart.size(); i++)
		{
			if (arePointsReal[i])
			{
				if (!m_createMeasures)
				{
					SafePtr<PointNode> point = make_safe<PointNode>();
					WritePtr<PointNode> wPoint = point.get();
					if (!wPoint)
					{
						m_clickResults.clear();
						return waitForNextPoint(controller);
					}

					wPoint->setDefaultData(controller);
					wPoint->setPosition(createdPointEnd[i]);
					wPoint->setUserIndex(indexes[currentIndex]);
					currentIndex++;
					if (!controller.getContext().getActiveName().empty())
						wPoint->setName(controller.getContext().getActiveName());
					else
						wPoint->setName(L"Inter");
					pointsToCreate.insert(point);
					//controller.getControlListener()->notifyUIControl(new control::function::AddNodes({ point }));
				}
				

				if (m_createMeasures)
				{
					Measure newMeasure;
					newMeasure.origin = createdPointStart[i];
					newMeasure.final = createdPointEnd[i];

					SafePtr<SimpleMeasureNode> measure = make_safe<SimpleMeasureNode>();
					WritePtr<SimpleMeasureNode> wMeasure = measure.get();
					if (!wMeasure)
					{
						assert(false);
						m_clickResults.clear();
						return waitForNextPoint(controller);
					}

					wMeasure->setDefaultData(controller);
					wMeasure->setMeasure(newMeasure);
					wMeasure->setUserIndex(indexesMeasure[currentIndex]);
					currentIndex++;
					if (!controller.getContext().getActiveName().empty())
						wMeasure->setName(controller.getContext().getActiveName());
					else
						wMeasure->setName(L"Auto_Dist");
					measuresToCreate.insert(measure);
					//controller.getControlListener()->notifyUIControl(new control::function::AddNodes(measure));
				}
			}
		}
		break;
	}
	}


	SafePtr<ClusterNode> clusterPoints = make_safe<ClusterNode>();
	{
		WritePtr<ClusterNode> wCluster = clusterPoints.get();
		wCluster->setTreeType(TreeType::Point);
		wCluster->setDefaultData(controller);
		wCluster->setName(L"Interpolated");
	}
	
	if (!pointsToCreate.empty())
	{
		controller.getControlListener()->notifyUIControl(new control::function::AddNodes({ clusterPoints }));
	}

	SafePtr<ClusterNode> clusterMeasures = make_safe<ClusterNode>();
	{
		WritePtr<ClusterNode> wCluster2 = clusterMeasures.get();
		wCluster2->setTreeType(TreeType::Measures);
		wCluster2->setDefaultData(controller);
		wCluster2->setName(L"Interpolated");
	}
	

	if (!measuresToCreate.empty())
	{
		controller.getControlListener()->notifyUIControl(new control::function::AddNodes({ clusterMeasures }));
	}

	for (SafePtr<AGraphNode> point : pointsToCreate)
	{
		AGraphNode::addOwningLink(clusterPoints, point);

		controller.getControlListener()->notifyUIControl(new control::function::AddNodes({ point }));
	}
	//controller.getControlListener()->notifyUIControl(new control::function::AddNodes(pointsToCreate));
	for (SafePtr<AGraphNode> measure : measuresToCreate)
	{
		AGraphNode::addOwningLink(clusterMeasures, measure);
		controller.getControlListener()->notifyUIControl(new control::function::AddNodes({ measure }));
	}
	//controller.getControlListener()->notifyUIControl(new control::function::AddNodes(measuresToCreate));

	resetClickUsages();
	return waitForNextPoint(controller);
}

bool ContextSetOfPoints::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextSetOfPoints::getType() const
{
	return (ContextType::setOfPoints);
}
