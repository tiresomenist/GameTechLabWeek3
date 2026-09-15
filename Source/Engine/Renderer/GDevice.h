#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include "Core/Core.h"

struct FVertexSimple;
struct FVertexTest;

// Device 생성 → Renderer/ResourceManager 생성 → (역순으로) Renderer/ResourceManager 소멸 → Device 소멸

class GDevice
{
public:
    static GDevice* GetInstance();

    void Initialize(HWND hWnd, uint32 Width, uint32 Height);
    void Release();
    void OnResize(uint32 Width, uint32 Height);

    bool CreateDeviceAndSwapChain(HWND hWindow, uint32 Width, uint32 Height);
    void ReleaseDeviceAndSwapChain();
    bool CreateFrameBuffer();
    void ReleaseFrameBuffer();
    bool CreateDepthStencilBuffer(int32 InWidth, int32 InHeight); 
    void ReleaseDepthStencilBuffer(); 

    ID3D11Buffer* CreateVertexBuffer(const void* VertexData, UINT ByteWidth);    //ID3D11Buffer* CreateVertexBuffer(FVertexTest* vertices, UINT byteWidth);
    void ReleaseVertexBuffer(ID3D11Buffer* vertexBuffer);
    ID3D11Buffer* CreateIndexBuffer(uint32_t* indices, UINT byteWidth);
    void ReleaseIndexBuffer(ID3D11Buffer* indexBuffer);

    void SwapBuffer();
    bool IsRenderReady() const { return bRenderReady && !bGraphicsFailed; }

    ID3D11Device* GetDevice() const { return Device; };
    ID3D11DeviceContext* GetContext() const { return DeviceContext; };
    ID3D11RenderTargetView* GetFrameBufferRTV() const { return FrameBufferRTV; };
    ID3D11DepthStencilView* GetDepthStencilView() const { return DepthStencilView; };
    const D3D11_VIEWPORT& GetViewport() const { return ViewportInfo; };

private:
    ID3D11Device* Device = nullptr;
    ID3D11DeviceContext* DeviceContext = nullptr;

    IDXGISwapChain* SwapChain = nullptr;

    ID3D11Texture2D* FrameBuffer = nullptr;
    ID3D11RenderTargetView* FrameBufferRTV = nullptr;

    ID3D11Texture2D* DepthStencilBuffer = nullptr;
    ID3D11DepthStencilView* DepthStencilView = nullptr;

    D3D11_VIEWPORT ViewportInfo{};
    bool bRenderReady = false;
    bool bGraphicsFailed = false;

    GDevice() = default;
    ~GDevice() = default;
    GDevice(const GDevice&) = delete;
    GDevice& operator=(const GDevice&) = delete;
};
