#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "Core/Core.h"
#include "Core/Container/FString.h"

class FTextureResource
{
public:
    FTextureResource() = default;
    ~FTextureResource() = default;

    FTextureResource(const FTextureResource&) = delete;
    FTextureResource& operator=(const FTextureResource&) = delete;

    // PNG를 GPU 리소스로 생성하며 실패 시 기존 리소스를 유지하고 예외를 전달함
    void Load(ID3D11Device* Device, FStringView FilePath);

    // 반환한 SRV의 소유권은 이 리소스에 유지함
    ID3D11ShaderResourceView* GetSRV() const { return ShaderResourceView.Get(); }
    uint32 GetWidth() const { return Width; }
    uint32 GetHeight() const { return Height; }

private:
    // 소멸하거나 교체할 때 COM 참조를 자동 해제함
    Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShaderResourceView;
    uint32 Width = 0;
    uint32 Height = 0;
};
