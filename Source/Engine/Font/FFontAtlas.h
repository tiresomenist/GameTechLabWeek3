#pragma once

#include <d3d11.h>
#include "Engine/Resource/DDSTextureLoader11.h"
#include <wrl/client.h>
#include <string>
#include "../../Core/Container/TMap.h"
#include "Engine/Renderer/FVertexSimple.h"

struct CharacterInfo 
{
    float u;
    float v;
    float width;
    float height;
};

class FFontAtlas 
{
public:
    static FFontAtlas& GetInstance()
    {
        static FFontAtlas Instance;
        return Instance;
    }
    FFontAtlas(const FFontAtlas&) = delete;
    FFontAtlas& operator=(const FFontAtlas&) = delete;

    bool Initialize(ID3D11Device* InDevice, const std::wstring& InFilePath);
    void Release();

    bool GetCharacterInfo(wchar_t InChar, CharacterInfo& OutInfo) const;

    ID3D11ShaderResourceView* GetSRV() const { return TextureSRV.Get(); }
    ID3D11ShaderResourceView* const* GetSRVAddress() const { return TextureSRV.GetAddressOf(); }

    //void BuildSingleCharQuad(wchar_t InChar, float InQuadSize, FFontVertex OutVertices[4], uint16_t OutIndices[6]) const;

private:
    FFontAtlas() = default;
    ~FFontAtlas() { Release(); }
    void BuildCharacterMap();

private:
    Microsoft::WRL::ComPtr<ID3D11Resource> AtlasTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureSRV;

    TMap<wchar_t, CharacterInfo> CharInfoMap;

    static constexpr int GridCols = 16;
    static constexpr int GridRows = 16;
    static constexpr float CellUVSize = 1.0f / 16.0f;
};