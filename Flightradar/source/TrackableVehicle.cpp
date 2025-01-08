#include "common.h"
#include "TrackableVehicle.h"

#define MIN_DISTANCE 5.0f

// Constructor
TrackableVehicle::TrackableVehicle(CVehicle* vehicle)
    : vehicle(vehicle),
    handle(reinterpret_cast<unsigned int>(vehicle)),
    path({}),
    lastKnownPosition(nullptr),
    randomNumber(rand()),
    isCurrentlyDrivenByPlayer(IsDrivenByPlayer()) {}

// Destructor
TrackableVehicle::~TrackableVehicle() {
    delete lastKnownPosition;
}

// Public Methods
CVector* TrackableVehicle::GetLastKnownPosition() const {
    return lastKnownPosition;
}

void TrackableVehicle::TrackPath() {
    if (!isCurrentlyDrivenByPlayer && IsDrivenByPlayer()) {
        ClearTrack();
    }

    isCurrentlyDrivenByPlayer = IsDrivenByPlayer();
    CVector currentPosition = vehicle->GetPosition();

    if (!ShouldTrack(currentPosition)) return;

    path.push_back(currentPosition);
    lastKnownPosition = new CVector(currentPosition);
}

bool TrackableVehicle::ShownOnRadar() {
    if (!IsPlayerFlyingPlaneOrHeli()) {
        return IsPlaneOrHeliFlying();
    }

    return IsPlaneOrHeli() || IsGroundVehicleAtAirport();
}

bool TrackableVehicle::DrawLine() {
    return ShownOnRadar() && !IsOnGround();
}

CRGBA TrackableVehicle::GetColor() {
    if (IsPoliceOrArmy()) return CRGBA(0, 0, 255);
    if (IsPlaneOrHeliFlying()) return CRGBA(0, 255, 0);
    if (IsPlaneOrHeliOnGround()) return CRGBA(255, 255, 0);
    if (IsGroundVehicleAtAirport()) return CRGBA(255, 255, 255);
    return CRGBA(255, 0, 0);
}

bool TrackableVehicle::IsDrivenByPlayer() {
    return vehicle->m_pDriver && vehicle->m_pDriver->IsPlayer();
}

bool TrackableVehicle::IsPlaneOrHeli() const {
    return IsPlaneOrHeli(vehicle);
}

bool TrackableVehicle::IsPlaneOrHeliFlying() const {
    return IsPlaneOrHeli() && !IsOnGround();
}

bool TrackableVehicle::IsPlaneOrHeliOnGround() const {
    return IsPlaneOrHeli() && IsOnGround();
}

bool TrackableVehicle::IsOnGround() const {
    return !vehicle->m_nVehicleFlags.bEngineOn;
}

bool TrackableVehicle::IsGroundVehicleAtAirport() {
    return !IsPlaneOrHeli() && IsOnAirportTerritory();
}

bool TrackableVehicle::IsPoliceOrArmy() {
    unsigned short modelId = vehicle->m_nModelIndex;

    return modelId == MODEL_CARGOBOB
        || modelId == MODEL_HUNTER
        || modelId == MODEL_POLMAV
        || modelId == MODEL_HYDRA;
}

bool TrackableVehicle::IsOnAirportTerritory() {
    CVector position = vehicle->GetPosition();

    Airport airportLS = Airport::LS();
    Airport airportSF = Airport::SF();
    Airport airportLV = Airport::LV();
    Airport airportLV2 = Airport::LV2();

    return airportLS.IsWithinAirport(position)
        || airportSF.IsWithinAirport(position)
        || airportLV.IsWithinAirport(position)
        || airportLV2.IsWithinAirport(position);
}

// Private Methods
bool TrackableVehicle::ShouldTrack(CVector currentPosition) {
    if (!ShownOnRadar()) return false;
    if (!lastKnownPosition) return true;

    float distance = DistanceBetweenPoints(*lastKnownPosition, currentPosition);
    return distance > MIN_DISTANCE;
}

void TrackableVehicle::ClearTrack() {
    lastKnownPosition = nullptr;
    path.clear();
}

// Static Helper Methods
bool TrackableVehicle::IsPlaneOrHeli(CVehicle* vehicle) {
    return vehicle->m_nVehicleSubClass == VEHICLE_PLANE || vehicle->m_nVehicleSubClass == VEHICLE_HELI;
}

bool TrackableVehicle::IsPlayerFlyingPlaneOrHeli() {
    CPlayerPed* playerPed = FindPlayerPed(0);
    CVehicle* playerVehicle = playerPed->m_pVehicle;

    return playerVehicle && IsPlaneOrHeli(playerVehicle);
}
