//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2020 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//////////////////////////////////////////////////////////////////////////////
//
// This API extends AcGi to support stream draw functionality
//

#pragma once

#pragma pack(push, 8)

class AcGiDrawStreamImp;

typedef bool (*GraphicsUpdateProc)(const AcArray<AcGiDrawable*>& drawableArray);

///////////////////////////////////////////////////////////////////////////////
// class AcGiDrawStream
//
class  AcGiDrawStream : public AcGiDrawable
{
    friend class AcGiDrawStreamImp;

public:
    ACRX_DECLARE_MEMBERS_EXPIMP(AcGiDrawStream, ACDBCORE2D_PORT);

    ACDBCORE2D_PORT static bool build(const AcArray<AcGiDrawStream*>& streamArray, GraphicsUpdateProc lpFunc);

    ACDBCORE2D_PORT AcGiDrawStream();
    ACDBCORE2D_PORT AcGiDrawStream(const AcGiDrawable* pOwner);
   ACDBCORE2D_PORT ~AcGiDrawStream();

    ACDBCORE2D_PORT AcGiDrawable* getOwner() const;
    ACDBCORE2D_PORT void setOwner(const AcGiDrawable* pOwner);
    ACDBCORE2D_PORT bool isValid() const;

    ACDBCORE2D_PORT bool serializeOut(IAcWriteStream* pOutput) const;
    ACDBCORE2D_PORT bool serializeIn(IAcReadStream* pInput, AcDbDatabase* pDb = NULL);

    // Overridden methods from AcGiDrawable
    ACDBCORE2D_PORT Adesk::Boolean isPersistent() const override;
    ACDBCORE2D_PORT AcDbObjectId id() const override;
    ACDBCORE2D_PORT AcGiDrawable::DrawableType drawableType() const override;
    ACDBCORE2D_PORT bool bounds(AcDbExtents& bounds) const override;
    ACDBCORE2D_PORT void setDrawStream(AcGiDrawStream* pStream) override;
    ACDBCORE2D_PORT AcGiDrawStream* drawStream() const override;

protected:
    // Overridden methods from AcGiDrawable
    ACDBCORE2D_PORT Adesk::UInt32 subSetAttributes(AcGiDrawableTraits* pTraits) override;
    ACDBCORE2D_PORT Adesk::Boolean subWorldDraw(AcGiWorldDraw* pWd) override;
    ACDBCORE2D_PORT void subViewportDraw(AcGiViewportDraw* pVd) override;
    ACDBCORE2D_PORT Adesk::UInt32 subViewportDrawLogicalFlags(AcGiViewportDraw* pVd) override;

protected:
    AcGiDrawStreamImp* m_pImp;
};

#pragma pack(pop)
