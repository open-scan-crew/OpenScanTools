#include "gui/widgets/PropertyPolylineMeasure.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "controller/Controller.h"
#include "controller/controls/ControlDataEdition.h"
#include "controller/controls/ControlClippingEdition.h"
#include "gui/Texts.hpp"
#include "gui/UnitConverter.h"

#include "models/graph/PolylineMeasureNode.h"

PropertyPolylineMeasure::PropertyPolylineMeasure(Controller& controller, QWidget* parent, float guiScale)
	: APropertyGeneral(controller.getDataDispatcher(), parent)
	, m_unitUsage(unit_usage::by_default)
{
	m_ui.setupUi(this);
	m_ui.genericPropsHeadWidget->setControllerInfo(controller);
	m_ui.genericPropsFeetWidget->setDataDispatcher(controller.getDataDispatcher());
	m_ui.subPropertyClipping->setDataDispatcher(&controller.getDataDispatcher());

	m_dataDispatcher.registerObserverOnKey(this, guiDType::renderValueDisplay);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendAuthorsList);

	m_polylineMethods.insert({ guiDType::renderValueDisplay, &PropertyPolylineMeasure::onRenderUnitUsage });

	m_ui.TotalInfield->setType(NumericType::DISTANCE);
	m_ui.TotalHorizontalInfield->setType(NumericType::DISTANCE);
	m_ui.HorizontalInfield->setType(NumericType::DISTANCE);
	m_ui.VerticalInfield->setType(NumericType::DISTANCE);

	resizeEvent(nullptr);
}

PropertyPolylineMeasure::~PropertyPolylineMeasure()
{
	m_dataDispatcher.unregisterObserver(this);
}

void PropertyPolylineMeasure::informData(IGuiData* data)
{
	APropertyGeneral::informData(data);
	if (m_polylineMethods.find(data->getType()) != m_polylineMethods.end())
	{
		PropertyPolylineMethod method = m_polylineMethods.at(data->getType());
		(this->*method)(data);
	}
}

bool PropertyPolylineMeasure::actualizeProperty(SafePtr<AGraphNode> object )
{
	if (object)
		m_measure = static_pointer_cast<PolylineMeasureNode>(object);

	m_ui.genericPropsHeadWidget->setObject(m_measure);
	m_ui.genericPropsFeetWidget->setObject(m_measure);
	m_ui.subPropertyClipping->setObject(m_measure);

	return updateMeasure();
}

void PropertyPolylineMeasure::onRenderUnitUsage(IGuiData* data)
{
	m_unitUsage = static_cast<GuiDataRenderUnitUsage*>(data)->m_valueParameters;
	updateUI();
}

bool PropertyPolylineMeasure::updateMeasure()
{
	ReadPtr<PolylineMeasureNode> node = m_measure.cget();

	if (!node || node->getMeasures().empty())
		return false;

	Measure hV = { node->getFirstPos(), node->getLastPos() };
	glm::dvec3 polylineArea = node->computeAreaOfPolyline();

	auto valueDisplay = [&](double t) { return QString::number(unit_converter::meterToX(t, m_unitUsage.distanceUnit), 'f', m_unitUsage.displayedDigits); };
	auto valueSquaredDisplay = [&](double t) { return QString::number(unit_converter::meterToX(unit_converter::meterToX(t, m_unitUsage.distanceUnit), m_unitUsage.distanceUnit), 'f', m_unitUsage.displayedDigits); };

	m_ui.HorizontalInfield->setValue(hV.getDistanceHorizontal());
	m_ui.VerticalInfield->setValue(hV.getDistanceAlongZ());

	m_ui.areaTableWidget->setRowCount(1);
	m_ui.areaTableWidget->setItem(0, 0, new QTableWidgetItem(valueSquaredDisplay(polylineArea.z)));
	m_ui.areaTableWidget->setItem(0, 1, new QTableWidgetItem(valueSquaredDisplay(polylineArea.x)));
	m_ui.areaTableWidget->setItem(0, 2, new QTableWidgetItem(valueSquaredDisplay(polylineArea.y)));

	m_ui.PointsCoordinatesTableWidget->clear();
	m_ui.PointsCoordinatesTableWidget->setColumnCount(4);
	m_ui.PointsCoordinatesTableWidget->setRowCount((int)node->getMeasures().size() + 1);
	QStringList m_TableHeaderPoint;
	m_TableHeaderPoint << "" << "X" << "Y" << "Z";	m_ui.PointsCoordinatesTableWidget->setHorizontalHeaderLabels(m_TableHeaderPoint);

	m_ui.SegmentsTableWidget->clear();
	m_ui.SegmentsTableWidget->setColumnCount(4);
	m_ui.SegmentsTableWidget->setRowCount((int)node->getMeasures().size());
	QStringList m_TableHeaderSeg;
	m_TableHeaderSeg << "" << TEXT_POLYLINEPROPERTYPANEL_SEGMENT_TABLE_TOTAL << TEXT_POLYLINEPROPERTYPANEL_SEGMENT_TABLE_HOR << TEXT_POLYLINEPROPERTYPANEL_SEGMENT_TABLE_VERT;
	m_ui.SegmentsTableWidget->setHorizontalHeaderLabels(m_TableHeaderSeg);

	resizeEvent(nullptr);

	double total(0.0);
	double totalH(0.0);
	uint32_t count(1);
	QLabel* point, * x, * y, * z, * seg, * t, * h, * v;
	point = x = y = z = seg = t = h = v = nullptr;

	point = new QLabel(m_ui.PointsCoordinatesTableWidget);
	x = new QLabel(m_ui.PointsCoordinatesTableWidget);
	y = new QLabel(m_ui.PointsCoordinatesTableWidget);
	z = new QLabel(m_ui.PointsCoordinatesTableWidget);

	point->setText(QString(TEXT_POLYLINEPROPERTYPANEL_POINT_TABLE_PT).arg(count));
	const Measure firstm = node->getMeasures().front();
	x->setText(valueDisplay(firstm.origin.x));
	y->setText(valueDisplay(firstm.origin.y));
	z->setText(valueDisplay(firstm.origin.z));

	m_ui.PointsCoordinatesTableWidget->setCellWidget(0, 0, point);
	m_ui.PointsCoordinatesTableWidget->setCellWidget(0, 1, x);
	m_ui.PointsCoordinatesTableWidget->setCellWidget(0, 2, y);
	m_ui.PointsCoordinatesTableWidget->setCellWidget(0, 3, z);

	for (const Measure& measure : node->getMeasures())
	{
		total += measure.getDistanceTotal();
		totalH += measure.getDistanceHorizontal();

		point = new QLabel(m_ui.PointsCoordinatesTableWidget);
		x = new QLabel(m_ui.PointsCoordinatesTableWidget);
		y = new QLabel(m_ui.PointsCoordinatesTableWidget);
		z = new QLabel(m_ui.PointsCoordinatesTableWidget);

		point->setText(QString(TEXT_POLYLINEPROPERTYPANEL_POINT_TABLE_PT).arg(count + 1));

		x->setText(valueDisplay(measure.final.x));
		y->setText(valueDisplay(measure.final.y));
		z->setText(valueDisplay(measure.final.z));

		m_ui.PointsCoordinatesTableWidget->setCellWidget(count, 0, point);
		m_ui.PointsCoordinatesTableWidget->setCellWidget(count, 1, x);
		m_ui.PointsCoordinatesTableWidget->setCellWidget(count, 2, y);
		m_ui.PointsCoordinatesTableWidget->setCellWidget(count, 3, z);

		seg = new QLabel(m_ui.PointsCoordinatesTableWidget);
		t = new QLabel(m_ui.PointsCoordinatesTableWidget);
		h = new QLabel(m_ui.PointsCoordinatesTableWidget);
		v = new QLabel(m_ui.PointsCoordinatesTableWidget);

		seg->setText(QString(TEXT_POLYLINEPROPERTYPANEL_SEGMENT_TABLE_SEG).arg(count));

		t->setText(valueDisplay(measure.getDistanceTotal()));
		h->setText(valueDisplay(measure.getDistanceHorizontal()));
		v->setText(valueDisplay(measure.getDistanceAlongZ()));

		m_ui.SegmentsTableWidget->setCellWidget(count - 1, 0, seg);
		m_ui.SegmentsTableWidget->setCellWidget(count - 1, 1, t);
		m_ui.SegmentsTableWidget->setCellWidget(count - 1, 2, h);
		m_ui.SegmentsTableWidget->setCellWidget(count - 1, 3, v);

		count++;
	}

	m_ui.TotalInfield->setValue(total);
	m_ui.TotalHorizontalInfield->setValue(totalH);
	resizeEvent(nullptr);

	return true;
}

void PropertyPolylineMeasure::updateUI()
{
	m_ui.AreaLabelSquaredBracket->setText('(' + unit_converter::getUnitText(m_unitUsage.distanceUnit) + '²' + ')');
	m_ui.SegmentsUnitWithBracketLabel->setText('(' + unit_converter::getUnitText(m_unitUsage.distanceUnit) + ')');
	m_ui.CoordinatesUnitWithBracketLabel->setText('(' + unit_converter::getUnitText(m_unitUsage.distanceUnit) + ')');

	updateMeasure();
	
}

void PropertyPolylineMeasure::resizeEvent(QResizeEvent* event)
{
	m_ui.PointsCoordinatesTableWidget->setColumnWidth(0, m_ui.PointsCoordinatesTableWidget->width() / 4);
	m_ui.PointsCoordinatesTableWidget->setColumnWidth(1, m_ui.PointsCoordinatesTableWidget->width() / 4);
	m_ui.PointsCoordinatesTableWidget->setColumnWidth(2, m_ui.PointsCoordinatesTableWidget->width() / 4);
	m_ui.PointsCoordinatesTableWidget->setColumnWidth(3, m_ui.PointsCoordinatesTableWidget->width() / 4);

	m_ui.SegmentsTableWidget->setColumnWidth(0, m_ui.SegmentsTableWidget->width() / 4);
	m_ui.SegmentsTableWidget->setColumnWidth(1, m_ui.SegmentsTableWidget->width() / 4);
	m_ui.SegmentsTableWidget->setColumnWidth(2, m_ui.SegmentsTableWidget->width() / 4);
	m_ui.SegmentsTableWidget->setColumnWidth(3, m_ui.SegmentsTableWidget->width() / 4);

	m_ui.areaTableWidget->setColumnWidth(0, m_ui.areaTableWidget->width() / 3);
	m_ui.areaTableWidget->setColumnWidth(1, m_ui.areaTableWidget->width() / 3);
	m_ui.areaTableWidget->setColumnWidth(2, m_ui.areaTableWidget->width() / 3);
}

