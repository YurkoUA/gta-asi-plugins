#include "Airport.h"

// Constructor definition
Airport::Airport(CRect area, float height)
    : Area(area), Height(height) {}

// Method to check if a position is within the airport
bool Airport::IsWithinAirport(CVector position) {
    CVector2D position2d = CVector2D(position.x, position.y);

    return Area.IsPointInside(position2d)
        && position.z >= Height - 1;
}

// Static methods to return predefined airports
const Airport Airport::LS() {
    return Airport(CRect(2141, -2468, 1396, -2629), 13);
}

const Airport Airport::SF() {
    return Airport(CRect(-1714, -191, -1002, 452), 14);
}

const Airport Airport::LV() {
    return Airport(CRect(1536, 1153, 1341, 1854), 10);
}

const Airport Airport::LV2() {
    return Airport(CRect(449, 2559, -77, 2466), 16);
}
