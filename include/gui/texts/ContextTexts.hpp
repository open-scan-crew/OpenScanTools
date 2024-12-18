#ifndef CONTEXT_TEXTS_HPP
#define CONTEXT_TEXTS_HPP

#include <qobject.h>

//ContextPointMeasure
#define TEXT_POINT_MEASURE_START QObject::tr("Ready for first point.")
#define TEXT_POINT_MEASURE_REJECTED QObject::tr("Point rejected.")
#define TEXT_POINT_MEASURE_NEXT_POINT QObject::tr("Point ok, ready for the next point.")
#define TEXT_POINT_MEASURE_CREATED QObject::tr("Measure done, ready for the next point.")

//ContextSaveCloseCreateProject
#define TEXT_SAVELOADCLOSE_SAVE_QUESTION QObject::tr("The project has been modified.\nDo you want to save it before closing?")
#define TEXT_SAVELOADCLOSE_SAVE_FREEVERSION_QUESTION QObject::tr("Do you want to close the project?\nModifications will not be saved in the freeviewer version")
#define TEXT_SAVELOADCLOSE_QUIT_QUESTION QObject::tr("Do you want to quit OpenScanTools?")
#define TEXT_SAVELOADCLOSE_QUIT_FREEVERSION_QUESTION QObject::tr("Do you want to quit OpenScanTools?\nModifications will not be saved in the freeviewer version.")
#define TEXT_BLANKPROJECT_NOLANGUAGE QObject::tr("No blank project has been found for your language. The English one will be opened by default.")
#define TEXT_CREATE_ALREADY_EXISTS QObject::tr("This folder already contains a project with the same name. Please use another name.")
#define TEXT_SAVELOADCLOSE_RESTORE_QUESTION QObject::tr("The project have backup files:\n%1\nDo you want to restore and load them?")
#define TEXT_SAVE_COMPLETE QObject::tr("Project saved!")

//ContextRobustCylinder
#define TEXT_LUCAS_SEARCH_ONGOING QObject::tr("Searching...")
#define TEXT_LUCAS_SEARCH_DONE QObject::tr("Done!")
#define TEXT_CYLINDER_FOUND QObject::tr("Pipe found, see properties.") //si c'est le texte actuellement mis en description, il doit être supprimé (les mesures sont dans les champs)
#define TEXT_CYLINDER_NOT_FOUND QObject::tr("No pipe found.")
#define TEXT_CYLINDER_INSULATED_THICKNESS_EXCEEDS QObject::tr("Insulated thickness exceeds detected pipe radius")

//ContextSlabDetect
#define TEXT_SLAB_NOT_FOUND QObject::tr("No box found.")

//ContextBigCylinderFit
#define TEXT_BIGCYLINDERFIT_START QObject::tr("Select the first point on the pipe perimeter.")
#define TEXT_BIGCYLINDERFIT_SECOND QObject::tr("Select the second point on the pipe perimeter, far enough away from the first one")
#define TEXT_BIGCYLINDERFIT_THIRD QObject::tr("Select the first point on the pipe length.")
#define TEXT_BIGCYLINDERFIT_FOURTH QObject::tr("Select the second point on the pipe length.")
#define TEXT_BIGCYLINDERFIT_FAILED QObject::tr("No pipe found. Select the first point on the pipe perimeter.")

//ContextPipeDetectionConnexion
#define TEXT_PIPEDETECTIONCONNEXION_MANUALEXTEND_FAIL QObject::tr("No pipe found. Select the first point on the pipe length.")
#define TEXT_PIPEDETECTIONCONNEXION_FAIL QObject::tr("No pipe found. Select a point on the pipe.")

//ContextBeamBending
#define TEXT_BEAMBENDING_START  QObject::tr("Select the first point on the lower beam face.")
#define TEXT_BEAMBENDING_NEXT  QObject::tr("Select the second point on the lower beam face.")
#define TEXT_BEAMBENDING_RESULT  QObject::tr("Max bending found, ready for the next one.")

//ContextPointToPlaneMeasure
#define TEXT_POINTTOPLANE_START QObject::tr("Select a point.")
#define TEXT_POINTTOPLANE_FAILED QObject::tr("No plane detected.")
#define TEXT_POINTTOPLANE_SELECT_PLANE QObject::tr("Select a point on the plane.")

//ContextPointToCylinderMeasure
#define TEXT_POINTTOCYLINDER_START QObject::tr("Select a point on the pipe.")

//ContextCylinderToPlaneMeasure
#define TEXT_CYLINDERTOPLANE_START QObject::tr("Select a pipe.")
#define TEXT_CYLINDERTOPLANE_FAILED QObject::tr("No pipe found. Select a pipe.")

//ContextPlane3
#define TEXT_PLANE3_FIRST QObject::tr("Select the first point on the plane.")
#define TEXT_PLANE3_SECOND QObject::tr("Select the second point on the plane.")
#define TEXT_PLANE3_THIRD QObject::tr("Select the third point on the plane.")

//ContextSphere
#define TEXT_SPHERE_FAILED QObject::tr("No sphere found.")

//ContextCylinderToCylinderMeasure
#define TEXT_CYLINDERTOCYLINDER_START QObject::tr("Select the first pipe.")
#define TEXT_CYLINDERTOCYLINDER_NEXT QObject::tr("Select the second pipe.")
#define TEXT_CYLINDERTOCYLINDER_FAILED_SECOND QObject::tr("No pipe found. Select the second pipe.")

//ContextColumnTilt
#define TEXT_COLUMNTILIT_FIRST_POINT QObject::tr("Select the first point of an axis that is perpendicular to the tilt search direction")
#define TEXT_COLUMNTILIT_SECOND_POINT QObject::tr("Select the second point of an axis that is perpendicular to the tilt search direction")
#define TEXT_COLUMNTILIT_THIRD_POINT QObject::tr("Select the first point on the column face (top or bottom)")
#define TEXT_COLUMNTILIT_FOURTH_POINT QObject::tr("Select the second point on the column face (bottom or top)")

//ContextColumnTilt
#define TEXT_COLUMNTILT_DONE QObject::tr("Tilt found, ready for the next one (using the same direction).")

//ContextState
#define TEXT_TEMPLATE_NOT_SELECTED QObject::tr("Error : no template selected.")

// ContextDuplicateTag
#define TEXT_INFO_DUPLICATE_TAG_PICK QObject::tr("Select a existing tag or pick a new position.")
#define TEXT_INFO_DUPLICATE_TAG_WRONG_ID QObject::tr("You must select tags that use the same template.")
#define TEXT_INFO_DUPLICATE_TAG_WRONG_AUTHOR QObject::tr("You must select tags of which you are the author.")

// ContextImportOSTObjects
#define TEXT_IMPORT_NO_MISSING_FILES QObject::tr("No missing files found")

//ContextClippingBox Creation & duplication
#define TEXT_CLIPPINGBOX_START QObject::tr("Ready to create a box. Click on the desired area in the point cloud.")
#define TEXT_CLIPPINGBOX_CREATION_DONE QObject::tr("Box created. Ready for the next one.")
#define TEXT_CLIPPINGBOX_DUPLICATION_DONE QObject::tr("Box duplicated. Ready for the next one.")

//ContextDeletePoints
#define TEXT_DELETE_POINTS_QUESTION QObject::tr("Warning: points will be permanently deleted. This action cannot be undone.\nDo you confirm?")
#define TEXT_DELETE_SAVE_BEFORE_QUESTION QObject::tr("It is recommended to save the project before deleting points. Do you want to save now?")

//ContextDeleteTags
#define TEXT_DELETE_TAGS_QUESTION QObject::tr("Warning: some tags use this template. Deleting the template will also delete the related tags.\nDo you confirm?")

//ContextTemplateModification
#define TEXT_TEMPLATE_MODIFICATION_QUESTION QObject::tr("Warning: all the tags that use this template will be modified accordingly.\nDo you confirm?")

//ContextMeshObjectCreation
#define TEXT_DIRECTORY_CREATION_FAILED QObject::tr("Failed to create directory: %1")
#define TEXT_FILE_COPY_FAILED QObject::tr("Failed to copy file: %1\nIt will be skipped.")

//ContextConvertionScan
#define TEXT_IMPORTING_SCAN QObject::tr("Importing")
#define TEXT_BIG_COORDINATES_DETECTED QObject::tr("The detected coordinates are too large, which may deteriorate the navigation.\nWe advise you to truncate them. Suggested values:");
#define TEXT_TRANSLATE_COORDINATES QObject::tr("You can either translate the coordinates to work on a local system, or keep them")
#define TEXT_CONVERTION_SCAN_ERR_FORMAT_NOT_SUPPORTED QObject::tr("The format of the scan is not supported.")
#define TEXT_CONVERTION_SCAN_FAILED QObject::tr("%1 could not be converted.")
#define TEXT_CONVERTION_CONVERTING QObject::tr("Conversion in progress")
#define TEXT_CONVERTION_FILE_ALREADY_EXIST QObject::tr("Some files already exist in the project folder:\n %1 \nYou can overwrite them by setting the overwrite option.")
#define TEXT_CONVERTION_WARNING QObject::tr("\n Would you like to continue the conversion?")
#define TEXT_CONVERTION_ERROR QObject::tr("\n Would you like to force the conversion?")
#define TEXT_CONVERTION_DONE QObject::tr("Conversion done!")
#define TEXT_CONVERTION_ERROR_FILE_NOT_VALID QObject::tr("File: %1 is not a valid point cloud file.\nConversion stopped.\n")
#define TEXT_CONVERTION_ERROR_FILE_FAILED_TO_WRITE QObject::tr("File: %1 failed to write.\nConversion stopped.\n")
#define TEXT_CONVERTION_FILE_SKIPPING QObject::tr("File: %1 already exists.\nSkipping conversion.\n")
#define TEXT_CONVERTION_DONE_TEXT QObject::tr("Conversion done in: %1 seconds.")
#define TEXT_SCAN_IMPORT_DONE_TEXT QObject::tr("%1 imported")
#define TEXT_SCAN_IMPORT_DONE_TIMING_TEXT QObject::tr("%1 imported in %2 seconds.")
#define TEXT_SCAN_IMPORT_FAILED QObject::tr("Scan import %1 failed.")


//ContextFindScan
#define TEXT_CONTEXT_FIND_SCAN_MODAL QObject::tr("Point is from scan : %1")
#define TEXT_CONTEXT_NO_SCAN_FOUND QObject::tr("No scan found")

//ContextExportVideoHD
#define TEXT_CONTEXT_EXPORT_VIDEO QObject::tr("Generate video.")
#define TEXT_CONTEXT_EXPORT_VIDEO_STEPS QObject::tr("Frames : %1 / %2")
#define TEXT_CONTEXT_EXPORT_VIDEO_DONE QObject::tr("Done")
#define TEXT_CONTEXT_EXPORT_VIDEO_FAIL QObject::tr("Fail")
#define TEXT_CONTEXT_EXPORT_VIDEO_TIME QObject::tr("Sequence generated in %1 seconds")

//ContextMoveManip
#define TEXT_MOVE_MANIP_START QObject::tr("Select a temporary position for the manipulator.")

//ContextManipulateObjects
#define TEXT_MANIP_OBJ_BASE_FIRST QObject::tr("Select the first base position.")
#define TEXT_MANIP_OBJ_BASE_SECOND QObject::tr("Select the second base position.")
#define TEXT_MANIP_OBJ_BASE_THIRD QObject::tr("Select the third base position.")
#define TEXT_MANIP_OBJ_TARGET_FIRST QObject::tr("Select the first target position.")
#define TEXT_MANIP_OBJ_TARGET_SECOND QObject::tr("Select the second target position.")
#define TEXT_MANIP_OBJ_TARGET_THIRD QObject::tr("Select the third target position.")

#define TEXT_NO_OBJECT_SELECTED QObject::tr("Please select objects")


#endif