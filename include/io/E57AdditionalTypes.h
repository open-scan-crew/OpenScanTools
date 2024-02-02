#ifndef E57_ADDITIONAL_TYPES_H
#define E57_ADDITIONAL_TYPES_H

enum TlCoordinatesFormat {
    TL_COORD_CARTESIAN_FLOAT = 0,
    TL_COORD_CARTESIAN_SCALED_INTEGER,
    TL_COORD_SPHERICAL_FLOAT
};

enum TlColorFormat {
    TL_RGB_NONE = -1,
    TL_RGB_UINT8_FORMAT = 0,
    TL_RGB_FLOAT_FORMAT
};

enum TlIntensityFormat {
    TL_I_NONE = -1,
    TL_I_UINT8_FORMAT = 0,
    TL_I_SCALED_INTEGER_FORMAT,
    TL_I_FLOAT_FORMAT
};

enum TlCartesianInvalidStateFormat {
    TL_STATE_NONE = -1,
    TL_STATE_PRESENT
};

enum TlResult {
    TL_RESULT_OK = 0,
    TL_CONVERSION_NO_ROTATION = 1,
    TL_CONVERSION_NO_TRANSLATION = 2,
    TL_CONVERSION_NO_ROTATION_AND_TRANSLATION = 3,
    TL_ERROR = -1,
    TL_ERROR_INVALID_FORMAT = -2,
    TL_ERROR_FORMAT_NOT_SUPPORTED = -3
};

struct E57AttribFormat {
    TlCoordinatesFormat coordinates;
    TlColorFormat color;
    TlIntensityFormat intensity;
    TlCartesianInvalidStateFormat state;
};

struct Limits {
    double iMin;
    double iMax;
};

#endif