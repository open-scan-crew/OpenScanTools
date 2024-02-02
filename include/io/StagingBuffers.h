#ifndef STAGING_BUFFERS_H
#define STAGING_BUFFERS_H

#include <cstdint>
#include "io/E57AdditionalTypes.h"

struct StagingBuffers
{
    uint64_t size = 0;
    float* cartesianX = nullptr;
    float* cartesianY = nullptr;
    float* cartesianZ = nullptr;
    float* sphericalRange = nullptr;
    float* sphericalAzimuth = nullptr;
    float* sphericalElevation = nullptr;
    uint8_t* uIntensity = nullptr;
    float* fIntensity = nullptr;
    uint8_t* colorRed = nullptr;
    uint8_t* colorGreen = nullptr;
    uint8_t* colorBlue = nullptr;
    uint8_t* state = nullptr;

    ~StagingBuffers()
    {
        clear();
    }

    void clear()
    {
        delete[] cartesianX;
        cartesianX = nullptr;
        delete[] cartesianY;
        cartesianY = nullptr;
        delete[] cartesianZ;
        cartesianZ = nullptr;
        delete[] sphericalRange;
        sphericalRange = nullptr;
        delete[] sphericalAzimuth;
        sphericalAzimuth = nullptr;
        delete[] sphericalElevation;
        sphericalElevation = nullptr;
        delete[] uIntensity;
        uIntensity = nullptr;
        delete[] fIntensity;
        fIntensity = nullptr;
        delete[] colorRed;
        colorRed = nullptr;
        delete[] colorGreen;
        colorGreen = nullptr;
        delete[] colorBlue;
        colorBlue = nullptr;
        delete[] state;
        state = nullptr;
    }

    void resize(uint64_t newSize, const E57AttribFormat& format)
    {
        clear();

        size = newSize;

        if (format.coordinates == TL_COORD_SPHERICAL_FLOAT)
        {
            sphericalRange = new float[newSize];
            sphericalAzimuth = new float[newSize];
            sphericalElevation = new float[newSize];
        }
        else
        {
            cartesianX = new float[newSize];
            cartesianY = new float[newSize];
            cartesianZ = new float[newSize];
        }
        uIntensity = new uint8_t[newSize];
        fIntensity = new float[newSize];
        colorRed = new uint8_t[newSize];
        colorGreen = new uint8_t[newSize];
        colorBlue = new uint8_t[newSize];
        state = new uint8_t[newSize];
    }
};

#endif