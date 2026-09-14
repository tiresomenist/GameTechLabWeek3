#include "pch.h"
#include "FTexture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../ThirdParty/stb/stb_image.h"

FTexture::FTexture()
    : TextureSRV(nullptr)
    , SamplerState(nullptr)
    , Width(0)
    , Height(0)
{
}

FTexture::~FTexture()
{
    Release();
}

bool FTexture::LoadFromFile(ID3D11Device* Device, const FString& FilePath)
{
    if (!Device) return false;
    Release();

    int Channels = 0;
    int LoadedWidth = 0;
    int LoadedHeight = 0;

    stbi_set_flip_vertically_on_load(false);
    unsigned char* Pixels = stbi_load(FilePath.c_str(), &LoadedWidth, &LoadedHeight, &Channels, 4);
    if (!Pixels)
    {
        return false;
    }

    Width = LoadedWidth;
    Height = LoadedHeight;

    D3D11_TEXTURE2D_DESC TexDesc = {};
    TexDesc.Width = static_cast<UINT>(Width);
    TexDesc.Height = static_cast<UINT>(Height);
    TexDesc.MipLevels = 1;
    TexDesc.ArraySize = 1;
    TexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    TexDesc.SampleDesc.Count = 1;
    TexDesc.Usage = D3D11_USAGE_IMMUTABLE;
    TexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA SubData = {};
    SubData.pSysMem = Pixels;
    SubData.SysMemPitch = static_cast<UINT>(Width * 4);

    ID3D11Texture2D* Texture2D = nullptr;
    HRESULT Hr = Device->CreateTexture2D(&TexDesc, &SubData, &Texture2D);

    stbi_image_free(Pixels);

    if (FAILED(Hr))
    {
        return false;
    }

    Hr = Device->CreateShaderResourceView(Texture2D, nullptr, &TextureSRV);
    Texture2D->Release();

    if (FAILED(Hr))
    {
        return false;
    }

    return CreateDefaultSampler(Device);
}

bool FTexture::CreateDefaultSampler(ID3D11Device* Device)
{
    if (SamplerState) return true;

    D3D11_SAMPLER_DESC SamplerDesc = {};
    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    SamplerDesc.MinLOD = 0.0f;
    SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    HRESULT hr = Device->CreateSamplerState(&SamplerDesc, &SamplerState);
    return SUCCEEDED(hr);
}

void FTexture::Bind(ID3D11DeviceContext* Context, UINT TextureSlot, UINT SamplerSlot)
{
    if (!Context) return;

    if (TextureSRV)
    {
        Context->PSSetShaderResources(TextureSlot, 1, &TextureSRV);
    }
    if (SamplerState)
    {
        Context->PSSetSamplers(SamplerSlot, 1, &SamplerState);
    }
}

void FTexture::Unbind(ID3D11DeviceContext* Context, UINT TextureSlot, UINT SamplerSlot)
{
    if (!Context) return;

    ID3D11ShaderResourceView* NullSRV = nullptr;
    ID3D11SamplerState* NullSampler = nullptr;

    Context->PSSetShaderResources(TextureSlot, 1, &NullSRV);
    Context->PSSetSamplers(SamplerSlot, 1, &NullSampler);
}

void FTexture::Release()
{
    if (TextureSRV)
    {
        TextureSRV->Release();
        TextureSRV = nullptr;
    }
    if (SamplerState)
    {
        SamplerState->Release();
        SamplerState = nullptr;
    }

    Width = 0;
    Height = 0;
}