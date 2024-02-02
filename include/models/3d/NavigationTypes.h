#ifndef NAVIGATION_TYPES_H
#define NAVIGATION_TYPES_H

enum class NavigationMode
{
    Explore,
    Panoramic,
    Examine,
    Orthographic,
    OrthoExamine
};

struct NavigationParameters
{
    //double 0 to 200 (0:slow, 100:normal(actual speed), 200: fast)
    double cameraTranslationSpeedFactor = 100.;
    double cameraRotationExamineFactor = 100.;

    double examineMinimumRadius = 0.25;

    bool wheelInverted = false;
    bool mouseDragInverted = false;
};

enum class NaviConstraint
{
    LockVertical,
    LockHorizontal,
    LockZValue,
    LockYValue,
    LockXValue
};


#endif
