#pragma once
#include "interfaces.h"
#include <array>

class CEntity;
namespace globals
{
	inline int tab;

	inline CEntity* localPlayer = nullptr;

	struct Visuals {
		std::array<float, 4U> world = { 1.f, 1.f, 1.f, 1.f };
		std::array<float, 4U> props = { 1.f, 1.f, 1.f, 1.f };
		std::array<float, 4U> sky = { 1.f, 1.f, 1.f, 1.f };
	};
	inline auto visuals = Visuals{};

	//aimbot
	inline bool aimbot = true;
	inline float aimbotFov{ 10.f };
	inline float aimSmoothing = 0.5f;
	inline bool silentAim = false;

	//esp
	inline bool esp = false;
	inline bool espHealthBar = true;
	inline bool boneESP = true;
	inline bool espOutline = false;


	// chams
	inline bool showChams = true;
	inline bool visibleOnly = false;
	inline float chamsHidden[3] = { 0.f, 1.f, 1.f };
	inline float chamsVisible[3] = { 1.f, 1.f, 0.f };
	inline const char* materials[]
	{
		"debug/debugambientcube",
		"models/inventory_items/trophy_majors/gold",
		"models/player/ct_fbi/ct_fbi_glass",
		"models/gibs/glass/glass",
		"models/inventory_items/trophy_majors/crystal_clear",
		"models/inventory_items/wildfire_gold/wildfire_gold_detail",
		"models/inventory_items/trophy_majors/crystal_blue",
		"models/inventory_items/trophy_majors/velvet",
		"models/inventory_items/cologne_prediction/cologne_prediction_glass",
		"models/inventory_items/dogtags/dogtags_outline",
		"models/inventory_items/dogtags/dogtags_lightray",
		"models/inventory_items/contributor_map_tokens/contributor_charset_color",
		"models/inventory_items/music_kit/darude_01/mp3_detail",
		"flat",
		"pearlescent",
		"plastic",
		"metallic"
	};
	inline int selectedMaterial = 0;

	//misc
	inline bool watermark = true;
	inline int playerFov = 90;
	inline bool revealRanks = true;
	inline bool bhop = true;
	inline bool edgebug = false;
	inline bool radar = false;
	inline bool edgeJump = true;

	inline bool glow = true;
	inline bool teamGlow = false;
	inline float teammateGlowCol[3] = {0.f,0.f,1.f};
	inline float enemyGlowCol[3] = { 1.f,0.f,0.f };


	inline bool worldColor;

	inline bool fovChanger = true;

	// hotkeys
	inline int chamKey;


	// update the local player pointer
	void UpdateLocalPlayer() noexcept;
}
