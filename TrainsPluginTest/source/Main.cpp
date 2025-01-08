#include "plugin.h"
#include "CFont.h"
#include "CSprite.h"
#include "CVehicle.h"
#include "CTrain.h"

using namespace plugin;

class TrainsPluginTest {
public:
	TrainsPluginTest() {
		Events::vehicleRenderEvent += [](CVehicle* vehicle) {
			if (vehicle->m_nVehicleSubClass != VEHICLE_TRAIN) return;

			CTrain* train = static_cast<CTrain*>(vehicle);

			if (!train) return;

			if (train->m_nTrainFlags.bIsFrontCarriage != 1) return;

			char modelIDText[32];
			sprintf_s(modelIDText, "%d. Track: %d. Tunnel: %d", train->m_nModelIndex, train->m_nTrackId, train->IsInTunnel());

			DrawTextOnEntity(train, modelIDText);
			};
	};

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
} TrainsPluginTestPlugin;
