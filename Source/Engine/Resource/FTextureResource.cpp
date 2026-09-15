#include "pch.h"
#include "FTextureResource.h"

#include "Core/Util/PNGLoader.h"

#include <format>
#include <stdexcept>
#include <utility>

void FTextureResource::Load(ID3D11Device* Device, FStringView FilePath)
{
    if (!Device)
        throw std::invalid_argument("Texture device is null");

    // 좌측 상단 기준 RGBA 8비트 픽셀을 읽음
    const FPNGImage Image = PNGLoader::LoadPNG(FilePath);

    // 픽셀을 변경하지 않는 단일 밉 레벨의 2D 텍스처 설정
    D3D11_TEXTURE2D_DESC Desc{};
    Desc.Width = Image.Width;
    Desc.Height = Image.Height;
    Desc.MipLevels = 1;
    Desc.ArraySize = 1;
    Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    Desc.SampleDesc.Count = 1;
    Desc.Usage = D3D11_USAGE_IMMUTABLE;
    Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA InitialData{};
    InitialData.pSysMem = &Image.Pixels[0];
    InitialData.SysMemPitch = Image.RowPitch;

    // 모든 생성이 성공하기 전까지 기존 멤버를 변경하지 않음
    Microsoft::WRL::ComPtr<ID3D11Texture2D> NewTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> NewSRV;

    HRESULT Result = Device->CreateTexture2D(&Desc, &InitialData, NewTexture.GetAddressOf());
    if (FAILED(Result))
        throw std::runtime_error(std::format(
            "Texture creation failed: {} (HRESULT: 0x{:08X})", FilePath, static_cast<uint32>(Result)));

    // 셰이더에서 텍스처 전체를 읽는 기본 SRV를 생성함
    Result = Device->CreateShaderResourceView(NewTexture.Get(), nullptr, NewSRV.GetAddressOf());
    if (FAILED(Result))
        throw std::runtime_error(std::format(
            "Texture SRV creation failed: {} (HRESULT: 0x{:08X})", FilePath, static_cast<uint32>(Result)));

    Texture = std::move(NewTexture);
    ShaderResourceView = std::move(NewSRV);
    Width = Image.Width;
    Height = Image.Height;
    // GPU 업로드가 끝났으므로 함수 종료 시 CPU 픽셀을 해제함
}
