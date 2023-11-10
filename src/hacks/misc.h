#pragma once

// access global variables
#include "../core/globals.h"

// access interfaces
#include "../core/interfaces.h"

class CUserCmd;
namespace hacks
{
	void RunBunnyHop(CUserCmd* cmd) noexcept;
	void EdgeBug(CUserCmd* cmd) noexcept;
	void RevealRanks(CUserCmd* cmd) noexcept;
	void ChangeFOV() noexcept;
	void Radar() noexcept;
	void WaterMark() noexcept;
	void EdgeJump(CUserCmd* cmd, int oldFlags) noexcept;
	void VelocityGraph() noexcept;
}
