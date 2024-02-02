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
// DESCRIPTION:
//
// This file contains the class AcGeInterval - a representation
// for an interval on the real line.  The following kinds of intervals
// are supported.
// . Open intervals where the bounds are not finite.
// . Closed intervals with finite bounds.
// . Half intervals such that one end is open and not finite,
//   and the other is closed and finite.
//
// Where applicable, all evaluations are performed within the
// tolerance stored within this class.

#ifndef AC_GEINTRVL_H
#define AC_GEINTRVL_H

#include "gegbl.h"
#pragma pack (push, 8)

class

AcGeInterval
{
public:
    GE_DLLEXPIMPORT AcGeInterval(double tol = 1.e-12);
    GE_DLLEXPIMPORT AcGeInterval(const AcGeInterval& src);
    GE_DLLEXPIMPORT AcGeInterval(double lower, double upper, double tol = 1.e-12);
    GE_DLLEXPIMPORT AcGeInterval(Adesk::Boolean boundedBelow, double bound,
                 double tol = 1.e-12);
    GE_DLLEXPIMPORT ~AcGeInterval();

    // Assignment operator.
    //
    GE_DLLEXPIMPORT AcGeInterval&  operator =       (const AcGeInterval& otherInterval);

    // Get/set methods.
    //
    GE_DLLEXPIMPORT double         lowerBound       () const;
    GE_DLLEXPIMPORT double         upperBound       () const;
    GE_DLLEXPIMPORT double         element          () const;
    GE_DLLEXPIMPORT void           getBounds        (double& lower, double& upper) const;
    GE_DLLEXPIMPORT double         length           () const;
    GE_DLLEXPIMPORT double         tolerance        () const;

    GE_DLLEXPIMPORT AcGeInterval&  set              (double lower, double upper);
    GE_DLLEXPIMPORT AcGeInterval&  set              (Adesk::Boolean boundedBelow, double bound);
    GE_DLLEXPIMPORT AcGeInterval&  set              ();
    GE_DLLEXPIMPORT AcGeInterval&  setUpper         (double upper);
    GE_DLLEXPIMPORT AcGeInterval&  setLower         (double lower);
    GE_DLLEXPIMPORT AcGeInterval&  setTolerance     (double tol);

    // Interval editing.
    //
    GE_DLLEXPIMPORT void           getMerge         (const AcGeInterval& otherInterval, AcGeInterval& result) const;
    GE_DLLEXPIMPORT int            subtract         (const AcGeInterval& otherInterval,
                                     AcGeInterval& lInterval,
                                     AcGeInterval& rInterval) const;
    GE_DLLEXPIMPORT Adesk::Boolean intersectWith    (const AcGeInterval& otherInterval, AcGeInterval& result) const;

    // Interval characterization.
    //
    GE_DLLEXPIMPORT Adesk::Boolean isBounded        () const;
    GE_DLLEXPIMPORT Adesk::Boolean isBoundedAbove   () const;
    GE_DLLEXPIMPORT Adesk::Boolean isBoundedBelow   () const;
    GE_DLLEXPIMPORT Adesk::Boolean isUnBounded      () const;
    GE_DLLEXPIMPORT Adesk::Boolean isSingleton      () const;

    // Relation to other intervals.
    //
    GE_DLLEXPIMPORT Adesk::Boolean isDisjoint       (const AcGeInterval& otherInterval) const;
    GE_DLLEXPIMPORT Adesk::Boolean contains         (const AcGeInterval& otherInterval) const;
    GE_DLLEXPIMPORT Adesk::Boolean contains         (double val) const;

    // Continuity
    //
    GE_DLLEXPIMPORT Adesk::Boolean isContinuousAtUpper (const AcGeInterval& otherInterval) const;
    GE_DLLEXPIMPORT Adesk::Boolean isOverlapAtUpper    (const AcGeInterval& otherInterval,
                                        AcGeInterval& overlap) const;
    // Equality
    //
    GE_DLLEXPIMPORT Adesk::Boolean operator ==      (const AcGeInterval& otherInterval) const;
    GE_DLLEXPIMPORT Adesk::Boolean operator !=      (const AcGeInterval& otherInterval) const;
    GE_DLLEXPIMPORT Adesk::Boolean isEqualAtUpper   (const AcGeInterval& otherInterval) const;
    GE_DLLEXPIMPORT Adesk::Boolean isEqualAtUpper   (double value) const;
    GE_DLLEXPIMPORT Adesk::Boolean isEqualAtLower   (const AcGeInterval& otherInterval) const;
    GE_DLLEXPIMPORT Adesk::Boolean isEqualAtLower   (double value) const;

    // To be used with periodic curves
    //
    GE_DLLEXPIMPORT Adesk::Boolean isPeriodicallyOn (double period, double& val);

    // Comparisons.
    //
    friend
    GE_DLLEXPIMPORT
    Adesk::Boolean operator >       (double val, const AcGeInterval& intrvl);
    GE_DLLEXPIMPORT Adesk::Boolean operator >       (double val) const;
    GE_DLLEXPIMPORT Adesk::Boolean operator >       (const AcGeInterval& otherInterval) const;
    friend
    GE_DLLEXPIMPORT
    Adesk::Boolean operator >=      (double val, const AcGeInterval& intrvl);
    GE_DLLEXPIMPORT Adesk::Boolean operator >=      (double val) const;
    GE_DLLEXPIMPORT Adesk::Boolean operator >=      (const AcGeInterval& otherInterval) const;
    friend
    GE_DLLEXPIMPORT
    Adesk::Boolean operator <       (double val, const AcGeInterval& intrvl);
    GE_DLLEXPIMPORT Adesk::Boolean operator <       (double val) const;
    GE_DLLEXPIMPORT Adesk::Boolean operator <       (const AcGeInterval& otherInterval) const;
    friend
    GE_DLLEXPIMPORT
    Adesk::Boolean operator <=      (double val, const AcGeInterval& intrvl);
    GE_DLLEXPIMPORT Adesk::Boolean operator <=      (double val) const;
    GE_DLLEXPIMPORT Adesk::Boolean operator <=      (const AcGeInterval& otherInterval) const;

protected:
    friend class AcGeImpInterval;

    class AcGeImpInterval  *mpImpInt;

    // Construct object from its corresponding implementation object.
    GE_DLLEXPIMPORT AcGeInterval (AcGeImpInterval&, int);

private:
    int              mDelInt;
};

#pragma pack (pop)
#endif
