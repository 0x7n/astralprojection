#include "hooks.h"

// include minhook for epic hookage
#include "../../ext/minhook/minhook.h"
#include "../../ext/imgui/imgui.h"
#include "../../ext/imgui/imgui_impl_win32.h"
#include "../../ext/imgui/imgui_impl_dx9.h"

#include <intrin.h>
#include <stdexcept>

#include "../hacks/aimbot.h"
#include "../hacks/misc.h"
#include "../hacks/visuals.h"

void hooks::Setup() noexcept
{
	MH_Initialize();

	// AllocKeyValuesMemory hook
	MH_CreateHook(
		memory::Get(interfaces::keyValuesSystem, 1),
		&AllocKeyValuesMemory,
		reinterpret_cast<void**>(&AllocKeyValuesMemoryOriginal)
	);

	MH_CreateHook(
		memory::Get(gui::device, 42),
		&EndScene,
		reinterpret_cast<void**>(&EndSceneOriginal)
	);

	MH_CreateHook(
		memory::Get(gui::device, 16),
		&Reset,
		reinterpret_cast<void**>(&ResetOriginal)
	);

	gui::DestroyDirectX();

	// CreateMove hook
	MH_CreateHook(
		memory::Get(interfaces::clientMode, 24),
		&CreateMove,
		reinterpret_cast<void**>(&CreateMoveOriginal)
	);

	// DrawModel hook
	MH_CreateHook(
		memory::Get(interfaces::studioRender, 29),
		&DrawModel,
		reinterpret_cast<void**>(&DrawModelOriginal)
	);

	/*
	
		// framestage hook


		MH_CreateHook(
		memory::Get(interfaces::client, 37),
		&ClientFrameStage,
		reinterpret_cast<void**>(&originalClientFrameStageFn)
	);
	
	*/
	
	// dopostscreenspaceeffects
	MH_CreateHook(
		memory::Get(interfaces::clientMode, 44), // function is @ index 44
		&DoPostScreenSpaceEffects,
		reinterpret_cast<void**>(&doPostScreenSpaceEffectsFnOriginal)
	);

	// PaintTraverse hooks
	MH_CreateHook(
		memory::Get(interfaces::panel, 41),
		&PaintTraverse,
		reinterpret_cast<void**>(&PaintTraverseOriginal)
	);

	MH_EnableHook(MH_ALL_HOOKS);
}

void hooks::Destroy() noexcept
{
	// restore hooks
	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);

	// uninit minhook
	MH_Uninitialize();
}

long __stdcall hooks::EndScene(IDirect3DDevice9* device) noexcept
{
	static const auto returnAddress = _ReturnAddress();

	const auto result = EndSceneOriginal(device, device);

	// stop endscene getting called twice
	if (_ReturnAddress() == returnAddress)
		return result;

	if (!gui::setup)
		gui::SetupMenu(device);

	
	gui::Render();

	// 	if (globals::watermark)
	//		gui::RenderWatermark();

	return result;
}

HRESULT __stdcall hooks::Reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params) noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	const auto result = ResetOriginal(device, device, params);
	ImGui_ImplDX9_CreateDeviceObjects();
	return result;
}

void* __stdcall hooks::AllocKeyValuesMemory(const std::int32_t size) noexcept
{
	// if function is returning to speficied addresses, return nullptr to "bypass"
	if (const std::uint32_t address = reinterpret_cast<std::uint32_t>(_ReturnAddress());
		address == reinterpret_cast<std::uint32_t>(memory::allocKeyValuesEngine) ||
		address == reinterpret_cast<std::uint32_t>(memory::allocKeyValuesClient)) 
		return nullptr;

	// return original
	return AllocKeyValuesMemoryOriginal(interfaces::keyValuesSystem, size);
}

bool __stdcall hooks::CreateMove(float frameTime, CUserCmd* cmd) noexcept
{
	// make sure this function is being called from CInput::CreateMove
	if (!cmd->commandNumber)
		return CreateMoveOriginal(interfaces::clientMode, frameTime, cmd);

	// this would be done anyway by returning true
	if (CreateMoveOriginal(interfaces::clientMode, frameTime, cmd) && globals::silentAim == true)
		interfaces::engine->SetViewAngles(cmd->viewAngles);

	// get our local player here
	globals::UpdateLocalPlayer();

	auto oldFlags = globals::localPlayer->GetFlags();

	if (globals::localPlayer && globals::localPlayer->IsAlive())
	{
		// example bhop
		hacks::RunBunnyHop(cmd);
		hacks::Radar();
		hacks::EdgeBug(cmd);
		hacks::ChangeFOV();
		hacks::EdgeJump(cmd, oldFlags);

		// run aimbot
		hacks::RunAimbot(cmd);
	}

	// this would be done anyway by returning true
	if (CreateMoveOriginal(interfaces::clientMode, frameTime, cmd) && globals::silentAim == false)
		interfaces::engine->SetViewAngles(cmd->viewAngles);

	return false;
}

void __stdcall hooks::DrawModel(void* results,const CDrawModelInfo& info,CMatrix3x4* bones,float* flexWeights,float* flexDelayedWeights,const CVector& modelOrigin,const std::int32_t flags) noexcept
{
	// make sure local player && renderable pointer != nullptr
	if (globals::localPlayer && info.renderable)
	{
		CEntity* entity = info.renderable->GetIClientUnknown()->GetBaseEntity();

		if (entity && entity->IsPlayer() && entity->GetTeam() != globals::localPlayer->GetTeam() && globals::showChams)
		{
			static IMaterial* material = nullptr;

			if (interfaces::studioRender->IsForcedMaterialOverride())
				return;

			if(globals::materials[globals::selectedMaterial] == "flat")
				material = interfaces::materialSystem->CreateMaterial("flat",CKeyValues::FromString("UnlitGeneric",""));
			else if (globals::materials[globals::selectedMaterial] == "pearlescent")
				material = interfaces::materialSystem->CreateMaterial("pearlescent",CKeyValues::FromString("VertexLitGeneric","$phong 1 ""$basemapalphaphongmask 1 ""$pearlescent 2"));
			else if (globals::materials[globals::selectedMaterial] == "metallic")
				material = interfaces::materialSystem->CreateMaterial("metallic", CKeyValues::FromString("VertexLitGeneric","envmap env_cubemap ""$model 1""$flat 1""$selfillium 1"));
			else if (globals::materials[globals::selectedMaterial] == "plastic")
				material = interfaces::materialSystem->CreateMaterial("flat", CKeyValues::FromString("VertexLitGeneric", "$bumpmap models/inventory_items/trophy_majors/matte_metal_normal""$additive 1""$normalmapalphaenvmapmask 1"));
			else
				material = interfaces::materialSystem->FindMaterial(globals::materials[globals::selectedMaterial]);


			interfaces::studioRender->SetAlphaModulation(1.f);

			if (!globals::visibleOnly)
			{
				material->SetMaterialVarFlag(IMaterial::IGNOREZ, true);
				interfaces::studioRender->SetColorModulation(globals::chamsHidden);
				interfaces::studioRender->ForcedMaterialOverride(material);
				DrawModelOriginal(interfaces::studioRender, results, info, bones, flexWeights, flexDelayedWeights, modelOrigin, flags);
			}

			// do not show through walls
			material->SetMaterialVarFlag(IMaterial::IGNOREZ, false);
			interfaces::studioRender->SetColorModulation(globals::chamsVisible);
			interfaces::studioRender->ForcedMaterialOverride(material);
			DrawModelOriginal(interfaces::studioRender, results, info, bones, flexWeights, flexDelayedWeights, modelOrigin, flags);

			// reset the material override + return from hook
			return interfaces::studioRender->ForcedMaterialOverride(nullptr);
		}
	}

	// call original func
	DrawModelOriginal(interfaces::studioRender, results, info, bones, flexWeights, flexDelayedWeights, modelOrigin, flags);
}

void __stdcall hooks::ClientFrameStage(CEntity::ClientFrameStage frame_stage)
{

	switch (frame_stage)
	{
	case CEntity::ClientFrameStage::FRAME_UNDEFINED:                       break;
	case CEntity::ClientFrameStage::FRAME_START:                           break;
	case CEntity::ClientFrameStage::FRAME_NET_UPDATE_START:                break;
	case CEntity::ClientFrameStage::FRAME_NET_UPDATE_POSTDATAUPDATE_START:				// Other models like localplayer, players and hands
		break;
	case CEntity::ClientFrameStage::FRAME_NET_UPDATE_POSTDATAUPDATE_END:   break;
	case CEntity::ClientFrameStage::FRAME_NET_UPDATE_END:
		break;
	case CEntity::ClientFrameStage::FRAME_RENDER_START:		// Run here too to avoid model flickering online
		//visuals::WorldColor();	
		break;
	case CEntity::ClientFrameStage::FRAME_RENDER_END:                      break;
	default:                                    break;
	}

	originalClientFrameStageFn(interfaces::client, frame_stage);
}

void __stdcall hooks::DoPostScreenSpaceEffects(const void* viewSetup) noexcept
{
	if (globals::localPlayer && interfaces::engine->IsInGame())
	{
		for (int i = 0; i < interfaces::glow->glowObjects.size; i++)
		{
			IGlowManager::CGlowObject& glowObject = interfaces::glow->glowObjects[i];

			if (!globals::glow)
				return;

			if (glowObject.IsUnused())
				continue;

			if (!glowObject.entity)
				continue;

			switch (glowObject.entity->GetClientClass()->classID)
			{
			case CClientClass::CCSPlayer:

				if (!glowObject.entity->IsAlive())
					break;

				if (glowObject.entity->GetTeam() != globals::localPlayer->GetTeam()) // enemy
				{
					glowObject.SetColor(globals::enemyGlowCol[0], globals::enemyGlowCol[1], globals::enemyGlowCol[2]);
				}
				else if (globals::teamGlow)
				{
					glowObject.SetColor(globals::teammateGlowCol[0], globals::teammateGlowCol[1], globals::teammateGlowCol[2]);
				}

				break;

			default:
				break;
			}
		}
	}

	doPostScreenSpaceEffectsFnOriginal(interfaces::clientMode, viewSetup);
}

void __stdcall hooks::PaintTraverse(std::uintptr_t vguiPanel, bool forceRepaint, bool allowForce) noexcept
{

	if (vguiPanel == interfaces::engineVGui->GetPanel(PANEL_TOOLS))
	{
		if (globals::localPlayer && interfaces::engine->IsInGame())
		{
			hacks::VelocityGraph();
			for (int i = 1; i < interfaces::globals->maxClients; i++)
			{
				CEntity* player = interfaces::entityList->GetEntityFromIndex(i);

				if (!player)
					continue;

				if (player->IsDormant() || !player->IsAlive())
					continue;

				if (player->GetTeam() == globals::localPlayer->GetTeam())
					continue;

				if (!globals::localPlayer->IsAlive())
					if (globals::localPlayer->GetObserverTarget() == player)
						continue;



				if (globals::esp)
				{
					// players bone matrix
					CMatrix3x4 bones[128];
					if (!player->SetupBones(bones, 128, 0x7FF0, interfaces::globals->currentTime))
						continue;

					// screen position of head
					CVector top;
					if (interfaces::debugOverlay->ScreenPosition(bones[8].Origin() + CVector{ 0.f,0.f,11.f }, top))
						continue;

					// screen position of feet
					CVector bottom;
					if (interfaces::debugOverlay->ScreenPosition(player->GetAbsOrigin() - CVector{ 0.f,0.f,9.f }, bottom))
						continue;

					// height of box
					const float h = bottom.y - top.y;

					// use the height to determine the width
					const float w = h * 0.3f;

					const auto left = static_cast<int>(top.x - w);
					const auto right = static_cast<int>(top.x + w);

					// set drawing color
					interfaces::surface->DrawSetColor(255, 255, 255, 255);

					// draw normal box
					interfaces::surface->DrawOutlinedRect(left, top.y, right, bottom.y);

					interfaces::surface->DrawSetColor(0, 0, 0, 255);

					if (globals::espOutline)
					{
						interfaces::surface->DrawOutlinedRect(left - 1, top.y - 1, right + 1, bottom.y + 1);
						interfaces::surface->DrawOutlinedRect(left + 1, top.y + 1, right - 1, bottom.y - 1);
					}

					if (globals::espHealthBar == true)
					{
						// health bar outline
						interfaces::surface->DrawOutlinedRect(left - 6, top.y - 1, left - 3, bottom.y + 1);

						// health to percentage
						const float healthFrac = player->GetHealth() * 0.01f;

						// set the healthbat color to split between red and green
						interfaces::surface->DrawSetColor((1.f - healthFrac) * 255, 255 * healthFrac, 0, 255);

						// draw healthbar
						interfaces::surface->DrawFilledRect(left - 5, bottom.y - (h * healthFrac), left - 4, bottom.y);
					}
				}
			
				if (globals::boneESP)
				{
					for (int i = 0; i < 0; i++)
					{

					}
				}
			}
		}
	}

	//call original function
	PaintTraverseOriginal(interfaces::panel, vguiPanel,forceRepaint, allowForce);
}