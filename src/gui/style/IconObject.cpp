#include "gui/style/IconObject.h"
#include "services/MarkerDefinitions.hpp"
#include <QPainter>
#include <QImage>
#include <QGraphicsEffect>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

#include "utils/Utils.h"

QImage tint(QImage src, QColor color, qreal strength = 1.0) {
	if (src.isNull())
		return QImage();
	QGraphicsScene scene;
	QGraphicsPixmapItem item;
	item.setPixmap(QPixmap::fromImage(src));
	QGraphicsColorizeEffect effect;
	effect.setColor(color);
	effect.setStrength(strength);
	item.setGraphicsEffect(&effect);
	scene.addItem(&item);
	QImage res(src);
	QPainter ptr(&res);
	scene.render(&ptr, QRectF(), src.rect());
	return res;
}

QPixmap scs::IconManager::getIcon(ElementType type, scs::MarkerIcon icon, QColor color)
{
	static std::unordered_map<ElementType, QImage> s_treeIcons =
	{
		{ ElementType::Torus, QImage(":/icons/arbo/elbow.png") },
		{ ElementType::Sphere, QImage(":/icons/arbo/sphere.png") },
		{ ElementType::Cylinder, QImage(":/icons/arbo/pipe.png") },
		{ ElementType::Box, QImage(":/icons/arbo/Ortho-yk.png") },
		{ ElementType::Grid, QImage(":/icons/arbo/gridded box-yk.png") },
		//{ ElementType::Cluster,  });
		//{ ElementType::MasterCluster,  });
		{ ElementType::Scan, QImage(":/icons/arbo/survey_equipment_100.png") },
		{ ElementType::Point, QImage(":/icons/arbo/point.png") },
		{ ElementType::SimpleMeasure, QImage(":/icons/arbo/measure_points.png") },
		{ ElementType::PolylineMeasure, QImage(":/icons/arbo/polyline.png") },
		{ ElementType::BeamBendingMeasure, QImage(":/icons/arbo/beam_bending.png") },
		{ ElementType::ColumnTiltMeasure, QImage(":/icons/arbo/column tilt.png") },
		{ ElementType::PointToPlaneMeasure, QImage(":/icons/arbo/point_plane_measurement.png") },
		{ ElementType::PipeToPipeMeasure, QImage(":/icons/arbo/cylinder_cylinder_measurement.png") },
		{ ElementType::PointToPipeMeasure, QImage(":/icons/arbo/cylinder_point_measurement.png") },
		{ ElementType::PipeToPlaneMeasure, QImage(":/icons/arbo/cylinder_plane_measurement.png") },
		{ ElementType::PCO, QImage(":/icons/arbo/global_box.png") },
		{ ElementType::ViewPoint, QImage(":/icons/arbo/viewpoint.png") },
		{ ElementType::MeshObject, QImage(":/icons/arbo/3dmodel.png") }
	};

	static std::unordered_map<scs::MarkerIcon, QImage> s_tagTreeIcons =
	{
		{ scs::MarkerIcon::Tag_Base, QImage(":/icons/arbo/tag_base.png") },
		{ scs::MarkerIcon::Tag_Attention, QImage(":/icons/arbo/tag_attention.png") },
		{ scs::MarkerIcon::Tag_Cone, QImage(":/icons/arbo/tag_base.png") },
		{ scs::MarkerIcon::Tag_Delete, QImage(":/icons/arbo/tag_cone.png") },
		{ scs::MarkerIcon::Tag_Drop, QImage(":/icons/arbo/tag_drop.png") },
		{ scs::MarkerIcon::Tag_Flag, QImage(":/icons/arbo/tag_flag.png") },
		{ scs::MarkerIcon::Tag_Information, QImage(":/icons/arbo/tag_information.png") },
		{ scs::MarkerIcon::Tag_Instrumentation, QImage(":/icons/arbo/tag_instrumentation.png") },
		{ scs::MarkerIcon::Tag_Recycle, QImage(":/icons/arbo/tag_recycle.png") },
		{ scs::MarkerIcon::Tag_Repair, QImage(":/icons/arbo/tag_repair.png") },
		{ scs::MarkerIcon::Tag_Tick, QImage(":/icons/arbo/tag_tick.png") },
		{ scs::MarkerIcon::Tag_Trashcan, QImage(":/icons/arbo/tag_trashcan.png") }
	};


	uint64_t hashIcon = getHash(type, icon, color);
	if (m_hashmapIcons.find(hashIcon) != m_hashmapIcons.end())
		return m_hashmapIcons.at(hashIcon);

	QPixmap pixmap = QPixmap();

	if (s_treeIcons.find(type) != s_treeIcons.end())
	{
		QImage iconImage = s_treeIcons.at(type);
		pixmap = QPixmap::fromImage(tint(iconImage, color, 1.0));
	}

	if (s_tagTreeIcons.find(icon) != s_tagTreeIcons.end())
	{
		QImage iconImage = s_tagTreeIcons.at(icon);
		pixmap = QPixmap::fromImage(tint(iconImage, color, 1.0));
	}
	else if (scs::markerStyleDefs.find(icon) != scs::markerStyleDefs.end())
		pixmap = QPixmap::fromImage(QImage(scs::markerStyleDefs.at(icon).qresource));

	if (!pixmap.isNull())
		m_hashmapIcons[hashIcon] = pixmap;

	return pixmap;
}

uint64_t scs::IconManager::getHash(ElementType type, scs::MarkerIcon icon, QColor color)
{
	uint64_t hash = 0;
	
	std::hash<int> hash_int;

	hash = hash_int((int)type);
	hash = Utils::hash_combine(hash, hash_int((int)icon));

	hash = Utils::hash_combine(hash, hash_int(color.red()));
	hash = Utils::hash_combine(hash, hash_int(color.blue()));
	hash = Utils::hash_combine(hash, hash_int(color.green()));

	return hash;
}
