#ifndef AIRPORT_H
#define AIRPORT_H

#include "CRect.h"
#include "CVector.h"

struct Airport {
    CRect Area;
    float Height;

    // Constructor
    Airport(CRect area, float height);

    // Public method
    bool IsWithinAirport(CVector position);

    // Static methods
    static const Airport LS();
    static const Airport SF();
    static const Airport LV();
    static const Airport LV2();
};

#endif // AIRPORT_H
