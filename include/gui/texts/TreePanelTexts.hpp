#ifndef TREE_PANEL_TEXTS_HPP
#define TREE_PANEL_TEXTS_HPP

#include <QtCore/qobject.h>

//ProjectTreePanel
#define TEXT_VIEWPOINTS_TREE_ROOT_NODE QObject::tr("Viewpoints List")
#define TEXT_VIEWPOINTS_TREE_NODE QObject::tr("Viewpoints")
#define TEXT_SCANS_TREE_NODE QObject::tr("Scans")
#define TEXT_SCANS_TREE_ROOT_NODE QObject::tr("Scans List")
#define TEXT_COLORS_TREE_NODE QObject::tr("Colors")
#define TEXT_PHASE_TREE_NODE QObject::tr("Phases")
#define TEXT_STATUS_TREE_NODE QObject::tr("Status")
#define TEXT_CLIPMETH_TREE_NODE QObject::tr("Clipping method")
#define TEXT_DISCIPLINE_TREE_NODE QObject::tr("Disciplines")
#define TEXT_STRUCT_TREE_NODE QObject::tr("Structure")
#define TEXT_TAGS_TREE_NODE QObject::tr("Tags")
#define TEXT_TAGS_TREE_ROOT_NODE QObject::tr("Tags List")
#define TEXT_ITEMS_TREE_NODE QObject::tr("Items")
#define TEXT_HIERARCHY_TREE_NODE QObject::tr("Hierarchy")
#define TEXT_AUTHORS_TREE_NODE QObject::tr("Authors")
#define TEXT_MEASURES_TREE_NODE QObject::tr("Measurements")
#define TEXT_MEASURES_TREE_ROOT_NODE QObject::tr("Measures List")
#define TEXT_BOXES_TREE_NODE QObject::tr("Boxes")
#define TEXT_BOXES_TREE_ROOT_NODE QObject::tr("Boxes List")
#define TEXT_POINT_TREE_NODE QObject::tr("Points")
#define TEXT_POINT_TREE_ROOT_NODE QObject::tr("Points List")
#define TEXT_PC_OBJECT_TREE_NODE QObject::tr("Point Cloud Objects")
#define TEXT_PC_OBJECT_TREE_ROOT_NODE QObject::tr("PCO List")
#define TEXT_OBJ_OBJECT_TREE_NODE QObject::tr("Imported Models")
#define TEXT_OBJ_OBJECT_TREE_ROOT_NODE QObject::tr("Models List")
#define TEXT_PIPES_TREE_NODE QObject::tr("Piping")
#define TEXT_PIPES_TREE_ROOT_NODE QObject::tr("Piping List")
#define TEXT_PIPING_TREE_ROOT_NODE QObject::tr("Piping Lines")
#define TEXT_SPHERES_TREE_NODE QObject::tr("Spheres")
#define TEXT_SPHERES_TREE_ROOT_NODE QObject::tr("Spheres List")
#define TEXT_MESHOBJECT_TREE_ROOT_NODE QObject::tr("External Model")

#define TEXT_SIMPLE_BOX_SUB_NODE QObject::tr("Box")
#define TEXT_GRID_SUB_NODE QObject::tr("Grid")
#define TEXT_CYLINDER_SUB_NODE QObject::tr("Sections")
#define TEXT_TORUS_SUB_NODE QObject::tr("Elbows")

#define TEXT_CONTEXT_NEW_CLUSTER QObject::tr("New cluster")
#define TEXT_CONTEXT_DELETE_CLUSTER_ITEM QObject::tr("Delete cluster and items")
#define TEXT_CONTEXT_DELETE_CLUSTER_REMOVE_ITEM QObject::tr("Delete cluster - remove items from hierarchy")
#define TEXT_CONTEXT_REMOVE_ITEM QObject::tr("Remove item from hierarchy")
#define TEXT_CONTEXT_MOVE_TO_SCAN QObject::tr("Move to scan")
#define TEXT_CONTEXT_DELETE_SCAN QObject::tr("Delete scan")
#define TEXT_CONTEXT_EXPORT_SCAN QObject::tr("Export scan")
#define TEXT_CONTEXT_MOVE_TO_ITEM QObject::tr("Move to item")
#define TEXT_CONTEXT_PICK_ITEMS QObject::tr("Pick selected")
#define TEXT_CONTEXT_DROP_ITEMS QObject::tr("Drop selected")
#define TEXT_CONTEXT_SELECT_ITEMS QObject::tr("Select children")
#define TEXT_CONTEXT_DELETE_ITEM QObject::tr("Delete item")
#define TEXT_CONTEXT_CHANGE_ATTRIBUTES QObject::tr("Change attributes")
#define TEXT_CONTEXT_ENABLE_CLIPPING QObject::tr("Activate")
#define TEXT_CONTEXT_DISABLE_CLIPPING QObject::tr("Disable")
#define TEXT_CONTEXT_EXTERIOR_CLIPPING QObject::tr("Show exterior")
#define TEXT_CONTEXT_INTERIOR_CLIPPING QObject::tr("Show interior")
#define TEXT_CONTEXT_DISABLE_ALL_CLIPPING QObject::tr("Desactivate all active clippings")
#define TEXT_CONTEXT_DISABLE_ALL_RAMPS QObject::tr("Desactivate all active ramps")
#define TEXT_CONTEXT_DISCONNECT_LINE QObject::tr("Disconnect the line")
#define TEXT_TOOLTIP_DISCONNECT_LINE QObject::tr("Delete accessories but keep straight sections")
#define TEXT_CONTEXT_SHOW_RAMP_RENDERING QObject::tr("Show ramp rendering")

#define TEXT_NEW_CLUSTER QObject::tr("New cluster")
#define TEXT_DELETE_CLUSTER QObject::tr("Delete cluster")
#define TEXT_MOVE_TO_SCAN QObject::tr("Move to scan") //voir pour rendre cela générique avec "Move to item" pour chaque objet ou scan.
#define TEXT_DELETE_SCAN QObject::tr("Delete scan") //proposer 2 choix en menu contextuel: "Remove scan(s) from project but keep files" ; "Remove scan(s) from project and delete files"
#define TEXT_MOVE_TO_TAG QObject::tr("Move to item") //voir pour rendre cela générique avec "Move to item" pour chaque objet ou scan.
#define TEXT_DELETE_TAG QObject::tr("Delete tag") //voir pour rendre cela générique avec "Delete" tout court.
#define TEXT_DELETE_MEASURE QObject::tr("Delete measure") //voir pour rendre cela générique avec "Delete" tout court.
#define TEXT_DELETE_CLIPPING QObject::tr("Delete box") //voir pour rendre cela générique avec "Delete" tout court.
#define TEXT_DELETE_POINT QObject::tr("Delete point") //voir pour rendre cela générique avec "Delete" tout court.
#define TEXT_SIMPLE_MEASURE QObject::tr("Simple") //suppression de measurement (le dossier parent s'appelle déjà Measurement)
#define TEXT_POLYLINE_MEASURE QObject::tr("Polyline") //suppression de measurement (le dossier parent s'appelle déjà Measurement)
#define TEXT_BEAMBENDING_MEASURE QObject::tr("Beam Bending") //suppression de measurement (le dossier parent s'appelle déjà Measurement)
#define TEXT_COLUMNTILT_MEASURE QObject::tr("Column Tilt") //suppression de measurement (le dossier parent s'appelle déjà Measurement)
#define TEXT_PIPETOPIPE_MEASURE QObject::tr("Pipe to Pipe") //suppression de measurement (le dossier parent s'appelle déjà Measurement)
#define TEXT_PIPETOPLANE_MEASURE QObject::tr("Pipe to Plane") //suppression de measurement (le dossier parent s'appelle déjà Measurement)
#define TEXT_POINTTOPLANE_MEASURE QObject::tr("Point to Plane") //suppression de measurement (le dossier parent s'appelle déjà Measurement)
#define TEXT_POINTTOPIPE_MEASURE QObject::tr("Point to Pipe") //suppression de measurement (le dossier parent s'appelle déjà Measurement)

#define TEXT_PLACEHOLDER_CHILDREN QObject::tr("Loading children...")


#endif