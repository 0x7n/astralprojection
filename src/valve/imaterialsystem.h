#pragma once
#include "../util/memory.h"

#include "ckeyvalues.h"

class IMaterial
{
public:
	constexpr void Modulate(float* color)
	{
		ColorModulate(color[0], color[1], color[2]);
		AlphaModulate(color[3]);
	}

public:
	enum EMaterialVarFlag : std::int32_t
	{
		IGNOREZ = 1 << 15,
		WIREFRAME = 1 << 28
	};

	constexpr void SetMaterialVarFlag(const std::int32_t flag, const bool on) noexcept
	{
		memory::Call<void>(this, 29, flag, on);
	}

	// Get the name of the material.  This is a full path to
	// the vmt file starting from "hl2/materials" (or equivalent) without
	// a file extension.
	virtual const char* GetName() const = 0;
	virtual const char* GetTextureGroupName() const = 0;

	// Returns true if this is the error material you get back from IMaterialSystem::FindMaterial if
	// the material can't be found.
	virtual bool IsErrorMaterial() const = 0;

	// Apply constant color or alpha modulation
	virtual void			AlphaModulate(float alpha) = 0;
	virtual void			ColorModulate(float r, float g, float b) = 0;

};

class IMaterialSystem
{
public:
	constexpr IMaterial* CreateMaterial(const char* name, CKeyValues* kv) noexcept
	{
		return memory::Call<IMaterial*>(this, 83, name, kv);
	}

	constexpr IMaterial* FindMaterial(const char* name) noexcept
	{
		return memory::Call<IMaterial*>(this, 84, name, nullptr, true, nullptr);
	}

	short FirstMaterial()
	{
		return memory::Call<short>(this, 86);
	}

	short NextMaterial(short handle)
	{
		return memory::Call<short>(this, 87, handle);
	}

	short InvalidMaterial()
	{
		return memory::Call<short>(this, 88);
	}

	IMaterial* GetMaterial(short handle)
	{
		return memory::Call<IMaterial*>(this, 89, handle);
	}
};
