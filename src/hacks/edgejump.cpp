#include "misc.h"

void hacks::EdgeJump(CUserCmd* cmd, int oldFlags) noexcept
{
	if (!globals::edgeJump) 
		return;
	if (!interfaces::engine->IsInGame()) 
		return;
	if (!globals::localPlayer || !globals::localPlayer->IsAlive())
		return;

	if( CEntity::FL_ONGROUND &&!(globals::localPlayer->GetFlags() && CEntity::FL_ONGROUND))
		cmd->buttons &= ~CUserCmd::IN_JUMP;

}