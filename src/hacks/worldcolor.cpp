#include "visuals.h"
#include "../core/globals.h"

void applyWorldColor(CColor color)
{

	for (auto handle = interfaces::materialSystem->FirstMaterial(); handle != interfaces::materialSystem->InvalidMaterial(); handle = interfaces::materialSystem->NextMaterial(handle))
	{
		auto* material = interfaces::materialSystem->GetMaterial(handle);

		if (!material) continue;

		const std::string_view textureGroup = material->GetTextureGroupName();

		if (strstr(material->GetTextureGroupName(), "World")) //textureGroup.starts_with("World") || textureGroup.starts_with("Static")
			material->ColorModulate(color.r / 255.f, color.g / 255.f, color.b / 255.f);
	}

}

void visuals::WorldColor() noexcept
{
	static bool usingWorldColor = true;

	if (!globals::worldColor)
	{
		if (usingWorldColor)
		{
			applyWorldColor(CColor::White());
			usingWorldColor = false;
		}
		return;
	}
	if (!interfaces::engine->IsInGame()) return;

	usingWorldColor = true;
	applyWorldColor(CColor::Red());
}

