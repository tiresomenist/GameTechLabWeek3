#pragma once

#include <d3d11.h>
#include <unordered_map>

#include "Core/Container/FString.h"
#include "Engine/Renderer/Text/FGlyphInfo.h"

class FFontAtlas
{
public:
	bool Build(ID3D11Device* Device, const FString& TTFPath, float PixelHeight);
	void Release();

	const FGlyphInfo* FindGlyph(char32_t Codepoint) const;
	ID3D11ShaderResourceView* GetSRV() const { return AtlasSRV; }

private:
	std::unordered_map<char32_t, FGlyphInfo> Glyphs;
	ID3D11Texture2D* AtlasTexture = nullptr;
	ID3D11ShaderResourceView* AtlasSRV = nullptr;
};