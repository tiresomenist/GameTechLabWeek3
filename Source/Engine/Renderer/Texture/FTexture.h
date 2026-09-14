#pragma once

#include "Core/Core.h"
#include <d3d11.h>
#include <string>
#include "Core/Container/FString.h"

class FTexture
{
public:
    FTexture();
    ~FTexture();

    bool LoadFromFile(ID3D11Device* Device, const FString& FilePath);

    void Bind(ID3D11DeviceContext* Context, UINT TextureSlot = 0, UINT SamplerSlot = 0);

    void Unbind(ID3D11DeviceContext* Context, UINT TextureSlot = 0, UINT SamplerSlot = 0);

    void Release();

    ID3D11ShaderResourceView* GetSRV() const { return TextureSRV; }
    ID3D11SamplerState* GetSamplerState() const { return SamplerState; }
    int32 GetWidth() const { return Width; }
    int32 GetHeight() const { return Height; }
    bool IsValid() const { return TextureSRV != nullptr; }

private:
    bool CreateDefaultSampler(ID3D11Device* Device);

private:
    ID3D11ShaderResourceView* TextureSRV = nullptr;
    ID3D11SamplerState* SamplerState = nullptr;

    int32 Width = 0;
    int32 Height = 0;
};