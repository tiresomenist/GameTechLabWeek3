#pragma once

#include <d3d11.h>

class FTexture
{
public:
	FTexture() = default;
	~FTexture() { Release(); }

	// 같은 COM 포인터를 두 객체가 들고 있다가 두 번 Release하는 걸 막는다
	FTexture(const FTexture&) = delete;
	FTexture& operator=(const FTexture&) = delete;

	// Pixels: 좌상단부터 한 줄씩 채운 픽셀 데이터
	// BytesPerPixel: RGBA8 = 4, R8 = 1  → SysMemPitch 계산에 쓴다
	bool Create(ID3D11Device* Device, UINT InWidth, UINT InHeight,
		DXGI_FORMAT Format, UINT BytesPerPixel, const void* Pixels);
	void Release();

	ID3D11ShaderResourceView* GetSRV() const { return SRV; }
	UINT GetWidth() const { return Width; }
	UINT GetHeight() const { return Height; }

private:
	ID3D11Texture2D* Texture = nullptr;
	ID3D11ShaderResourceView* SRV = nullptr;
	UINT Width = 0;
	UINT Height = 0;
};