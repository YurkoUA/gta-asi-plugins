#include "fstream"
#include "ctime"
#include "chrono"
#include "vector"
#include "map"
#include "plugin.h"
#include "common.h"
#include "CFont.h"
#include "CRadar.h"
#include "CMenuManager.h"
#include "CVehicle.h"
#include "CSprite.h"
#include "RenderWare.h"
#include "d3d9.h"

using namespace plugin;

#define LINE_WIDTH  2.5f
#define MIN_DISTANCE 5.0f

struct Airport {
	CRect Area;
	float Height;

	Airport(CRect area, float height)
		: Area(area), Height(height) {}

public:
	bool IsWithinAirport(CVector position) {
		CVector2D position2d = CVector2D(position.x, position.y);

		return Area.IsPointInside(position2d)
			&& position.z >= Height - 1;
	}

	static const Airport LS() { return Airport(CRect(2141, -2468, 1396, -2629), 13); }
	static const Airport SF() { return Airport(CRect(-1714, -191, -1002, 452), 14); }
	static const Airport LV() { return Airport(CRect(1536, 1153, 1341, 1854), 10); }
	static const Airport LV2() { return Airport(CRect(449, 2559, -77, 2466), 16); }
};

class TrackableVehicle {
public:
	CVehicle* vehicle;
	unsigned int handle;
	std::vector<CVector> path = { };
	CVector* lastKnownPosition = nullptr;
	int randomNumber;
	bool isCurrentlyDrivenByPlayer;

public:
	TrackableVehicle(CVehicle* vehicle) {
		this->vehicle = vehicle;
		this->handle = reinterpret_cast<unsigned int>(vehicle);
		this->path = { };
		this->randomNumber = rand();
		this->isCurrentlyDrivenByPlayer = IsDrivenByPlayer();
	}

	~TrackableVehicle() {
		delete lastKnownPosition;
	}

	CVector* GetLastKnownPosition() const {
		return lastKnownPosition;
	}

	void TrackPath() {
		if (!isCurrentlyDrivenByPlayer && IsDrivenByPlayer()) {
			ClearTrack();
		}

		isCurrentlyDrivenByPlayer = IsDrivenByPlayer();

		CVector currentPosition = vehicle->GetPosition();

		if (!ShouldTrack(currentPosition)) return;

		path.push_back(currentPosition);

		lastKnownPosition = new CVector(currentPosition);
	}

	bool ShownOnRadar() {
		if (!IsPlayerFlyingPlaneOrHeli()) {
			return IsPlaneOrHeliFlying();
		}

		return IsPlaneOrHeli() || IsGroundVehicleAtAirport();
	}

	bool DrawLine() {
		return ShownOnRadar() && !IsOnGround();
	}

	CRGBA GetColor() {
		if (IsPoliceOrArmy()) return CRGBA(0, 0, 255);

		if (IsPlaneOrHeliFlying()) return CRGBA(0, 255, 0);

		if (IsPlaneOrHeliOnGround()) return CRGBA(255, 255, 0);

		if (IsGroundVehicleAtAirport()) return CRGBA(255, 255, 255);

		return CRGBA(255, 0, 0);
	};

	bool IsDrivenByPlayer() {
		return vehicle->m_pDriver && vehicle->m_pDriver->IsPlayer();
	}

	bool IsPlaneOrHeli() const {
		return IsPlaneOrHeli(vehicle);
	}

	bool IsPlaneOrHeliFlying() const {
		return IsPlaneOrHeli() && !IsOnGround();
	}

	bool IsPlaneOrHeliOnGround() const {
		return IsPlaneOrHeli() && IsOnGround();
	}

	bool IsOnGround() const {
		return !vehicle->m_nVehicleFlags.bEngineOn;
	}

	bool IsGroundVehicleAtAirport() {
		return !IsPlaneOrHeli() && IsOnAirportTerritory();
	}

	bool IsPoliceOrArmy() {
		unsigned short modelId = vehicle->m_nModelIndex;

		return modelId == MODEL_CARGOBOB
			|| modelId == MODEL_HUNTER
			|| modelId == MODEL_POLMAV
			|| modelId == MODEL_HYDRA
			;
	}

	bool IsOnAirportTerritory() {
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

private:
	bool ShouldTrack(CVector currentPosition) {
		if (!ShownOnRadar()) return false;
		if (!lastKnownPosition) return true;

		float distance = DistanceBetweenPoints(*lastKnownPosition, currentPosition);

		return distance > MIN_DISTANCE;
	}

	void ClearTrack() {
		lastKnownPosition = nullptr;
		path.clear();
	}

	bool IsDrivenByPlayer() const {
		return FindPlayerPed(0)->m_pVehicle == vehicle;
	}

private:
	static bool IsPlaneOrHeli(CVehicle* vehicle) {
		return vehicle->m_nVehicleSubClass == VEHICLE_PLANE || vehicle->m_nVehicleSubClass == VEHICLE_HELI;
	}

	static bool IsPlayerFlyingPlaneOrHeli() {
		CPlayerPed* playerPed = FindPlayerPed(0);

		CVehicle* playerVehicle = playerPed->m_pVehicle;

		return playerVehicle && IsPlaneOrHeli(playerVehicle);
	}
};

class Flightradar {
public:
	static std::map<unsigned int, TrackableVehicle*> trackedVehicles;
	static std::ofstream logFile;

	Flightradar() {
		Events::drawBlipsEvent += [] {
			if (!FindPlayerPed(0)) return;

			for (const auto& [key, value] : trackedVehicles) {
				DrawVehicle(value);
			}
			};

		Events::vehicleRenderEvent += [](CVehicle* vehicle) {
			unsigned int handle = reinterpret_cast<unsigned int>(vehicle);

			if (trackedVehicles.contains(handle)) return;

			TrackableVehicle* trackableVehicle = new TrackableVehicle(vehicle);

			if (!trackableVehicle->ShownOnRadar()) return;

			trackableVehicle->TrackPath();
			trackedVehicles.insert({ handle, trackableVehicle });
			LogVehicleTracked(trackableVehicle);
			};

		Events::vehicleDtorEvent += [](CVehicle* vehicle) {
			unsigned int handle = reinterpret_cast<unsigned int>(vehicle);

			if (trackedVehicles.contains(handle)) {
				TrackableVehicle* trackable = trackedVehicles[handle];

				trackedVehicles.erase(handle);

				delete trackable;
			}
			};
	};

	static void DrawVehicle(TrackableVehicle* trackableVehicle) {
		trackableVehicle->TrackPath();
		DrawMarker(trackableVehicle);

		if (trackableVehicle->DrawLine()) {
			DrawLine(trackableVehicle);
		}

		char text[16];
		sprintf_s(text, "Points: %d", trackableVehicle->path.size());

		DrawTextOnEntity(trackableVehicle->vehicle, text);
	}

	static void DrawMarker(TrackableVehicle* trackableVehicle) {
		CVector& objectPosn = trackableVehicle->vehicle->GetPosition();
		CVector2D coords;
		CRadar::TransformRealWorldPointToRadarSpace(coords, CVector2D(objectPosn.x, objectPosn.y));
		float distance = CRadar::LimitRadarPoint(coords);

		if (distance < 1.0f) {
			CVector2D screen;
			CRadar::TransformRadarPointToScreenSpace(screen, coords);
			CVector playerPosn = FindPlayerCentreOfWorld_NoInteriorShift(0);

			unsigned char blipType = RADAR_TRACE_NORMAL;
			if (playerPosn.z - objectPosn.z > 4.0f)
				blipType = RADAR_TRACE_HIGH;
			else if (playerPosn.z - objectPosn.z < -2.0f)
				blipType = RADAR_TRACE_LOW;

			auto color = trackableVehicle->GetColor();

			CRadar::ShowRadarTraceWithHeight(screen.x, screen.y, 1, color.r, color.g, color.b, color.a, blipType);
		}
	}

	static void DrawLine(TrackableVehicle* trackableVehicle) {
		int nodesCount = trackableVehicle->path.size();

		CVector2D* radarPoints = new CVector2D[nodesCount];
		RwIm2DVertex* lineVerts = new RwIm2DVertex[nodesCount * 4];

		for (int i = 0; i < nodesCount; i++) {
			CVector point = trackableVehicle->path[i];;
			CVector2D point2d = CVector2D(point.x, point.y);
			CVector2D tmpPoint;

			CRadar::TransformRealWorldPointToRadarSpace(tmpPoint, point2d);

			if (!FrontEndMenuManager.m_bDrawRadarOrMap)
				CRadar::TransformRadarPointToScreenSpace(radarPoints[i], tmpPoint);
			else {
				CRadar::LimitRadarPoint(tmpPoint);
				CRadar::TransformRadarPointToScreenSpace(radarPoints[i], tmpPoint);
				radarPoints[i].x *= static_cast<float>(RsGlobal.maximumWidth) / 640.0f;
				radarPoints[i].y *= static_cast<float>(RsGlobal.maximumHeight) / 448.0f;
				CRadar::LimitToMap(&radarPoints[i].x, &radarPoints[i].y);
			}
		}

		if (!FrontEndMenuManager.m_bDrawRadarOrMap
			&& reinterpret_cast<D3DCAPS9 const*>(RwD3D9GetCaps())->RasterCaps & D3DPRASTERCAPS_SCISSORTEST)
		{
			RECT rect;
			CVector2D posn;
			CRadar::TransformRadarPointToScreenSpace(posn, CVector2D(-1.0f, -1.0f));
			rect.left = static_cast<LONG>(posn.x + 2.0f);
			rect.bottom = static_cast<LONG>(posn.y - 2.0f);
			CRadar::TransformRadarPointToScreenSpace(posn, CVector2D(1.0f, 1.0f));
			rect.right = static_cast<LONG>(posn.x - 2.0f);
			rect.top = static_cast<LONG>(posn.y + 2.0f);
			reinterpret_cast<IDirect3DDevice9*>(GetD3DDevice())->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
			reinterpret_cast<IDirect3DDevice9*>(GetD3DDevice())->SetScissorRect(&rect);
		}

		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);

		unsigned int vertIndex = 0;
		for (short i = 0; i < (nodesCount - 1); i++) {
			CVector2D point[4], shift[2];
			CVector2D dir = radarPoints[i + 1] - radarPoints[i];
			float angle = atan2(dir.y, dir.x);
			if (!FrontEndMenuManager.m_bDrawRadarOrMap) {
				shift[0].x = cosf(angle - 1.5707963f) * LINE_WIDTH;
				shift[0].y = sinf(angle - 1.5707963f) * LINE_WIDTH;
				shift[1].x = cosf(angle + 1.5707963f) * LINE_WIDTH;
				shift[1].y = sinf(angle + 1.5707963f) * LINE_WIDTH;
			}
			else {
				// Adjust map zoom value and clamp it within the valid range
				float mp = FrontEndMenuManager.m_fMapZoom;
				if (mp < 140.0f)
					mp = 140.0f;
				else if (mp > 960.0f)
					mp = 960.0f;

				// Scale the map zoom factor to a normalized range
				mp = (mp - 140.0f) / (960.0f - 140.0f); // Normalize mp to range [0, 1]
				mp = mp * 0.6f + 0.4f;                  // Scale to range [0.4, 1.0]

				// Calculate the shifted points
				shift[0].x = cosf(angle - 1.5707963f) * LINE_WIDTH * mp;
				shift[0].y = sinf(angle - 1.5707963f) * LINE_WIDTH * mp;
				shift[1].x = cosf(angle + 1.5707963f) * LINE_WIDTH * mp;
				shift[1].y = sinf(angle + 1.5707963f) * LINE_WIDTH * mp;

			}

			auto color = GetHeightColor(trackableVehicle->path[i].z);

			Setup2dVertex(color, lineVerts[vertIndex + 0], radarPoints[i].x + shift[0].x, radarPoints[i].y + shift[0].y);
			Setup2dVertex(color, lineVerts[vertIndex + 1], radarPoints[i + 1].x + shift[0].x, radarPoints[i + 1].y + shift[0].y);
			Setup2dVertex(color, lineVerts[vertIndex + 2], radarPoints[i].x + shift[1].x, radarPoints[i].y + shift[1].y);
			Setup2dVertex(color, lineVerts[vertIndex + 3], radarPoints[i + 1].x + shift[1].x, radarPoints[i + 1].y + shift[1].y);

			vertIndex += 4;
		}

		RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, lineVerts, 4 * (nodesCount - 1));

		if (!FrontEndMenuManager.m_bDrawRadarOrMap
			&& reinterpret_cast<D3DCAPS9 const*>(RwD3D9GetCaps())->RasterCaps & D3DPRASTERCAPS_SCISSORTEST)
		{
			reinterpret_cast<IDirect3DDevice9*>(GetD3DDevice())->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		}
	}

	static void Setup2dVertex(CRGBA color, RwIm2DVertex& vertex, float x, float y) {
		vertex.x = x;
		vertex.y = y;
		vertex.u = vertex.v = 0.0f;
		vertex.z = CSprite2d::NearScreenZ + 0.0001f;
		vertex.rhw = CSprite2d::RecipNearClip;
		vertex.emissiveColor = RWRGBALONG(color.r, color.g, color.b, color.a);
	}

	static CRGBA GetHeightColor(float height) {
		const int NUMBER_OF_COLORS = 33;
		const int LAST_COLOR = NUMBER_OF_COLORS - 1;

		const int STEP = 12;
		const int MAX_HEIGHT = NUMBER_OF_COLORS * STEP;

		CRGBA colors[NUMBER_OF_COLORS] =
		{
			CRGBA(255, 255, 255),
			CRGBA(255, 224, 98),
			CRGBA(255, 234, 0),
			CRGBA(240, 255, 0),
			CRGBA(204, 255, 0),
			CRGBA(66, 255, 0),
			CRGBA(30, 255, 0),
			CRGBA(0, 255, 12),
			CRGBA(0, 255, 54),
			CRGBA(0, 255, 114),
			CRGBA(0, 255, 156),
			CRGBA(0, 255, 210),
			CRGBA(0, 255, 228),
			CRGBA(0, 234, 255),
			CRGBA(0, 192, 255),
			CRGBA(0, 168, 255),
			CRGBA(0, 150, 255),
			CRGBA(0, 120, 255),
			CRGBA(0, 84, 255),
			CRGBA(0, 48, 255),
			CRGBA(0, 30, 255),
			CRGBA(0, 0, 255),
			CRGBA(18, 0, 255),
			CRGBA(36, 0, 255),
			CRGBA(54, 0, 255),
			CRGBA(78, 0, 255),
			CRGBA(96, 0, 255),
			CRGBA(120, 0, 255),
			CRGBA(150, 0, 255),
			CRGBA(174, 0, 255),
			CRGBA(216, 0, 255),
			CRGBA(255, 0, 228),
			CRGBA(255, 0, 255)
		};

		if (height <= 0) return colors[0];
		if (height >= MAX_HEIGHT) return colors[LAST_COLOR];

		int index = (height / STEP) - 1;

		if (index < 0) index = 0;

		return colors[index];
	}

	static void LogVehicleTracked(TrackableVehicle* trackableVehicle) {
		auto now = std::chrono::system_clock::now();
		std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

		logFile << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %H:%M:%S")
			<< " | Vehicle ID: " << trackableVehicle->vehicle->m_nModelIndex
			<< " | Handle: " << trackableVehicle->handle;

		CVector* position = trackableVehicle->GetLastKnownPosition();

		if (position) {
			logFile << " | Position: "
				<< " X = "
				<< position->x
				<< "; Y = "
				<< position->y
				<< "; Z = "
				<< position->z;
		}

		logFile << std::endl;
		logFile << std::endl;
	}

	static void DrawTextOnEntity(CEntity* entity, const char* text) {
		CVector& posn = entity->GetPosition();
		RwV3d rwp = { posn.x, posn.y, posn.z + 1.5f };
		RwV3d screenCoors;
		float w, h;

		if (CSprite::CalcScreenCoors(rwp, &screenCoors, &w, &h, true, true)) {
			DrawTextOnScreen(screenCoors.x, screenCoors.y, text);
		}
	}

	static void DrawTextOnScreen(float x, float y, const char* text) {
		// Setup font properties
		CFont::SetBackground(false, false);
		CFont::SetProportional(true);
		CFont::SetJustify(false);
		CFont::SetFontStyle(FONT_SUBTITLES);
		CFont::SetEdge(1);
		CFont::SetDropColor(CRGBA(0, 0, 0, 255)); // Black shadow
		CFont::SetColor(CRGBA(255, 255, 255, 255)); // White text
		CFont::SetScale(SCREEN_MULTIPLIER(0.4f), SCREEN_MULTIPLIER(0.8f));

		// Print the text at the specified screen position
		CFont::PrintString(x, y, text);
	}
} FlightradarPlugin;

std::ofstream Flightradar::logFile = std::ofstream("flightradar.log.txt", std::ios::trunc);
std::map<unsigned int, TrackableVehicle*> Flightradar::trackedVehicles;
