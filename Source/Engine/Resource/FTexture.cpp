#include "pch.h"
#include "FTexture.h"

bool FTexture::Create(ID3D11Device* Device, UINT InWidth, UINT InHeight, DXGI_FORMAT Format, UINT BytesPerPixel, const void* Pixels)
{
	Release();
	D3D11_TEXTURE2D_DESC Desc = {};
	Desc.Width = InWidth;
	Desc.Height = InHeight;
	Desc.Format = Format;
	Desc.MipLevels = 1;
	Desc.ArraySize = 1;
	Desc.SampleDesc.Count = 1;
	Desc.Usage = D3D11_USAGE_IMMUTABLE;
	Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	Desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA Data = {};
	Data.pSysMem = Pixels;
	Data.SysMemPitch = InWidth * BytesPerPixel;

	if (FAILED(Device->CreateTexture2D(&Desc, &Data, &Texture)))
	{
		return false;
	}

	if (FAILED(Device->CreateShaderResourceView(Texture, nullptr, &SRV)))
	{
		Release();
		return false;
	}

	Width = InWidth;
	Height = InHeight;
	return true;
}

void FTexture::Release()
{
	if (SRV)
	{
		SRV->Release();
		SRV = nullptr;
	}
	if (Texture)
	{
		Texture->Release();
		Texture = nullptr;
	}
}
