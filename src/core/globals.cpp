#include "globals.h"


void globals::UpdateLocalPlayer() noexcept
{
	// get local player index from engine
	const std::int32_t localPlayerIndex = interfaces::engine->GetLocalPlayerIndex();

	// get local player entity from client entity list
	localPlayer = interfaces::entityList->GetEntityFromIndex(localPlayerIndex);
}
