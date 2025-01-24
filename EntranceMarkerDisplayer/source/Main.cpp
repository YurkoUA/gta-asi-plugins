#include "plugin.h"
#include "CRadar.h"
#include "CEntryExit.h"
#include "CEntryExitManager.h"

using namespace plugin;

class EntranceMarkerDisplayer {
public:
    EntranceMarkerDisplayer() {
		Events::drawBlipsEvent += [] {
			for (CEntryExit* enex : CEntryExitManager::mp_poolEntryExits) {
				if (!enex) continue;

				if (enex->m_nFlags.bEnableAccess == 0) continue;

				if (enex->m_nArea > 0) continue;

				DrawMarker(enex->m_vecExitPos);
			}
			};
	};

	static void DrawMarker(CVector position) {
		CVector2D coords;
		CRadar::TransformRealWorldPointToRadarSpace(coords, CVector2D(position.x, position.y));
		float distance = CRadar::LimitRadarPoint(coords);

		if (distance < 1.0f) {
			CVector2D screen;
			CRadar::TransformRadarPointToScreenSpace(screen, coords);
			CVector playerPosn = FindPlayerCentreOfWorld_NoInteriorShift(0);

			unsigned char blipType = RADAR_TRACE_NORMAL;
			if (playerPosn.z - position.z > 4.0f)
				blipType = RADAR_TRACE_HIGH;
			else if (playerPosn.z - position.z < -2.0f)
				blipType = RADAR_TRACE_LOW;

			CRadar::ShowRadarTraceWithHeight(screen.x, screen.y, 1, 255, 255, 0, 255, blipType);
		}
	}
} EntranceMarkerDisplayerPlugin;
