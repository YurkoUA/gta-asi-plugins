#ifndef TRACKABLEVEHICLE_H
#define TRACKABLEVEHICLE_H

#include <vector>
#include "CVehicle.h"
#include "CVector.h"
#include "CRGBA.h"
#include "Airport.h"

class TrackableVehicle {
public:
    CVehicle* vehicle;
    unsigned int handle;
    std::vector<CVector> path;
    CVector* lastKnownPosition;
    int randomNumber;
    bool isCurrentlyDrivenByPlayer;

    // Constructor and Destructor
    TrackableVehicle(CVehicle* vehicle);
    ~TrackableVehicle();

    // Public Methods
    CVector* GetLastKnownPosition() const;
    void TrackPath();
    bool ShownOnRadar();
    bool DrawLine();
    CRGBA GetColor();
    bool IsDrivenByPlayer();
    bool IsPlaneOrHeli() const;
    bool IsPlaneOrHeliFlying() const;
    bool IsPlaneOrHeliOnGround() const;
    bool IsOnGround() const;
    bool IsGroundVehicleAtAirport();
    bool IsPoliceOrArmy();
    bool IsOnAirportTerritory();

private:
    // Private Methods
    bool ShouldTrack(CVector currentPosition);
    void ClearTrack();

    // Static Helper Methods
    static bool IsPlaneOrHeli(CVehicle* vehicle);
    static bool IsPlayerFlyingPlaneOrHeli();
};

#endif // TRACKABLEVEHICLE_H
#pragma once
