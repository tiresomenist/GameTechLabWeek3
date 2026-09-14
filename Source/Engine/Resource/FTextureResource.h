#pragma once

#include <d3d11.h>

class FTextureResource
{
    friend class GResourceManager;

public:
    FTextureResource() = default;
    FTextureResource(const FTextureResource&) = delete;
    FTextureResource& operator=(const FTextureResource&) = delete;

    ~FTextureResource()
    {
        if (ShaderResourceView) ShaderResourceView->Release();
    }

    ID3D11ShaderResourceView* GetShaderResourceView() const { return ShaderResourceView; }

private:
    ID3D11ShaderResourceView* ShaderResourceView = nullptr;
};
