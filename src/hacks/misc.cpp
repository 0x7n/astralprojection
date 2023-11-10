#include "misc.h"

#include "../../ext/imgui/imgui.h"
#include "../../ext/imgui/imgui_impl_win32.h"
#include "../../ext/imgui/imgui_impl_dx9.h"


void hacks::RunBunnyHop(CUserCmd* cmd) noexcept
{
	if (!globals::bhop)
		return;

	if (!(globals::localPlayer->GetFlags() & CEntity::FL_ONGROUND))
		cmd->buttons &= ~CUserCmd::IN_JUMP;
}

void hacks::EdgeBug(CUserCmd* cmd) noexcept
{

	if (!globals::edgebug)
		return;

	if (!globals::localPlayer || !globals::localPlayer->IsAlive())
		return;

	if (!(globals::localPlayer->GetFlags() & CEntity::FL_ONGROUND))
		cmd->buttons &= ~CUserCmd::IN_DUCK;

}



void hacks::RevealRanks(CUserCmd* cmd) noexcept
{

	if (!globals::revealRanks)
		return;

}

void hacks::ChangeFOV() noexcept
{
	if (globals::fovChanger)
		globals::localPlayer->Fov() = globals::playerFov;
	else
		globals::localPlayer->Fov() = globals::localPlayer->DefaultFov();
}

void hacks::Radar() noexcept
{
	if (!globals::radar)
		return;

	for (int i = 1; i <= interfaces::globals->maxClients; i++)
	{
		CEntity* ent = interfaces::entityList->GetEntityFromIndex(i);

		if (!ent)
			return;

		if (ent->GetTeam() != globals::localPlayer->GetTeam() && ent->GetIndex() != globals::localPlayer->GetIndex())
		{
			ent->Spotted() = true;
		}
	}
}

void hacks::WaterMark() noexcept
{
	if (!globals::watermark)
		return;

	ImGui::Begin("Watermark", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("Astralproject.xyz");
	ImGui::End();

}

void hacks::VelocityGraph() noexcept
{
	const std::string speeD = "100";
	const auto p1 = std::wstring(speeD.begin(), speeD.end());
	interfaces::surface->DrawRenderText(p1.c_str(), wcslen(p1.c_str()));
}
