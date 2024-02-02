#ifndef FILE_TYPE_TEXTS_HPP
#define FILE_TYPE_TEXTS_HPP

#include <QObject>
#include "io/FileUtils.h"

#include <unordered_map>

//File Extensions
#define TEXT_FILE_TYPE_TLP QObject::tr("Project File (*.tlp)")
#define TEXT_FILE_TYPE_TLT QObject::tr("templates (*.tlt)")

#define TEXT_FILE_TYPE_ASCII QObject::tr("ASCII scans (*.pts *.xyz)")
#define TEXT_FILE_TYPE_TLS QObject::tr("OpenScanTools Scans (*.tls)")
#define TEXT_FILE_TYPE_E57 QObject::tr("E57 Scans (*.e57)")
#define TEXT_FILE_TYPE_FLS QObject::tr("Faro Laser Scans (*.fls)")
#define TEXT_FILE_TYPE_LSPROJ QObject::tr("Faro Project (*.lsproj)")
#define TEXT_FILE_TYPE_RCS QObject::tr("Autodesk ReCap Scans (*.rcs)")
#define TEXT_FILE_TYPE_RCP QObject::tr("Autodesk ReCap Project (*.rcp)")
#define TEXT_FILE_TYPE_PTS QObject::tr("ASCII scans (*.pts)")

#define TEXT_FILE_TYPE_ALL_SCANS_OPEN QObject::tr("All Scans %1").arg("(*.tls *.e57 *.fls *.lsproj *.rcs *.rcp *.pts *.xyz)") + ";;" \
    + TEXT_FILE_TYPE_TLS  + ";;" \
    + TEXT_FILE_TYPE_E57 + ";;" \
    + TEXT_FILE_TYPE_FLS + ";;" \
    + TEXT_FILE_TYPE_LSPROJ + ";;" \
    + TEXT_FILE_TYPE_RCS + ";;" \
    + TEXT_FILE_TYPE_RCP + ";;" \
    + TEXT_FILE_TYPE_ASCII

#define TEXT_FILE_TYPE_OBJ QObject::tr("Wavefront (*.obj)")
#define TEXT_FILE_TYPE_STEP QObject::tr("STEP File  (*.step *.stp)")
#define TEXT_FILE_TYPE_IFC QObject::tr("IFC File  (*.ifc)")
#define TEXT_FILE_TYPE_FBX QObject::tr("Filmbox File (*.fbx)")
#define TEXT_FILE_TYPE_DXF QObject::tr("DXF File  (*.dxf)")
#define TEXT_FILE_TYPE_ALL_OBJECTS QObject::tr("All objects %1").arg("(*.obj *.step *.stp *.ifc *.fbx *.dxf)") + ";;" \
    + TEXT_FILE_TYPE_OBJ + ";;" \
    + TEXT_FILE_TYPE_STEP + ";;" \
    + TEXT_FILE_TYPE_IFC + ";;" \
    + TEXT_FILE_TYPE_FBX + ";;" \
    + TEXT_FILE_TYPE_DXF

#define TEXT_FILE_TYPE_LITE_OBJECTS QObject::tr("All objects %1").arg("(*.obj *.ifc *.fbx *.dxf)") + ";;" \
    + TEXT_FILE_TYPE_OBJ + ";;" \
    + TEXT_FILE_TYPE_IFC + ";;" \
    + TEXT_FILE_TYPE_FBX + ";;" \
    + TEXT_FILE_TYPE_DXF


#define TEXT_FILE_TYPE_IMAGE QObject::tr("Image File  (*.jpeg *.jpg *.png)")

const static std::unordered_map<FileType, QString> s_OutputFormatTexts = {
    { FileType::E57, TEXT_FILE_TYPE_E57 },
    { FileType::TLS, TEXT_FILE_TYPE_TLS },
    { FileType::FARO_LS, TEXT_FILE_TYPE_FLS },
    { FileType::FARO_PROJ, TEXT_FILE_TYPE_LSPROJ },
    //    { FileType::RCS, TEXT_FILE_TYPE_RCS },
    { FileType::RCP, TEXT_FILE_TYPE_RCP },
    { FileType::PTS, TEXT_FILE_TYPE_PTS}
};

const static std::map<QString, FileType> s_InputFormat = {
    { TEXT_FILE_TYPE_E57, FileType::E57 },
    { TEXT_FILE_TYPE_TLS, FileType::TLS },
    { TEXT_FILE_TYPE_FLS, FileType::FARO_LS },
    { TEXT_FILE_TYPE_RCS, FileType::RCS },
    { TEXT_FILE_TYPE_RCP, FileType::RCP }
};

const static std::map<QString, FileType> s_OutputFormat = {
    { TEXT_FILE_TYPE_E57, FileType::E57 },
    { TEXT_FILE_TYPE_TLS, FileType::TLS }
};

#endif