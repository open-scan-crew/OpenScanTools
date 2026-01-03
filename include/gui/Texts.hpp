#ifndef TEXTS_HPP
#define TEXTS_HPP

#include <QtCore/qobject.h>

// Basic Wording
#define TEXT_WARNING QObject::tr("Warning!")
#define TEXT_INFORMATION QObject::tr("Information")
#define TEXT_QUESTION QObject::tr("Question")

//SaveLoadSystem

#define TEXT_ERROR_NO_TLS_PATH QObject::tr("Error : TLS path not found on : %1")
#define TEXT_CANT_LOAD_PROJECT QObject::tr("Error : The project cannot be loaded.")
#define TEXT_CANT_LOAD_PROJECT_NO_AUTHOR QObject::tr("Error : The project can't be loaded.\r\nThe tld file : [%1] do not have author.")
#define TEXT_CANT_LOAD_TAG QObject::tr("Error : The tag cannot be loaded.")
#define TEXT_CANT_LOAD_TAG_NO_AUTHOR QObject::tr("Error : The tag can't be loaded.\r\nThe tld file : [%1] do not have author.")
#define TEXT_TEMPLATE_NOT_REGISTERED QObject::tr("Error : the file contain at least one template that is not registered.")
#define TEXT_TEMPLATE_INVALID_TAG QObject::tr("Error : name or userId data in tag not found.")
#define TEXT_CANT_LOAD_PROJECT_TLP_ERROR QObject::tr("The tlp project file may contains an error or has been removed or the filepath contains unrecognized caracter.")
#define TEXT_CANT_LOAD_PROJECT_OUTDATED_SAVELOADSYTEM QObject::tr("This project can't be opened with this version, please download and install the latest version first")
#define TEXT_TEMPLATE_INVALID_MEASURE QObject::tr("Error : name data in measure not found.")
#define TEXT_TEMPLATE_INVALID_CLIPPING QObject::tr("Error : name data in boxes not found.")
#define TEXT_TEMPLATE_INVALID_PIPE QObject::tr("Error : name data in pipe not found.")
#define TEXT_TEMPLATE_INVALID_POINT QObject::tr("Error : name data in point not found.")
#define TEXT_TEMPLATE_INVALID_PCO QObject::tr("Error : position data in pipe not found.")
#define TEXT_TEMPLATE_INVALID_MESHOBJECT_PATH QObject::tr("Error : path data in pipe not found.")
#define TEXT_TEMPLATE_INVALID_MESHOBJECT_NAME QObject::tr("Error : name data in pipe not found.")
#define TEXT_TEMPLATE_INVALID_MESHOBJECT_POSITION QObject::tr("Error : name data in pipe not found.")
#define TEXT_TEMPLATE_INVALID_USER_ORIENTATION QObject::tr("Error : user orientation data not found.")
#define TEXT_LOAD_FAILED_FILE_NOT_FOUND QObject::tr("Failed to load %1.\nFile not found : \n%2")

//StatusPanel.cpp
#define TEXT_CAMERA QObject::tr("Camera")
#define TEXT_MOUSE QObject::tr("Mouse")
#define TEXT_DISTANCE QObject::tr("Distance")
#define TEXT_MODE QObject::tr("Mode")
#define TEXT_SCAN QObject::tr("Scan")
#define TEXT_CURRENT_FUNCTION QObject::tr("Current function")

//ScansPanel
#define TEXT_SCAN_NAME QObject::tr("Scan name")
#define TEXT_SCANNER_MODEL QObject::tr("Scanner model")
#define TEXT_SCANNER_SERIAL_NUMBER QObject::tr("Scanner serial number")
#define TEXT_ACQUISITION_DATE QObject::tr("Acquisition date")
#define TEXT_DESCRIPTION QObject::tr("Description")
#define TEXT_RGB QObject::tr("RGB")
#define TEXT_INTENSITY QObject::tr("Intensity")
#define TEXT_POINTS_NUMBER QObject::tr("Number of points")
#define TEXT_IMPORT_DATE QObject::tr("Import date")
#define TEXT_POSTION QObject::tr("Position")
#define TEXT_ROTATION QObject::tr("Rotation")

//ClustersPanel
#define TEXT_CLUSTER_NAME QObject::tr("Cluster name")
#define TEXT_CLUSTER_USER QObject::tr("User")
#define TEXT_COLOR  QObject::tr("Color") // TEXT_OBJECT_PROPERTY_COLOR
#define TEXT_CLUSTER_NAME_ALREADY_EXIST QObject::tr("Name already taken!")

//PolylinePropertyPanel
#define TEXT_POLYLINEPROPERTYPANEL_POINT_TABLE_PT QObject::tr("Point %1")
#define TEXT_POLYLINEPROPERTYPANEL_SEGMENT_TABLE_SEG QObject::tr("D%1")
#define TEXT_POLYLINEPROPERTYPANEL_SEGMENT_TABLE_TOTAL QObject::tr("Total")
#define TEXT_POLYLINEPROPERTYPANEL_SEGMENT_TABLE_HOR QObject::tr("Hor.")
#define TEXT_POLYLINEPROPERTYPANEL_SEGMENT_TABLE_VERT QObject::tr("Vert.")

//Colors
#define TEXT_BLUE QObject::tr("Blue")
#define TEXT_GREEN QObject::tr("Green")
#define TEXT_RED QObject::tr("Red")
#define TEXT_ORANGE QObject::tr("Orange")
#define TEXT_YELLOW QObject::tr("Yellow")
#define TEXT_PURPLE QObject::tr("Purple")
#define TEXT_LIGHT_GREY QObject::tr("Light Grey")
#define TEXT_BROWN QObject::tr("Brown")

//ClippingStatus
#define TEXT_ACTIVE QObject::tr("Active")
#define TEXT_INACTIVE QObject::tr("Inactive")

//ClippingMethod
#define TEXT_CLIPINTERN QObject::tr("Show interior")
#define TEXT_CLIPEXTERN QObject::tr("Show exterior")

//ClippingControl
#define TEXT_ACTIVE_CLIPPING_RAMP_MAX_REACHED QObject::tr("You can't activate more clippings or rampes, the maximum is reached (max: %1).")

//ControlPorject
#define TEXT_NO_PROJECT_LOADED QObject::tr("No project loaded")

//ControlTemplateEdit
#define TEXT_EDIT_TEMPLATE_NOT_EXIST QObject::tr("Error : the template you try to edit does not exist.") //signification? Comment peut-on essayer d'éditer quelque chose d'inexistant?
#define TEXT_TAG_NAME_TEMPLATE_ALREADY_TAKEN QObject::tr("Error : tag template name already used.")
#define TEXT_EDIT_TAG_TEMPLATE_NOT_EXIST QObject::tr("Error : you tried to edit a tag template that doesn't exists anymore.") //signification?
#define TEXT_DELETE_TAG_TEMPLATE_WITH_TAGS QObject::tr("Error : you tried to delete a tag template that is used in the current project. Remove the tags before deleting the template.") 
#define TEXT_DELETE_TAG_TEMPLATE_NOT_EXIST QObject::tr("Error : you tried to delete a tag template that doesn't exist anymore.") //signification?
#define TEXT_CANCELED_DELETE_TAG_TEMPLATE_ALREADY_EXIST QObject::tr("Error : you tried to cancel the deletion of a tag template that is already present in your template list.") //signification?
#define TEXT_DUPLICATED_TAG_TEMPLATE_ALREADY_EXIST QObject::tr("Error : you tried to duplicate a tag template that already exists.") //signification?
#define TEXT_TEMPLATE_DUPLICATE_UNDO_FAILED QObject::tr("Error : Undo failed : this template does not exist.") //signification?
#define TEXT_TEMPLATE_RENAME_FAILED QObject::tr("Error : you tried to rename a template that does not exist anymore.") //signification?
#define TEXT_TEMPLATE_ALREADY_PRESENT QObject::tr("This template already exists under the name %1.")


// TODO  - Remove _TITLE_ , _NAME_,
//NewProjectDialog
#define TEXT_PROJECT_CREATION QObject::tr("Project Creation")
#define TEXT_ENTER_PROJECT_NAME QObject::tr("Enter project name")
#define TEXT_ENTER_PROJECT_DIRECTORY QObject::tr("Enter project folder") //directory renommé en folder, plus couramment utilisé
#define TEXT_CREATE_PROJECT QObject::tr("Create Project")
#define TEXT_CANCEL QObject::tr("Cancel")

//ConvertionOptionsBox
#define TEXT_CONVERTION_TITLE QObject::tr("Conversion Options")
#define TEXT_PROJECT_ERROR_TITLE QObject::tr("Error : cannot create project.")
#define TEXT_PROJECT_ERROR_NOT_EMPTY QObject::tr("Error : project name and project folder are mandatory fields.") //directory renommé en folder, plus couramment utilisé

//TemplatePropertiesPanel
#define TEXT_NAME QObject::tr("Name")
#define TEXT_SHAPE QObject::tr("Shape")
#define TEXT_POS QObject::tr("Position")
#define TEXT_TAG_INDEX QObject::tr("Index")
#define TEXT_DISCPLINE QObject::tr("Discipline")
#define TEXT_PHASE QObject::tr("Phase")
#define TEXT_ID QObject::tr("Identifier") //ID réactualisé en Identifier. Auparavant, cela s'appelait prefix. Pour mémoire, ce champ est un générique
#define TEXT_TAG_LINKS QObject::tr("Links")
#define TEXT_TAG_ADDLINK QObject::tr("add hyperlink")
#define TEXT_TAG_REMOVELINK QObject::tr("delete hyperlink")

//ControlPCObject
#define TEXT_SCAN_IMPORT_AS_OBJECT_FAILED QObject::tr("Failed to create the point cloud object.")

// TemplateManager
#define TEXT_EXPORT_TEMPLATE QObject::tr("Export tag template")
#define TEXT_IMPORT_TEMPLATE QObject::tr("Import tag template")

// ToolBarImportObjects
#define TEXT_IMPORT_SHARED_OBJECTS QObject::tr("Import shared objects")
#define TEXT_IMPORT_TYPE_TLD QObject::tr("Tags (*.tld)")
#define TEXT_IMPORT_TYPE_TLO QObject::tr("Objects (*.tlo)")
#define TEXT_IMPORT_TYPE_TLV QObject::tr("Viewpoints (*.tlv)")
#define TEXT_IMPORT_TYPE_ALL_OBJECTS QObject::tr("All objects %1")
#define TEXT_IMPORT_TYPE_ALL_OBJECTS_OPEN TEXT_IMPORT_TYPE_ALL_OBJECTS.arg("(*.tld *.tlo *.tlv)") + ";;" \
	+ TEXT_IMPORT_TYPE_TLD  + ";;" \
	+ TEXT_IMPORT_TYPE_TLO + ";;" \
	+ TEXT_IMPORT_TYPE_TLV
#define TEXT_IMPORT_SCANTRA QObject::tr("Import Scantra registration")
#define TEXT_IMPORT_SCANTRA_FILES QObject::tr("Scantra database (*.scdb)")

//PropertyClippingSettings.cpp
#define TEXT_POINT_PICKING QObject::tr("Ready for point picking.")
#define TEXT_POINT_PICKING_REJECTED QObject::tr("Point rejected.")
#define TEXT_POINT_PICKING_DONE QObject::tr("Measure done.")
#define TEXT_BOXES_SETTINGS_PROPERTIES_NAME QObject::tr("Boxes creation settings")
#define TEXT_3DMODEL_SETTINGS_PROPERTIES_NAME QObject::tr("3D Model duplication settings")
#define TEXT_PCO_SETTINGS_PROPERTIES_NAME QObject::tr("Point cloud object duplication settings")

//PropertyUserOrientation.cpp
#define TEXT_USER_ORIENTATION_PROPERTIES_WARNING_NAME_ALREADY_USED QObject::tr("This orientation name already exists")
#define TEXT_USER_ORIENTATION_PROPERTIES_WARNING_NO_NAME QObject::tr("Please give a name to the orientation")
#define TEXT_USER_ORIENTATION_PROPERTIES_WARNING_MISSING_POINT QObject::tr("Points are missing. The orientation cannot be created")
#define TEXT_USER_ORIENTATION_PROPERTIES_SAME_POINT_AXIS QObject::tr("Same %1 points. Axis is null.")
#define TEXT_USER_ORIENTATION_PROPERTIES_NAN_VEC QObject::tr("Incorrect %1 point")


//PropertyPointCloud
#define TEXT_COLOR QObject::tr("Color")
#define TEXT_ADD_ANIMATION_KEYPOINT QObject::tr("Add as a key point")

//MultiProperty
#define TEXT_MULTI_ATTRIBUTES_CHANGE_QUESTION QObject::tr("You will replace data of selected objets.\nDo you confirm?")
#define TEXT_MULTI_ATTRIBUTES_CHANGE_TITLE QObject::tr("Replace data ?")

//ControlIO
#define TEXT_TAG_TO_PATH_ERROR QObject::tr("Error : incorrect file path (you may have specified a folder with administrator rights).")
#define TEXT_SCANTRA_NOT_FOUND_SCANS QObject::tr("Some transformations could not be applied, please read the report.")

//ControlProject
#define TEXT_PROJECT_CLOSING QObject::tr(" Closing Project...")
#define TEXT_PROJECT_ERROR_LOADING QObject::tr("Failed to load project:\n%1\n")
#define TEXT_PROJECT_ERROR_CENTRAL QObject::tr("You are trying to open a central project !")

//TemplateEditorDialog
#define TEXT_REFERENCE QObject::tr("Reference")
#define TEXT_TYPE QObject::tr("Type")
#define TEXT_DEFAULT_VALUE QObject::tr("Default value")
#define TEXT_TITLE_NEW_FIELD QObject::tr("New Field")
#define TEXT_TITLE_FIELD_REMOVAL_BOX QObject::tr("Field Removal")
#define TEXT_MESSAGE_FIELD_REMOVAL_BOX QObject::tr("Warning: by removing this field from the template, you will also remove it in the related tags. Do you confirm?")
#define TEXT_TEMPLATEEDITOR_LIST QObject::tr("List")
#define TEXT_TEMPLATEEDITOR_DATE QObject::tr("Date")
#define TEXT_TEMPLATEEDITOR_NUMBER QObject::tr("Number")
#define TEXT_TEMPLATEEDITOR_STRING QObject::tr("String")

//TemplateManagerDialog
#define TEXT_TITLE_DELETE_TEMPLATE_BOX QObject::tr("Delete template(s)?")
#define TEXT_MESSAGE_DELETE_TEMPLATE_BOX QObject::tr("Do you really want to delete this template?\nWarning: you cannot undo this action.")
#define TEXT_DELETE_TEMPLATE_BUTTON QObject::tr("Delete template(s)")
#define TEXT_EXPORT_TEMPLATE_BOX QObject::tr("Export template")
#define TEXT_DUPLICATE_TEMPLATE_BOX QObject::tr("Duplicate template")
#define TEXT_EDIT_TEMPLATE_BOX QObject::tr("Edit template")
#define TEXT_TEMPLATE_WARNING_SELECT_ONE QObject::tr("You must select a template.")
#define TEXT_TEMPLATE_WARNING_SELECT_MULTIPLE QObject::tr("You must select at least one template.")

//ToolBarShowHideGroup - A REVOIR - pas nécessaire d'avoir 2 textes - le changement d'icône d'oeil suffit (oeil ouvert ou barré). Donc oeil + scan marker suffit
#define TEXT_HIDE_MODEL_SCAN_MARKER QObject::tr("Hide scan markers")
#define TEXT_SHOW_MODEL_SCAN_MARKER QObject::tr("Show scan markers")
#define TEXT_HIDE_TAG_MARKER QObject::tr("Hide tags")
#define TEXT_SHOW_TAG_MARKER QObject::tr("Show tags")
#define TEXT_HIDE_TEXTS_MARKER QObject::tr("Hide texts")
#define TEXT_SHOW_TEXTS_MARKER QObject::tr("Show texts")
#define TEXT_HIDE_MEASURES_MARKER QObject::tr("Hide measures")
#define TEXT_SHOW_MEASURES_MARKER QObject::tr("Show measures")
#define TEXT_HIDE_SELECTED_OBJECT QObject::tr("Hide selected object")
#define TEXT_SHOW_SELECTED_OBJECT QObject::tr("Show selected object")
#define TEXT_SHOWHIDE_TOOLBAR_NAME QObject::tr("Show hide toolbar")

// Custom widget
#define TEXT_ENTER_VALUE_PLACEHOLDER QObject::tr("Enter value")

//ControlUserList
#define TEXT_USERLIST_NAME_EXIST QObject::tr("Error : List name already used")
#define TEXT_USERLIST_NAME_EXIST_RENAME QObject::tr("Error : You tried to rename an item with a name that is already used")
#define TEXT_USERLIST_ITEM_EXIST QObject::tr("Error : you tried to add an item that already exists in this list")
#define TEXT_USERLIST_ITEM_EXIST_RENAME QObject::tr("Error : You tried to rename an item with a name that is already used")

//ControlUserList & ControlPipesStandards
#define TEXT_LIST_IMPORT_FAILED QObject::tr("Error : Unexpected error, try to import your file once again")
#define TEXT_LIST_ALREADY_EXIST QObject::tr("Error : List with the same name already exist.")
#define TEXT_LIST_OPEN_FILE_FAILED QObject::tr("Error : The file cannot be opened.")
#define TEXT_LIST_EXPORT_FAILED QObject::tr("Error : export failed.")
#define TEXT_LIST_EXPORT_SUCCESS QObject::tr("Success : File exported at %1")
#define TEXT_LIST_EXPORT_ERROR QObject::tr("Error : export failed due to an internal error. Try to relaunch OpenScanTools.")

//ControlStandards
#define TEXT_STANDARDS_NAME_EXIST QObject::tr("Error : Standard name already used.")
#define TEXT_STANDARDS_NAME_EXIST_RENAME QObject::tr("Error : you tried to rename an item with a name that is already used.")
#define TEXT_STANDARDS_ITEM_EXIST QObject::tr("Error : you tried to add a value that already exists in this list.")
#define TEXT_STANDARDS_ITEM_EXIST_RENAME QObject::tr("Error : you tried to change to a value that already exists in this list.")
#define TEXT_STANDARDS_REDO_NOT_VALID QObject::tr("Error : you tried to redo a delete of a list that already exists or that is not valid")

//DialogDeleteScan
#define TEXT_IMPORTANT_DATA_REMOVAL_TITLE QObject::tr("Important data removal")
#define TEXT_IMPORTANT_SCAN_REMOVAL_MESSAGE QObject::tr("Are you sure you want to delete the file(s) from the hard drive?\nWARNING: this operation is irreversible!")

//ControlIO
#define TEXT_EXPORT_TAGS QObject::tr("Tags successfully exported at %1")

//BeamBending & ColumnTilt
#define TEXT_NA QObject::tr("NA")
#define TEXT_RATIOSUP_YES QObject::tr("Yes")
#define TEXT_RATIOSUP_NO QObject::tr("No")
#define TEXT_RELIABILITY_RELIABLE QObject::tr("Reliable")
#define TEXT_RELIABILITY_UNRELIABLE QObject::tr("Unreliable")

//Align View
#define TEXT_VIEW_TOP QObject::tr("Top view")
#define TEXT_VIEW_BOTTOM QObject::tr("Bottom view")
#define TEXT_VIEW_LEFT QObject::tr("Left view")
#define TEXT_VIEW_RIGHT QObject::tr("Right view")
#define TEXT_VIEW_FRONT QObject::tr("Front view")
#define TEXT_VIEW_BACK QObject::tr("Back view")
#define TEXT_VIEW_ISO QObject::tr("Iso view")

// DialogImportMeshObject
#define TEXT_DIALOG_TITLE_IMPORT_WAVEFRONT QObject::tr("Import settings")
#define TEXT_DIALOG_ERROR_SCALING QObject::tr("Scaling is incorrect")
#define TEXT_DIALOG_ERROR_DIRECTION QObject::tr("Directions are incorrect")

//GeneralDialog
#define TEXT_DIALOG_YES QObject::tr("Yes")
#define TEXT_DIALOG_NO QObject::tr("No")
#define TEXT_DIALOG_CANCEL QObject::tr("Cancel")

// DialogExportVideo
#define TEXT_EXPORT_VIDEO_MISSING_VIEWPOINTS QObject::tr("Error : Viewpoints are missing.")
#define TEXT_EXPORT_VIDEO_SAME_VIEWPOINTS QObject::tr("Error : Same selected viewpoints.")
#define TEXT_EXPORT_VIDEO_ORTHO_VIEWPOINT QObject::tr("Error : Orthographic viewpoint not allowed.")
#define TEXT_EXPORT_VIDEO_MP4_RESOLUTION_TOO_HIGH QObject::tr("The image resolution is too high. It cannot exceed 8294400 pixels (4K). Please reduce it.")
#define TEXT_EXPORT_VIDEO_MP4_X265_MISSING QObject::tr("x265 encoder not found. Please install ffmpeg first, then add the folder that contains ffmpeg.exe in the general settings of OpenScanTools.")

#endif
