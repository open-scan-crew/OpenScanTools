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

#include "managedAPI/Wrap_RCVector.h"
#include "globals.h"

namespace Autodesk { namespace RealityComputing { namespace Managed {

    public enum class TransformType
    {
        Rigid = (int)NS_RCFoundation::TransformType::Rigid, // Rotation + translation only (scale must be 1.0)
        RigidWithUniformScale = (int)NS_RCFoundation::TransformType::RigidWithUniformScale // Rotation + uniform scale + translation only
    };

    public value class RCTransform
    {
    public:

        // from ReCap
        RCTransform(const NS_RCFoundation::RCTransform% recapTransform)
        {
            translation = RCVector3d(recapTransform.getTranslation());
            scale = RCVector3d(recapTransform.getRotation().getScale());
            rotation = RCVector3d(recapTransform.getRotation().toEulerAngle());
        }

        // from AcGe library
        RCTransform(const AcGeMatrix3d% acGeMatrix)
        {
            auto recapTransform = NS_RCFoundation::RCTransform::fromAcGeMatrix3d(acGeMatrix);
            translation = RCVector3d(recapTransform.getTranslation());
            scale = RCVector3d(recapTransform.getRotation().getScale());
            rotation = RCVector3d(recapTransform.getRotation().toEulerAngle());
        }

        // from Revit
        RCTransform(NS_RevitDB::Transform^ revitTransform)
        {
            auto basisX = revitTransform->BasisX;
            auto basisY = revitTransform->BasisY;
            auto basisZ = revitTransform->BasisZ;
            auto translate = revitTransform->Origin;

            double rawMatrix[16] =
            {
                basisX->X, basisX->Y, basisX->Z, 0,
                basisY->X, basisY->Y, basisY->Z, 0,
                basisZ->X, basisZ->Y, basisZ->Z, 0,
                translate->X, translate->Y, translate->Z, 1
            };

            auto recapTransform = NS_RCFoundation::RCTransform();
            recapTransform.fromColumnMajor(rawMatrix);

            translation = RCVector3d(recapTransform.getTranslation());
            scale = RCVector3d(recapTransform.getRotation().getScale());
            rotation = RCVector3d(recapTransform.getRotation().toEulerAngle());
        }

        void LookAt(RCVector3d eye, RCVector3d center, RCVector3d up)
        {
            auto recapTransform = NS_RCFoundation::lookAt(eye.ToReCapObject(), center.ToReCapObject(), up.ToReCapObject());
            translation = RCVector3d(recapTransform.getTranslation());
            scale = RCVector3d(recapTransform.getRotation().getScale());
            rotation = RCVector3d(recapTransform.getRotation().toEulerAngle());
        }

        // to ReCap
        NS_RCFoundation::RCTransform ToReCapTransform()
        {
            auto rotationMatrix = NS_RCFoundation::RCRotationMatrix(this->rotation.ToReCapObject(), this->scale.ToReCapObject());
            auto transformMatrix = NS_RCFoundation::RCTransform(rotationMatrix, this->translation.ToReCapObject());
            return transformMatrix;
        }

        // to AcGe library
        AcGeMatrix3d ToAcGeMatrix3d()
        {
            return ToReCapTransform().toAcGeMatrix3d();
        }

        RCTransform MultiplyBy(RCTransform other)
        {
            return RCTransform(ToReCapTransform() * other.ToReCapTransform());
        }

        RCVector3d MultiplyBy(RCVector3d pt)
        {
            return RCVector3d(ToReCapTransform() * pt.ToReCapObject());
        }

        RCTransform GetInverse(TransformType type)
        {
            return RCTransform(ToReCapTransform().getInverse((NS_RCFoundation::TransformType)type));
        }

        // to Revit
        NS_RevitDB::Transform^ ToRevitTransform()
        {
            if (scale.x != scale.y || scale.y != scale.z)
            {
                throw "Revit Tranform allows only uniform scaling. You can't use ToRevitTransform on this RCTransform object.";
            }
            auto recapTransform = ToReCapTransform();
            double rawMatrix[16];
            recapTransform.toColumnMajor(rawMatrix);

            auto trf = NS_RevitDB::Transform::Identity;
            trf->BasisX = gcnew NS_RevitDB::XYZ(rawMatrix[0], rawMatrix[1], rawMatrix[2]); // rawMatrix[3] = 0
            trf->BasisY = gcnew NS_RevitDB::XYZ(rawMatrix[4], rawMatrix[5], rawMatrix[6]); // rawMatrix[7] = 0
            trf->BasisZ = gcnew NS_RevitDB::XYZ(rawMatrix[8], rawMatrix[9], rawMatrix[10]); // rawMatrix[11] = 0
            trf->Origin = gcnew NS_RevitDB::XYZ(rawMatrix[12], rawMatrix[13], rawMatrix[14]); // rawMatrix[15] = 1

            return trf;
        }

        virtual System::String^ ToString() override
        {
            System::String^ result = gcnew System::String("\n");
            result += "Translation:            " + translation.ToString() + "\n";
            result += "Scale:                  " + scale.ToString() + "\n";
            result += "Rotation(Euler angles): " + rotation.ToString() + "\n";
            return result;
        }

        property RCVector3d translation;
        property RCVector3d scale;
        property RCVector3d rotation;
    };

    public ref class RCProjection
    {
    public:
        RCProjection()
        {
            mProjection = new NS_RCFoundation::RCProjection();
        }

        RCProjection(RCProjection% other)
        {
            mProjection = new NS_RCFoundation::RCProjection(*other.ToReCapObject());
        }

        // from ReCap
        RCProjection(const NS_RCFoundation::RCProjection% recapProjection)
        {
            mProjection = new NS_RCFoundation::RCProjection(recapProjection);
        }

        // from Revit
        RCProjection(NS_RevitDB::DirectContext3D::Camera ^ revitCamera)
        {
            if (revitCamera == nullptr || !revitCamera->IsValidObject)
            {
                throw gcnew NullReferenceException("Revit CameraInfo is null or invalid.");
            }
            mProjection = new NS_RCFoundation::RCProjection();

            if (revitCamera->ProjectionMethod == NS_RevitDB::DirectContext3D::ProjectionMethod::Perspective)
            {
                double fovY = atan(revitCamera->VerticalExtent / (2 * revitCamera->TargetDistance)) * 180 / M_PI;
                mProjection->setPerspective
                (
                    fovY, // angle
                    revitCamera->HorizontalExtent / revitCamera->VerticalExtent, // ratio width / height
                    revitCamera->NearDistance, // near
                    revitCamera->FarDistance // far 
                );
            }
            else
            {
                double halfWidth = revitCamera->HorizontalExtent * 0.5;
                double halfHeight = revitCamera->VerticalExtent * 0.5;
                // TODO: check signs of left-right and bottom-top
                mProjection->setOrthographic(
                    -halfWidth,	// left
                    halfWidth, // right
                    -halfHeight, // bottom
                    halfHeight,	// top
                    revitCamera->NearDistance, // near
                    revitCamera->FarDistance // far
                );
            }
        }

        void SetOrthographic(double left, double right, double bottom, double top, double nearval, double farval)
        {
            mProjection->setOrthographic(left, right, bottom, top, nearval, farval);
        }

        void SetPerspective(double angle, double ratio, double fNear, double fFar)
        {
            mProjection->setPerspective(angle, ratio, fNear, fFar);
        }

        RCVector3d TransformPoint(RCVector3d pt)
        {
            return RCVector3d(mProjection->transformPoint(pt.ToReCapObject()));
        }

        RCProjection^ GetInverse()
        {
            return gcnew RCProjection(mProjection->getInverse());
        }

        // to ReCap
        NS_RCFoundation::RCSharedPtr<NS_RCFoundation::RCProjection> ToReCapObject()
        {
            return mProjection.Get();
        }

    private:
        RCScopedPointer<NS_RCFoundation::RCProjection> mProjection;
    };
}}}    // namespace Autodesk::RealityComputing::Managed
