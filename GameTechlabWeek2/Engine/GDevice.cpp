#include "pch.h"
#include "GDevice.h"
#include "Engine/Log.h"

GDevice* GDevice::GetInstance()
{
    static GDevice* Device = new GDevice();
    return Device;
}

void GDevice::Initialize(HWND hWindow, uint32 InWidth, uint32 InHeight)
{
    CreateDeviceAndSwapChain(hWindow, InWidth, InHeight);
    CreateFrameBuffer();
    CreateDepthStencilBuffer(static_cast<int32>(ViewportInfo.Width), static_cast<int32>(ViewportInfo.Height));
}

void GDevice::Release()
{
    ReleaseDeviceAndSwapChain();
    ReleaseFrameBuffer();
    ReleaseDepthStencilBuffer();
}

void GDevice::OnResize(uint32 Width, uint32 Height)
{
    // 최소화되었을 때 Width/Height가 0으로 들어올 수 있음
    if (Width == 0 || Height == 0)
    {
        return;
    }

    // Device 초기화 전에 WM_SIZE가 들어올 수 있음
    if (Device == nullptr ||
        DeviceContext == nullptr ||
        SwapChain == nullptr)
    {
        return;
    }

    // 기존 RTV/DSV 바인딩 해제
    DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    // 기존 크기에 종속된 리소스 해제
    ReleaseDepthStencilBuffer();
    ReleaseFrameBuffer();

    DeviceContext->Flush();

    // 스왑체인 백 버퍼 크기 변경
    const HRESULT Result = SwapChain->ResizeBuffers(
        0,                      // 기존 BufferCount 유지
        Width,
        Height,
        DXGI_FORMAT_UNKNOWN,    // 기존 포맷 유지
        0
    );

    if (FAILED(Result))
    {
        return;
    }

    // 새로운 크기로 뷰포트 갱신
    ViewportInfo.TopLeftX = 0.0f;
    ViewportInfo.TopLeftY = 0.0f;
    ViewportInfo.Width = static_cast<float>(Width);
    ViewportInfo.Height = static_cast<float>(Height);
    ViewportInfo.MinDepth = 0.0f;
    ViewportInfo.MaxDepth = 1.0f;

    // 새로운 백 버퍼를 이용해 RTV/DSV 재생성
    CreateFrameBuffer();

    if (!CreateDepthStencilBuffer(
        static_cast<int32>(Width),
        static_cast<int32>(Height)))
    {
        return;
    }

    // 새 렌더 타깃과 뷰포트 적용
    DeviceContext->OMSetRenderTargets(
        1,
        &FrameBufferRTV,
        DepthStencilView
    );

    DeviceContext->RSSetViewports(1, &ViewportInfo);
}

bool GDevice::CreateDeviceAndSwapChain(HWND hWindow, uint32 Width, uint32 Height)
{
    // 지원하는 Direct3D 기능 레벨을 정의
    D3D_FEATURE_LEVEL featurelevels[] = { D3D_FEATURE_LEVEL_11_0 };

    // 스왑 체인 설정 구조체 초기화
    DXGI_SWAP_CHAIN_DESC swapchaindesc = {};
    swapchaindesc.BufferDesc.Width = Width;
    swapchaindesc.BufferDesc.Height = Height;
    swapchaindesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;   // 색상 포맷
    swapchaindesc.SampleDesc.Count = 1;                             // 멀티 샘플링 비활성화
    swapchaindesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;    // 렌더 타겟으로 사용
    swapchaindesc.BufferCount = 2;                                  // 더블 버퍼링
    swapchaindesc.OutputWindow = hWindow;                           // 렌더링할 창 핸들
    swapchaindesc.Windowed = TRUE;                                  // 창 모드
    swapchaindesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;       // 스왑 방식

    UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG) || defined(DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        createDeviceFlags,
        featurelevels, ARRAYSIZE(featurelevels), D3D11_SDK_VERSION,
        &swapchaindesc, &SwapChain, &Device, nullptr, &DeviceContext
    );

    if (FAILED(hr)) {
        UE_LOG("[GDevice] Failed to create Direct3D 11 Device and SwapChain.");
        return false;
    }

    SwapChain->GetDesc(&swapchaindesc);

    ViewportInfo = { 0.0f, 0.0f, (float)swapchaindesc.BufferDesc.Width, (float)swapchaindesc.BufferDesc.Height, 0.0f, 1.0f };

    return true;
}


void GDevice::ReleaseDeviceAndSwapChain()
{
    if (DeviceContext)
    {
        DeviceContext->Flush();
    }
    if (SwapChain)
    {
        SwapChain->Release();
        SwapChain = nullptr;
    }
    if (Device)
    {
        Device->Release();
        Device = nullptr;
    }
    if (DeviceContext)
    {
        DeviceContext->Release();
        DeviceContext = nullptr;
    }
}

bool GDevice::CreateFrameBuffer()
{
    HRESULT hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&FrameBuffer);
    if (FAILED(hr))
    {
        UE_LOG("[GDevice] Failed to get back buffer from SwapChain. HRESULT: {}\n", hr);
        return false;
    }

    D3D11_RENDER_TARGET_VIEW_DESC framebufferRTVdesc = {};
    framebufferRTVdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;        // 색상 포맷
    framebufferRTVdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;   // 2D 텍스처
    hr = Device->CreateRenderTargetView(FrameBuffer, &framebufferRTVdesc, &FrameBufferRTV);
    if (FAILED(hr))
    {
        UE_LOG("[GDevice] Failed to create Render Target View. HRESULT: {}\n", hr);

        if (FrameBuffer)
        {
            FrameBuffer->Release();
            FrameBuffer = nullptr;
        }
        return false;
    }
    return true;
}

void GDevice::ReleaseFrameBuffer()
{
    if (FrameBuffer)
    {
        FrameBuffer->Release();
        FrameBuffer = nullptr;
    }
    if (FrameBufferRTV)
    {
        FrameBufferRTV->Release();
        FrameBufferRTV = nullptr;
    }
}

bool GDevice::CreateDepthStencilBuffer(int32 InWidth, int32 inHeight)
{
    HRESULT hr;

    D3D11_TEXTURE2D_DESC DepthStencilDesc = {};
    DepthStencilDesc.Width = InWidth;
    DepthStencilDesc.Height = inHeight;
    DepthStencilDesc.MipLevels = 1;
    DepthStencilDesc.ArraySize = 1;
    DepthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;    // 24비트 깊이(Depth) + 8비트 스텐실(Stencil)
    DepthStencilDesc.SampleDesc.Count = 1;                      // MSAA - 스왑체인 생성 시의 SampleDesc
    DepthStencilDesc.SampleDesc.Quality = 0;
    DepthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    DepthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    DepthStencilDesc.CPUAccessFlags = 0;
    DepthStencilDesc.MiscFlags = 0;

    hr = Device->CreateTexture2D(&DepthStencilDesc, nullptr, &DepthStencilBuffer);
    if (FAILED(hr))
    {
        return false;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc = {};
    DSVDesc.Format = DepthStencilDesc.Format;
    DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    DSVDesc.Texture2D.MipSlice = 0;

    hr = Device->CreateDepthStencilView(DepthStencilBuffer, &DSVDesc, &DepthStencilView);
    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

void GDevice::ReleaseDepthStencilBuffer()
{
    if (DepthStencilBuffer)
    {
        DepthStencilBuffer->Release();
        DepthStencilBuffer = nullptr;
    }
    if (DepthStencilView)
    {
        DepthStencilView->Release();
        DepthStencilView = nullptr;
    }
}

ID3D11Buffer* GDevice::CreateVertexBuffer(FVertexSimple* vertices, UINT byteWidth)
{
    D3D11_BUFFER_DESC vertexbufferdesc = {};
    vertexbufferdesc.ByteWidth = byteWidth;
    vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexbufferSRD = { vertices };

    ID3D11Buffer* vertexBuffer = nullptr;

    HRESULT hr = Device->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer);

    if (FAILED(hr))
    {
        UE_LOG("[GDevice] Failed to create Vertex Buffer. HRESULT: {}\n", hr);
        return nullptr;
    }
    return vertexBuffer;
};

void GDevice::ReleaseVertexBuffer(ID3D11Buffer* vertexBuffer)
{
    if (vertexBuffer)
    {
        vertexBuffer->Release();
    }
}

ID3D11Buffer* GDevice::CreateIndexBuffer(uint32_t* indices, UINT byteWidth)
{
    ID3D11Buffer* indexBuffer = nullptr;

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = byteWidth;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = indices;

    HRESULT hr = Device->CreateBuffer(&bd, &initData, &indexBuffer);
    if (FAILED(hr))
    {
        UE_LOG("[GDevice] Failed to create Index Buffer. HRESULT: {}\n", hr);
        return nullptr;
    }

    return indexBuffer;
}

void GDevice::ReleaseIndexBuffer(ID3D11Buffer* indexBuffer)
{
    if (indexBuffer)
    {
        indexBuffer->Release();
    }
}

void GDevice::SwapBuffer()
{
    SwapChain->Present(1, 0);
}
