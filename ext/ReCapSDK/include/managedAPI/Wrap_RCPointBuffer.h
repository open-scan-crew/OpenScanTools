//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Copyright 2020 by Autodesk, Inc.
//
// The information contained herein is confidential, proprietary to Autodesk,
// Inc., and considered a trade secret as defined in section 499C of the
// penal code of the State of California.  Use of this information by anyone
// other than authorized employees of Autodesk, Inc. is granted only under a
// written non-disclosure agreement, expressly prescribing the scope and
// manner of such use.
//
//        AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS.
//        AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
//        MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC.
//        DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
//        UNINTERRUPTED OR ERROR FREE.
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "managedAPI/Wrap_RCPoint.h"
#include "managedAPI/Wrap_RCVector.h"

#include "RCScopedPointer.h"

#include "globals.h"

namespace Autodesk { namespace RealityComputing { namespace Managed {

    public enum class RCAttributeType
    {
        None = (int)NS_RCData::RCAttributeType::None,
        Position = (int)NS_RCData::RCAttributeType::Position,
        Color = (int)NS_RCData::RCAttributeType::Color,
        Intensity = (int)NS_RCData::RCAttributeType::Intensity,
        Classification = (int)NS_RCData::RCAttributeType::Classification,
        Normal = (int)NS_RCData::RCAttributeType::Normal,
        SegmentId = (int)NS_RCData::RCAttributeType::SegmentId,
        TimeStamp = (int)NS_RCData::RCAttributeType::TimeStamp,
        RowIndex = (int)NS_RCData::RCAttributeType::RowIndex,
        ColumnIndex = (int)NS_RCData::RCAttributeType::ColumnIndex,
        End = (int)NS_RCData::RCAttributeType::End,
    };

    public enum class RCCoordinateType
    {
        Cartesian = (int)NS_RCData::RCCoordinateType::Cartesian,
        Spherical = (int)NS_RCData::RCCoordinateType::Spherical,
    };

    public ref class RCPointBuffer
    {
        RCScopedPointer<NS_RCData::RCPointBuffer> mPointBuffer;

    public:

        RCPointBuffer();

        unsigned int GetSize();

        void Resize(UInt32 size);

        void Clear();

        RCCoordinateType GetCoordinateType();

        void SetCoordinateType(RCCoordinateType type);

        //virtual void appendPositions(const RCBuffer<RCVector3d>& points);

        virtual void AppendPosition(RCVector3d point);

        //virtual bool setPositions(const RCBuffer<RCVector3d>& points);

        //virtual bool setPositionsAt(unsigned int index, const RCBuffer<RCVector3d>& points);

        //virtual const RCBuffer<RCVector3d>* const getPositions();

        virtual bool SetPositionAt(UInt32 index, RCVector3d point);

        virtual bool GetPositionAt(UInt32 index, RCVector3d% point);

        bool AddAttribute(RCAttributeType attribute);

        bool RemoveAttribute(RCAttributeType attribute);

        bool HasAttribute(RCAttributeType attribute);

        virtual bool HasNormal();

        //virtual bool setNormals(const RCBuffer<RCVector3d>& normals);

        //virtual bool setNormalsAt(unsigned int index, const RCBuffer<RCVector3d>& normals);

        //virtual const RCBuffer<RCVector3d>* const getNormals();

        virtual bool SetNormalAt(UInt32 index, RCVector3d normal);

        virtual bool GetNormalAt(UInt32 index, RCVector3d% normal);

        virtual bool HasClassification();

        //virtual bool setClassifications(const RCBuffer<unsigned char>& classifications);

        //virtual bool setClassificationsAt(unsigned int index, const RCBuffer<unsigned char>& classifications);

        //virtual const RCBuffer<unsigned char>* const getClassifications();

        virtual bool SetClassificationAt(UInt32 index, Byte classification);

        virtual bool GetClassificationAt(UInt32 index, [Out] Byte% classification);

        virtual bool HasColor();

        //virtual bool setColors(const RCBuffer<RCVector4ub>& colors);

        //virtual bool setColorsAt(unsigned int index, const RCBuffer<RCVector4ub>& colors);

        //virtual const RCBuffer<RCVector4ub>* const getColors();

        virtual bool SetColorAt(UInt32 index, RCColor color);

        virtual bool GetColorAt(UInt32 index, RCColor% color);

        virtual bool HasIntensity();

        //virtual bool setIntensities(const RCBuffer<unsigned char>& intensities);

        //virtual bool setIntensitiesAt(unsigned int index, const RCBuffer<unsigned char>& intensities);

        //virtual const RCBuffer<unsigned char>* const getIntensities();

        virtual bool SetIntensityAt(UInt32 index, Byte intensity);

        virtual bool GetIntensityAt(UInt32 index, [Out] Byte% intensity);

        virtual bool HasSegmentId();

        virtual void SetNumberOfSegments(UInt32 numberOfSegments);

        virtual unsigned short GetNumberOfSegments();

        virtual bool AreSegmentIdsValid();

        //virtual bool setSegmentIds(const RCBuffer<unsigned short>& segmentIds);

        //virtual bool setSegmentIdsAt(unsigned int index, const RCBuffer<unsigned short>& segmentIds);

        //virtual const RCBuffer<unsigned short>* const getSegmentIds();

        virtual bool SetSegmentIdAt(UInt32 index, UInt16 segment);

        virtual bool GetSegmentIdAt(UInt32 index, [Out] UInt16% segment);

        virtual bool HasTimeStamp();

        //virtual bool setTimeStamps(const RCBuffer<double>& timeStamps);

        //virtual bool setTimeStampsAt(unsigned int index, const RCBuffer<double>& timeStamps);

        //virtual const RCBuffer<double>* const getTimeStamps();

        virtual bool SetTimeStampAt(UInt32 index, Double timeStamp);

        virtual bool GetTimeStampAt(UInt32 index, [Out] Double% timeStamp);

        //bool setColumnIndices(const RCBuffer<unsigned int>& columnIndices);

        //bool setColumnIndicesAt(std::uint64_t index, const RCBuffer<unsigned int>& columnIndices);

        //const RCBuffer<unsigned int>* const getColumnIndices();

        bool SetColumnIndexAt(UInt64 index, UInt32 columnIndex);

        bool GetColumnIndexAt(UInt64 index, [Out] UInt32% columnIndex);

        //bool setRowIndices(const RCBuffer<unsigned int>& rowIndices);

        //bool setRowIndicesAt(std::uint64_t index, const RCBuffer<unsigned int>& rowIndices);

        //const RCBuffer<unsigned int>* const getRowIndices();

        bool SetRowIndexAt(UInt64 index, UInt32 rowIndex);

        bool GetRowIndexAt(UInt64 index, [Out] UInt32% rowIndex);

#pragma region set attribute values from raw array

        bool SetPositions(Collections::Generic::List<RCVector3d>^ points);

        bool SetNormals(Collections::Generic::List<RCVector3d>^ normals);

        bool SetClassifications(Collections::Generic::List<Byte>^ classifications);

        bool SetColors(Collections::Generic::List<RCColor>^ colors);

        bool SetIntensities(Collections::Generic::List<Byte>^ intensities);

        bool SetSegmentIds(Collections::Generic::List<UInt16>^ segmentIds);

        bool SetTimeStamps(Collections::Generic::List<Double>^ timeStamps);

        bool SetColumnIndices(Collections::Generic::List<UInt32>^ columnIndices);

        bool SetRowIndices(Collections::Generic::List<UInt32>^ rowIndices);
#pragma endregion

    };
}}}    // namespace Autodesk::RealityComputing::Managed