#include "plugin.h"
#include "CRadar.h"
#include "CPools.h"

using namespace plugin;

class VendingMachineDisplayer {
public:
    VendingMachineDisplayer() {
		Events::drawBlipsEvent += [] {
			for (CObject* object : CPools::ms_pObjectPool) {
				int modelIndex = object->m_nModelIndex;

				if (modelIndex == MODEL_CJ_SPRUNK1 || modelIndex == MODEL_CJ_EXT_SPRUNK)
					DrawMarker(object, CRGBA(0, 255, 0, 255));
				else if (modelIndex == MODEL_VENDMACH || modelIndex == MODEL_VENDMACHFD)
					DrawMarker(object, CRGBA(255, 0, 0, 255));
				else if (modelIndex == MODEL_CJ_CANDYVENDOR || modelIndex == MODEL_CJ_EXT_CANDY)
					DrawMarker(object, CRGBA(255, 255, 0, 255));
			}
			};
	};

	static void DrawMarker(CObject* object, CRGBA color) {
		CVector position = object->GetPosition();
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

			CRadar::ShowRadarTraceWithHeight(screen.x, screen.y, 1, color.r, color.g, color.b, color.a, blipType);
		}
	}
} VendingMachineDisplayerPlugin;
