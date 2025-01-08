#include "plugin.h"
#include "CVehicle.h"
#include "CFont.h"
#include "CSprite.h"

using namespace plugin;

class VehicleIdDisplay {
public:
    VehicleIdDisplay() {
		Events::vehicleRenderEvent += [](CVehicle* vehicle) {
			if (!vehicle) return;
			//if (vehicle->m_pDriver && vehicle->m_pDriver->IsPlayer()) return;

			CVector& posn = vehicle->GetPosition();
			RwV3d rwp = { posn.x, posn.y, posn.z + 1.5f };
			RwV3d screenCoors;
			float w, h;

			if (CSprite::CalcScreenCoors(rwp, &screenCoors, &w, &h, true, true)) {
				// Draw the model ID above the vehicle
				char modelIDText[32];
				sprintf_s(modelIDText, "ID: %d; Subclass: %d", vehicle->m_nModelIndex, vehicle->m_nVehicleSubClass);

				DrawTextOnScreen(screenCoors.x, screenCoors.y, modelIDText);
			}
		};
	};

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
} VehicleIdDisplayPlugin;
