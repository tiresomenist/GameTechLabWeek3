#include "pch.h"
#pragma once
#include "FRenderer.h"
#include "Core/Math/Matrix.h"
#include "Engine/Renderer/FVertexSimple.h"
#include "Editor/Window/UEditorWindow.h"
#include "Engine/Renderer/GDevice.h"
#include "Engine/Scene/UScene.h"
#include "Editor/FEditor.h"
#include "Editor/Gizmo/UGizmo.h"
#include "Engine/Renderer/RenderUtil.h"
#include "Core/Core.h"
#include "Engine/Component/UCameraComponent.h"
#include "Engine/Component/Primitive/UPrimitiveComponent.h"
#include "Engine/Resource/FMeshResource.h"
#include "Engine/Resource/GResourceManager.h"
#include "Engine/Log.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include <format>
#include <filesystem>
#include <wrl/client.h>
#include <stdexcept>
#include <vector>

namespace
{
    void CheckHR(HRESULT Result)
    {
        if (FAILED(Result))
            throw std::runtime_error(std::format("D3D resource creation failed: {}", Result));
    }
}


void FRenderer::Create(HWND HWnd, GDevice* InDevice)
{
    if (!InDevice || !InDevice->GetDevice() || !InDevice->GetContext())
        throw std::runtime_error("Renderer requires an initialized device");
	Device = InDevice;
	DeviceContext = InDevice->GetContext();
	D3DDevice = InDevice->GetDevice();
	ViewportInfo = InDevice->GetViewport();
	CreateRasterizerState(); 
	if (!CreateShaders()) throw std::runtime_error("Shader compilation failed");
	CreateConstantBuffer();
	CreateAlphaBlendState();
	CreateDepthStencilStates();
	CreateTextResources();
	CreateLineResources();

	IMGUI_CHECKVERSION();
	if (!ImGui::CreateContext()) throw std::runtime_error("ImGui context failed");
	bImGuiContextCreated = true;
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("Assets/Fonts/Pretendard-Regular.ttf", 16.0f);

	// Setup Platform/Renderer backends
	bImGuiWin32Initialized = ImGui_ImplWin32_Init(HWnd);
	if (!bImGuiWin32Initialized) throw std::runtime_error("ImGui Win32 initialization failed");
	bImGuiDX11Initialized = ImGui_ImplDX11_Init(D3DDevice, DeviceContext);
	if (!bImGuiDX11Initialized) throw std::runtime_error("ImGui DX11 initialization failed");
    if (!ImGui_ImplDX11_CreateDeviceObjects())
        throw std::runtime_error("ImGui GPU resource creation failed");
}

void FRenderer::Shutdown()
{
    if (DeviceContext) DeviceContext->ClearState();
    ReleaseConstantBuffer();
    ReleaseLineResources();
    ReleaseShaders();
    ReleaseRasterizerState();
    ReleaseAlphaBlendState();
    ReleaseDepthStencilStates();
    ReleaseTextResources();
    if (bImGuiDX11Initialized) ImGui_ImplDX11_Shutdown();
    if (bImGuiWin32Initialized) ImGui_ImplWin32_Shutdown();
    if (bImGuiContextCreated) ImGui::DestroyContext();
    bImGuiDX11Initialized = bImGuiWin32Initialized = bImGuiContextCreated = false;
    DeviceContext = nullptr;
    D3DDevice = nullptr;
    Device = nullptr;
}

bool FRenderer::CreateShaders()
{
	Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;

	// Simple Shader (VS & PS)
	if (!CompileShader(L"Assets/Shaders/MainShader.hlsl", "mainVS", "vs_5_0", shaderBlob.ReleaseAndGetAddressOf())) return false;
	CheckHR(D3DDevice->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &SimpleVertexShader));

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	CheckHR(D3DDevice->CreateInputLayout(layout, ARRAYSIZE(layout), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &SimpleInputLayout));
	shaderBlob.Reset();

	if (!CompileShader(L"Assets/Shaders/MainShader.hlsl", "mainPS", "ps_5_0", shaderBlob.ReleaseAndGetAddressOf())) return false;
	CheckHR(D3DDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &SimplePixelShader));
	shaderBlob.Reset();

	// Highlight Shader (VS & PS)
	if (!CompileShader(L"Assets/Shaders/MainShader.hlsl", "VS_Highlight", "vs_5_0", shaderBlob.ReleaseAndGetAddressOf())) return false;
	CheckHR(D3DDevice->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &HighlightVertexShader));
	shaderBlob.Reset();

	if (!CompileShader(L"Assets/Shaders/MainShader.hlsl", "PS_Highlight", "ps_5_0", shaderBlob.ReleaseAndGetAddressOf())) return false;
	CheckHR(D3DDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &HighlightPixelShader));
	shaderBlob.Reset();

	return true;
}
bool FRenderer::CompileShader(const WCHAR* FilePath, const LPCSTR EntryPoint, const LPCSTR ShaderModel, ID3DBlob** OutBlob)
{
    if (!OutBlob) return false;
    *OutBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> ErrorBlob;
    const HRESULT Hr = D3DCompileFromFile(FilePath, nullptr, nullptr, EntryPoint, ShaderModel,
        0, 0, OutBlob, ErrorBlob.GetAddressOf());
    if (ErrorBlob)
        UE_LOG("Shader diagnostic: {}", static_cast<const char*>(ErrorBlob->GetBufferPointer()));
    return SUCCEEDED(Hr);
}

void FRenderer::ReleaseShaders()
{
	if (SimpleInputLayout)
	{
		SimpleInputLayout->Release();
		SimpleInputLayout = nullptr;
	}
	if (SimplePixelShader)
	{
		SimplePixelShader->Release();
		SimplePixelShader = nullptr;
	}
	if (SimpleVertexShader)
	{
		SimpleVertexShader->Release();
		SimpleVertexShader = nullptr;
	}
	if (HighlightVertexShader)
	{
		HighlightVertexShader->Release();
		HighlightVertexShader = nullptr;
	}
	if (HighlightPixelShader)
	{
		HighlightPixelShader->Release();
		HighlightPixelShader = nullptr;
	}
}

void FRenderer::PrepareRTVDSV()
{
	ViewportInfo = Device->GetViewport(); // 리사이징 된 현재 뷰포트 복사
	DeviceContext->RSSetViewports(1, &ViewportInfo);

	DeviceContext->RSSetState(DefaultRasterizerState);

	ID3D11RenderTargetView* RTV = Device->GetFrameBufferRTV();
	ID3D11DepthStencilView* DSV = Device->GetDepthStencilView();
	DeviceContext->ClearRenderTargetView(RTV, ClearColor);
	DeviceContext->ClearDepthStencilView(DSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	DeviceContext->OMSetRenderTargets(1, &RTV, DSV);

	DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

ID3D11Buffer* FRenderer::CreateVertexBuffer(FVertexSimple* vertices, UINT byteWidth)
{
	D3D11_BUFFER_DESC vertexbufferdesc = {};
	vertexbufferdesc.ByteWidth = byteWidth;
	vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexbufferSRD = { vertices };

	ID3D11Buffer* vertexBuffer = nullptr;

	CheckHR(D3DDevice->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer));

	return vertexBuffer;
}

void FRenderer::ReleaseVertexBuffer(ID3D11Buffer* vertexBuffer)
{
	if (vertexBuffer)
	{
		vertexBuffer->Release();
	}
}

void FRenderer::CreateConstantBuffer()
{
	D3D11_BUFFER_DESC constantbufferdesc = {};
	constantbufferdesc.ByteWidth = (sizeof(FConstants) + 0xf) & 0xfffffff0;
	constantbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
	constantbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantbufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	CheckHR(D3DDevice->CreateBuffer(&constantbufferdesc, nullptr, &TransformConstantBuffer));

}

void FRenderer::ReleaseConstantBuffer()
{
	if (TransformConstantBuffer)
	{
		TransformConstantBuffer->Release();
		TransformConstantBuffer = nullptr;
	}
}

void FRenderer::CreateRasterizerState()
{
	D3D11_RASTERIZER_DESC rasterizerdesc = {};
	rasterizerdesc.FillMode = D3D11_FILL_SOLID;
	rasterizerdesc.CullMode = D3D11_CULL_BACK;  // 백 페이스 컬링
	CheckHR(D3DDevice->CreateRasterizerState(&rasterizerdesc, &DefaultRasterizerState));

	D3D11_RASTERIZER_DESC wireframeDesc = rasterizerdesc;
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	CheckHR(D3DDevice->CreateRasterizerState(&wireframeDesc, &WireframeRasterizerState));

	D3D11_RASTERIZER_DESC rasterizerdescHighlight = {};
	rasterizerdescHighlight.FillMode = D3D11_FILL_SOLID;
	rasterizerdescHighlight.CullMode = D3D11_CULL_NONE;  // 프론트 페이스 컬링
	CheckHR(D3DDevice->CreateRasterizerState(&rasterizerdescHighlight, &CullFrontRasterizerState));

	D3D11_RASTERIZER_DESC rasterizerdescGrid = {};
	rasterizerdescGrid.FillMode = D3D11_FILL_SOLID;
	rasterizerdescGrid.CullMode = D3D11_CULL_NONE;
	CheckHR(D3DDevice->CreateRasterizerState(&rasterizerdescGrid, &CullNoneRasterizerState));
}

void FRenderer::ReleaseRasterizerState()
{
	if (DefaultRasterizerState)
	{
		DefaultRasterizerState->Release();
		DefaultRasterizerState = nullptr;
	}
	if (WireframeRasterizerState)
	{
		WireframeRasterizerState->Release();
		WireframeRasterizerState = nullptr;
	}
	if (CullFrontRasterizerState)
	{
		CullFrontRasterizerState->Release();
		CullFrontRasterizerState = nullptr;
	}
	if (CullNoneRasterizerState)
	{
		CullNoneRasterizerState->Release();
		CullNoneRasterizerState = nullptr;
	}
}

void FRenderer::CreateAlphaBlendState()
{
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;

	// 0번째 렌더 타겟(메인 화면)
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;       // 새로 그릴 픽셀의 알파값 비중
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;  // 이미 그려진 픽셀의 비중 (1 - SrcAlpha)
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;           // 두 색상을 더함

	// 알파 채널 자체를 섞는다
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	CheckHR(D3DDevice->CreateBlendState(&blendDesc, &AlphaBlendState));
}

void FRenderer::ReleaseAlphaBlendState()
{
	if (AlphaBlendState)
	{
		AlphaBlendState->Release();
		AlphaBlendState = nullptr;
	}
}

void FRenderer::CreateDepthStencilStates()
{
	D3D11_DEPTH_STENCIL_DESC DSDesc = {};
	DSDesc.DepthEnable = TRUE;
	DSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DSDesc.DepthFunc = D3D11_COMPARISON_LESS;
	DSDesc.StencilEnable = FALSE;
	CheckHR(D3DDevice->CreateDepthStencilState(&DSDesc, &DefaultDepthStencilState));

	D3D11_DEPTH_STENCIL_DESC GizmoDSDesc = DSDesc;
	GizmoDSDesc.DepthEnable = FALSE;
	CheckHR(D3DDevice->CreateDepthStencilState(&GizmoDSDesc, &GizmoDepthStencilState));

	D3D11_DEPTH_STENCIL_DESC HighlightDesc = {};
	HighlightDesc.DepthEnable = TRUE;  // 깊이 검사는 유지
	HighlightDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // 깊이 기록 안 함
	HighlightDesc.DepthFunc = D3D11_COMPARISON_LESS;

	CheckHR(D3DDevice->CreateDepthStencilState(&HighlightDesc, &HighlightDepthStencilState));
}

void FRenderer::ReleaseDepthStencilStates()
{
	if (DefaultDepthStencilState)
	{
		DefaultDepthStencilState->Release();
		DefaultDepthStencilState = nullptr;
	}
	if (GizmoDepthStencilState)
	{
		GizmoDepthStencilState->Release();
		GizmoDepthStencilState = nullptr;
	}
	if (HighlightDepthStencilState)
	{
		HighlightDepthStencilState->Release();
		HighlightDepthStencilState = nullptr;
	}
}

void FRenderer::CreateLineResources()
{
	Microsoft::WRL::ComPtr<ID3DBlob> ShaderBlob;
	if (!CompileShader(L"Assets/Shaders/ShaderLine.hlsl", "mainVS_Line", "vs_5_0", ShaderBlob.ReleaseAndGetAddressOf()))
		throw std::runtime_error("Line VS compile failed");
	CheckHR(D3DDevice->CreateVertexShader(ShaderBlob->GetBufferPointer(), ShaderBlob->GetBufferSize(), nullptr, &LineVertexShader));

	D3D11_INPUT_ELEMENT_DESC Layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	CheckHR(D3DDevice->CreateInputLayout(Layout, ARRAYSIZE(Layout), ShaderBlob->GetBufferPointer(), ShaderBlob->GetBufferSize(), &LineInputLayout));
	ShaderBlob.Reset();

	if (!CompileShader(L"Assets/Shaders/ShaderLine.hlsl", "mainPS_Line", "ps_5_0", ShaderBlob.ReleaseAndGetAddressOf()))
		throw std::runtime_error("Line PS compile failed");
	CheckHR(D3DDevice->CreatePixelShader(ShaderBlob->GetBufferPointer(), ShaderBlob->GetBufferSize(), nullptr, &LinePixelShader));

	D3D11_BUFFER_DESC VertexBufferDesc = {};
	VertexBufferDesc.ByteWidth = sizeof(FVertexSimple) * MaxLineCount * 2;
	VertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	VertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	CheckHR(D3DDevice->CreateBuffer(&VertexBufferDesc, nullptr, &LineVertexBuffer));

	D3D11_BUFFER_DESC IndexBufferDesc = {};
	IndexBufferDesc.ByteWidth = sizeof(uint32) * MaxLineCount * 2;
	IndexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	IndexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	CheckHR(D3DDevice->CreateBuffer(&IndexBufferDesc, nullptr, &LineIndexBuffer));
}

void FRenderer::ReleaseLineResources()
{
	if (LineVertexBuffer) LineVertexBuffer->Release();
	if (LineIndexBuffer) LineIndexBuffer->Release();
	if (LineInputLayout) LineInputLayout->Release();
	if (LineVertexShader) LineVertexShader->Release();
	if (LinePixelShader) LinePixelShader->Release();
	LineVertexBuffer = nullptr;
	LineIndexBuffer = nullptr;
	LineInputLayout = nullptr;
	LineVertexShader = nullptr;
	LinePixelShader = nullptr;
	LineVertices.Empty();
	LineIndices.Empty();
	CachedGridVertices.Empty();
	CachedGridIndices.Empty();
	bGridCacheValid = false;
}

void FRenderer::BeginLineBatch()
{
	LineVertices.Empty();
	LineIndices.Empty();
	bLineBufferOverflowLogged = false;
}

void FRenderer::AddLine(const FVector& Start, const FVector& End, const FVector4& Color)
{
	if (LineVertices.Num() + 2 > static_cast<int>(MaxLineCount * 2))
	{
		if (!bLineBufferOverflowLogged)
		{
			UE_LOG("[FRenderer] Line batch buffer overflow ({} / {} verts). Dropping line(s).\n", LineVertices.Num(), MaxLineCount * 2);
			bLineBufferOverflowLogged = true;
		}
		return;
	}

	const uint32 StartIndex = static_cast<uint32>(LineVertices.Num());
	LineVertices.Add({ Start.X, Start.Y, Start.Z, Color.X, Color.Y, Color.Z, Color.W });
	LineVertices.Add({ End.X, End.Y, End.Z, Color.X, Color.Y, Color.Z, Color.W });
	LineIndices.Add(StartIndex);
	LineIndices.Add(StartIndex + 1);
}

void FRenderer::RebuildGridCache(const FVector& GridCenter, const FVector& CameraWorldPos)
{
	constexpr float GridSpacing = 1.0f;
	constexpr float FadeStartDistance = 60.0f;
	constexpr float FadeEndDistance = 150.0f;
	constexpr int HalfGridCount = static_cast<int>(FadeEndDistance / GridSpacing) + 5;
	constexpr float GridExtent = HalfGridCount * GridSpacing;
	const FVector4 GridColor(0.3f, 0.3f, 0.3f, 1.0f);

	CachedGridVertices.Empty();
	CachedGridIndices.Empty();

	// 라인 하나는 카메라와의 "수선 거리"(라인이 뻗어나가는 축과 수직인 거리)로 알파를 정함.
	// (끝점까지의 직선 거리로 계산하면 카메라 바로 옆을 지나는 긴 라인도 양쪽 끝점이 멀어서 사라져버림)
	auto FadeAlpha = [](float PerpendicularDist) -> float
	{
		const float T = std::clamp((PerpendicularDist - FadeStartDistance) / (FadeEndDistance - FadeStartDistance), 0.0f, 1.0f);
		return 1.0f - (T * T * (3.0f - 2.0f * T));
	};

	auto AddCachedLine = [this](const FVector& Start, const FVector& End, const FVector4& Color, float Alpha)
	{
		const uint32 StartIndex = static_cast<uint32>(CachedGridVertices.Num());
		const float FinalAlpha = Color.W * Alpha;
		CachedGridVertices.Add({ Start.X, Start.Y, Start.Z, Color.X, Color.Y, Color.Z, FinalAlpha });
		CachedGridVertices.Add({ End.X, End.Y, End.Z, Color.X, Color.Y, Color.Z, FinalAlpha });
		CachedGridIndices.Add(StartIndex);
		CachedGridIndices.Add(StartIndex + 1);
	};

	for (int GridIndex = -HalfGridCount; GridIndex <= HalfGridCount; ++GridIndex)
	{
		const float LineX = GridCenter.X + GridIndex * GridSpacing;
		const float LineY = GridCenter.Y + GridIndex * GridSpacing;
		AddCachedLine(FVector(LineX, GridCenter.Y - GridExtent, 0.0f), FVector(LineX, GridCenter.Y + GridExtent, 0.0f), GridColor, FadeAlpha(std::fabs(LineX - CameraWorldPos.X)));
		AddCachedLine(FVector(GridCenter.X - GridExtent, LineY, 0.0f), FVector(GridCenter.X + GridExtent, LineY, 0.0f), GridColor, FadeAlpha(std::fabs(LineY - CameraWorldPos.Y)));
	}
}

void FRenderer::AddWorldGrid(const FVector& CameraWorldPos)
{
	constexpr float GridSpacing = 1.0f;
	constexpr float AxisLength = 500.0f;

	// 카메라 위치를 그리드 한 칸 단위로 스냅 — 카메라가 같은 칸에 머무는 동안은 캐시를 재사용
	const float SnapCenterX = std::floor(CameraWorldPos.X / GridSpacing) * GridSpacing;
	const float SnapCenterY = std::floor(CameraWorldPos.Y / GridSpacing) * GridSpacing;

	if (!bGridCacheValid || SnapCenterX != CachedGridCenterX || SnapCenterY != CachedGridCenterY)
	{
		CachedGridCenterX = SnapCenterX;
		CachedGridCenterY = SnapCenterY;
		RebuildGridCache(FVector(SnapCenterX, SnapCenterY, 0.0f), CameraWorldPos);
		bGridCacheValid = true;
	}

	if (LineVertices.Num() + CachedGridVertices.Num() > static_cast<int>(MaxLineCount * 2))
	{
		if (!bLineBufferOverflowLogged)
		{
			UE_LOG("[FRenderer] Line batch buffer overflow ({} cached grid verts). Dropping grid.\n", CachedGridVertices.Num());
			bLineBufferOverflowLogged = true;
		}
	}
	else
	{
		const uint32 IndexOffset = static_cast<uint32>(LineVertices.Num());
		for (const FVertexSimple& Vertex : CachedGridVertices) LineVertices.Add(Vertex);
		for (uint32 Index : CachedGridIndices) LineIndices.Add(IndexOffset + Index);
	}

	// 월드 원점 기준 축 표시는 카메라를 따라가지 않고 고정
	const FVector Origin(0.0f, 0.0f, 0.0f);
	AddLine(Origin, FVector(AxisLength, 0.0f, 0.0f), FVector4(1.0f, 0.0f, 0.0f, 1.0f));
	AddLine(Origin, FVector(0.0f, AxisLength, 0.0f), FVector4(0.0f, 1.0f, 0.0f, 1.0f));
	AddLine(Origin, FVector(0.0f, 0.0f, AxisLength), FVector4(0.0f, 0.0f, 1.0f, 1.0f));
}

void FRenderer::AddBoundingBox(const UPrimitiveComponent* Primitive, const UCameraComponent* Camera)
{
	FVector Min;
	FVector Max;
	if (!Primitive || !Primitive->GetLocalBounds(Min, Max)) return;

	const FMatrix& WorldMatrix = Primitive->GetRenderWorldMatrix(Camera);
	const FVector Corners[8] =
	{
		WorldMatrix.TransformPosition(FVector(Min.X, Min.Y, Min.Z)),
		WorldMatrix.TransformPosition(FVector(Max.X, Min.Y, Min.Z)),
		WorldMatrix.TransformPosition(FVector(Max.X, Max.Y, Min.Z)),
		WorldMatrix.TransformPosition(FVector(Min.X, Max.Y, Min.Z)),
		WorldMatrix.TransformPosition(FVector(Min.X, Min.Y, Max.Z)),
		WorldMatrix.TransformPosition(FVector(Max.X, Min.Y, Max.Z)),
		WorldMatrix.TransformPosition(FVector(Max.X, Max.Y, Max.Z)),
		WorldMatrix.TransformPosition(FVector(Min.X, Max.Y, Max.Z)),
	};
	static constexpr uint32 Edges[12][2] =
	{
		{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },
		{ 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 },
		{ 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 },
	};
	const FVector4 BoxColor(1.0f, 1.0f, 0.0f, 1.0f);
	for (const auto& Edge : Edges)
	{
		AddLine(Corners[Edge[0]], Corners[Edge[1]], BoxColor);
	}
}

void FRenderer::RenderLineBatch(const FMatrix& ViewProj)
{
	if (LineIndices.IsEmpty()) return;

	D3D11_MAPPED_SUBRESOURCE MappedResource = {};
	if (FAILED(DeviceContext->Map(LineVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource))) return;
	memcpy(MappedResource.pData, LineVertices.GetData(), sizeof(FVertexSimple) * LineVertices.Num());
	DeviceContext->Unmap(LineVertexBuffer, 0);

	if (FAILED(DeviceContext->Map(LineIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource))) return;
	memcpy(MappedResource.pData, LineIndices.GetData(), sizeof(uint32) * LineIndices.Num());
	DeviceContext->Unmap(LineIndexBuffer, 0);

	UpdateTransformConstantBuffer(ViewProj);
	const UINT Stride = sizeof(FVertexSimple);
	const UINT Offset = 0;
	DeviceContext->IASetInputLayout(LineInputLayout);
	DeviceContext->IASetVertexBuffers(0, 1, &LineVertexBuffer, &Stride, &Offset);
	DeviceContext->IASetIndexBuffer(LineIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	DeviceContext->VSSetShader(LineVertexShader, nullptr, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, &TransformConstantBuffer);
	DeviceContext->PSSetShader(LinePixelShader, nullptr, 0);
	DeviceContext->RSSetState(CullNoneRasterizerState);
	DeviceContext->OMSetBlendState(AlphaBlendState, nullptr, 0xffffffff); // 그리드 페이드아웃을 위해 알파 블렌딩 사용 (알파 1인 라인은 그대로 불투명하게 보임)
	DeviceContext->OMSetDepthStencilState(DefaultDepthStencilState, 0);
	DeviceContext->DrawIndexed(static_cast<UINT>(LineIndices.Num()), 0, 0);
}

void FRenderer::CreateTextResources()
{
	// 텍스트 셰이더
	Microsoft::WRL::ComPtr<ID3DBlob> ShaderBlob;
	if (!CompileShader(L"Assets/Shaders/TextShader.hlsl", "mainVS_Text", "vs_5_0", ShaderBlob.ReleaseAndGetAddressOf()))
		throw std::runtime_error("Text VS compile failed");
	CheckHR(D3DDevice->CreateVertexShader(ShaderBlob->GetBufferPointer(), ShaderBlob->GetBufferSize(), nullptr, &TextVertexShader));

	D3D11_INPUT_ELEMENT_DESC Layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	CheckHR(D3DDevice->CreateInputLayout(Layout, ARRAYSIZE(Layout), ShaderBlob->GetBufferPointer(), ShaderBlob->GetBufferSize(), &TextInputLayout));
	ShaderBlob.Reset();

	if (!CompileShader(L"Assets/Shaders/TextShader.hlsl", "mainPS_Text", "ps_5_0", ShaderBlob.ReleaseAndGetAddressOf()))
		throw std::runtime_error("Text PS compile failed");
	CheckHR(D3DDevice->CreatePixelShader(ShaderBlob->GetBufferPointer(), ShaderBlob->GetBufferSize(), nullptr, &TextPixelShader));

	// 샘플러 (기존 프로젝트에 샘플러가 하나도 없어서 신규 생성)
	D3D11_SAMPLER_DESC SamplerDesc = {};
	SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	CheckHR(D3DDevice->CreateSamplerState(&SamplerDesc, &FontSamplerState));

	// 깊이 검사는 하되(오브젝트에 가려지게) 기록은 안 함(라벨끼리 겹칠 때 z-fight 방지)
	D3D11_DEPTH_STENCIL_DESC DSDesc = {};
	DSDesc.DepthEnable = TRUE;
	DSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	DSDesc.DepthFunc = D3D11_COMPARISON_LESS;
	CheckHR(D3DDevice->CreateDepthStencilState(&DSDesc, &TextDepthStencilState));

	// Vertex Buffer: 매 프레임 내용이 바뀌므로 DYNAMIC, 고정 용량으로 1회만 생성
	D3D11_BUFFER_DESC VBDesc = {};
	VBDesc.ByteWidth = sizeof(FVertexText) * MaxTextVertices;
	VBDesc.Usage = D3D11_USAGE_DYNAMIC;
	VBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	CheckHR(D3DDevice->CreateBuffer(&VBDesc, nullptr, &TextVertexBuffer));

	// Index Buffer: quad 패턴(0,1,2,0,2,3)이 항상 동일하므로 1회 IMMUTABLE 생성
	const UINT MaxQuads = MaxTextVertices / 4;
	std::vector<uint32> Indices;
	Indices.reserve(MaxQuads * 6);
	for (UINT q = 0; q < MaxQuads; ++q)
	{
		const uint32 Base = q * 4;
		Indices.push_back(Base + 0);
		Indices.push_back(Base + 1);
		Indices.push_back(Base + 2);
		Indices.push_back(Base + 0);
		Indices.push_back(Base + 2);
		Indices.push_back(Base + 3);
	}

	D3D11_BUFFER_DESC IBDesc = {};
	IBDesc.ByteWidth = static_cast<UINT>(sizeof(uint32) * Indices.size());
	IBDesc.Usage = D3D11_USAGE_IMMUTABLE;
	IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA IBData = { Indices.data() };
	CheckHR(D3DDevice->CreateBuffer(&IBDesc, &IBData, &TextIndexBuffer));
}

void FRenderer::ReleaseTextResources()
{
	if (TextVertexBuffer) { TextVertexBuffer->Release(); TextVertexBuffer = nullptr; }
	if (TextIndexBuffer) { TextIndexBuffer->Release(); TextIndexBuffer = nullptr; }
	if (TextDepthStencilState) { TextDepthStencilState->Release(); TextDepthStencilState = nullptr; }
	if (FontSamplerState) { FontSamplerState->Release(); FontSamplerState = nullptr; }
	if (TextInputLayout) { TextInputLayout->Release(); TextInputLayout = nullptr; }
	if (TextPixelShader) { TextPixelShader->Release(); TextPixelShader = nullptr; }
	if (TextVertexShader) { TextVertexShader->Release(); TextVertexShader = nullptr; }
}

void FRenderer::UpdateTextVertexBuffer(TArray<FVertexText>& Vertices)
{
	if (Vertices.Num() == 0) return;

	const UINT CopyCount = (static_cast<UINT>(Vertices.Num()) < MaxTextVertices) ? static_cast<UINT>(Vertices.Num()) : MaxTextVertices;

	D3D11_MAPPED_SUBRESOURCE Mapped;
	DeviceContext->Map(TextVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped);
	memcpy(Mapped.pData, Vertices.GetData(), sizeof(FVertexText) * CopyCount);
	DeviceContext->Unmap(TextVertexBuffer, 0);
}

void FRenderer::RenderText(UINT IndexCount)
{
	if (IndexCount == 0) return;

	UINT Stride = sizeof(FVertexText);
	UINT Offset = 0;
	DeviceContext->IASetInputLayout(TextInputLayout);
	DeviceContext->IASetVertexBuffers(0, 1, &TextVertexBuffer, &Stride, &Offset);
	DeviceContext->IASetIndexBuffer(TextIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DeviceContext->VSSetShader(TextVertexShader, nullptr, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, &TransformConstantBuffer);

	DeviceContext->PSSetShader(TextPixelShader, nullptr, 0);
	FFontAtlas* FontAtlas = GResourceManager::GetInstance()->GetDefaultFont();
	if (!FontAtlas) return;
	ID3D11ShaderResourceView* SRV = FontAtlas->GetSRV();
	DeviceContext->PSSetShaderResources(0, 1, &SRV);
	DeviceContext->PSSetSamplers(0, 1, &FontSamplerState);

	DeviceContext->RSSetState(CullNoneRasterizerState);
	float BlendFactor[4] = { 0, 0, 0, 0 };
	DeviceContext->OMSetBlendState(AlphaBlendState, BlendFactor, 0xffffffff);
	DeviceContext->OMSetDepthStencilState(TextDepthStencilState, 0);

	DeviceContext->DrawIndexed(IndexCount, 0, 0);
}

void FRenderer::BeginFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	PrepareRTVDSV();
}

void FRenderer::EndFrame()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	GDevice::GetInstance()->SwapBuffer();

	ID3D11RenderTargetView* nullRTV = nullptr;
	DeviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);
}

void FRenderer::Render(float DeltaTime, FEditor* Editor, UScene* Scene)
{
    if (!Device || !Device->IsRenderReady() || !Editor || !Scene) return;
	BeginFrame();

	UCameraComponent* Camera = Editor->GetEditorCamera();
	bWireframeMode = Editor->GetViewportRenderMode() == EViewportRenderMode::Wireframe;

	Camera->SetAspectRatio(Device->GetViewport().Width / Device->GetViewport().Height);
	FMatrix ViewProjMatrix = Camera->GetViewMatrix() * Camera->GetProjectionMatrix();
	TArray<FPrimitiveRenderData> RenderList = RenderUtil::GetRenderList(Editor, Scene);

	for (auto& Item : RenderList)
	{
		if (!Item.WorldMatrix || !Item.VertexBuffer || !Item.IndexBuffer || Item.IndexCount == 0)
		{
			continue;
		}
		FMatrix MVP = (*Item.WorldMatrix) * ViewProjMatrix;
		UpdateTransformConstantBuffer(MVP);
		if (Item.isSelected)
		{
			RenderHighlight(Item);
		}
		RenderPrimitive(Item);
	}

	// Render Windows
	for (auto Item : Editor->GetWindows())
	{
		Item->Render(DeltaTime);
	}

	BeginLineBatch();
	AddWorldGrid(Camera->GetWorldLocation());
	USceneComponent* SelectedComponent = Editor->GetSelectedSceneComponent();
	if (SelectedComponent && SelectedComponent->IsA(UPrimitiveComponent::GetClass()))
	{
		AddBoundingBox(static_cast<UPrimitiveComponent*>(SelectedComponent), Camera);
	}
	RenderLineBatch(ViewProjMatrix);

	// Render Gizmo
	TArray<FPrimitiveRenderData> GizmoRenderList = RenderUtil::GetGizmoList(Editor, Scene);
	for (auto Item : GizmoRenderList)
	{
		FMatrix MVP = (*Item.WorldMatrix) * ViewProjMatrix;
		UpdateTransformConstantBuffer(MVP);
		if (Item.isSelected)
		{
			RenderHighlight(Item);
		}
		RenderGizmo(Item);
	}

	FFontAtlas* FontAtlas = GResourceManager::GetInstance()->GetDefaultFont();
	if (FontAtlas)
	{
		TArray<FWorldTextItem> TextItems = RenderUtil::GetTextRenderList(Scene, Camera, Editor->IsShowingUUIDLabels());
		TArray<FVertexText> TextVerts = FTextMeshBuilder::Build(TextItems, *FontAtlas);
	UpdateTextVertexBuffer(TextVerts);
	UpdateTransformConstantBuffer(ViewProjMatrix); // 텍스트는 이미 월드공간이라 World=Identity
	const UINT TextVertexCount = (static_cast<UINT>(TextVerts.Num()) < MaxTextVertices) ? static_cast<UINT>(TextVerts.Num()) : MaxTextVertices;
	RenderText(TextVertexCount / 4 * 6);
	}

	UpdateTransformConstantBuffer(ViewProjMatrix);
	EndFrame();
}

void FRenderer::UpdateTransformConstantBuffer(const FMatrix& MVP)
{
	if (!DeviceContext || !TransformConstantBuffer)
	{
		return;
	}
	D3D11_MAPPED_SUBRESOURCE constantbufferMSR{};

	HRESULT hr = DeviceContext->Map(TransformConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
	if (SUCCEEDED(hr))
	{
		FConstants* constants = (FConstants*)constantbufferMSR.pData;
		if (constants)
		{
			constants->MVP = MVP;
		}
		DeviceContext->Unmap(TransformConstantBuffer, 0);
	}
	else
	{
		UE_LOG("[FRenderer] Failed to Map TransformConstantBuffer. HRESULT: {}\n", hr);
	}
}

void FRenderer::RenderPrimitive(const FPrimitiveRenderData& Data)
{
	UINT Offset = 0;
	DeviceContext->IASetInputLayout(SimpleInputLayout);
	DeviceContext->IASetVertexBuffers(0, 1, &Data.VertexBuffer, &Data.Stride, &Offset);
	DeviceContext->IASetIndexBuffer(Data.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	DeviceContext->IASetPrimitiveTopology(Data.Topology);

	DeviceContext->VSSetShader(SimpleVertexShader, nullptr, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, &TransformConstantBuffer);

	DeviceContext->RSSetState(bWireframeMode ? WireframeRasterizerState : DefaultRasterizerState);

	DeviceContext->PSSetShader(SimplePixelShader, nullptr, 0);

	DeviceContext->OMSetDepthStencilState(DefaultDepthStencilState, 0);
	// BindMaterial(Data.Material); 

	DeviceContext->DrawIndexed(Data.IndexCount, 0, 0);
}

void FRenderer::RenderHighlight(const FPrimitiveRenderData& Data)
{
	UINT Offset = 0;
	DeviceContext->IASetInputLayout(SimpleInputLayout);
	DeviceContext->IASetVertexBuffers(0, 1, &Data.VertexBuffer, &Data.Stride, &Offset);
	DeviceContext->IASetIndexBuffer(Data.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	DeviceContext->IASetPrimitiveTopology(Data.Topology);

	DeviceContext->VSSetShader(HighlightVertexShader, nullptr, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, &TransformConstantBuffer);

	DeviceContext->RSSetState(bWireframeMode ? WireframeRasterizerState : CullFrontRasterizerState);

	DeviceContext->PSSetShader(HighlightPixelShader, nullptr, 0);

	DeviceContext->OMSetDepthStencilState(HighlightDepthStencilState, 0);

	DeviceContext->DrawIndexed(Data.IndexCount, 0, 0);
}

void FRenderer::RenderGizmo(const FPrimitiveRenderData& Data)
{
	UINT Offset = 0;
	DeviceContext->IASetInputLayout(SimpleInputLayout);
	DeviceContext->IASetVertexBuffers(0, 1, &Data.VertexBuffer, &Data.Stride, &Offset);
	DeviceContext->IASetIndexBuffer(Data.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	DeviceContext->IASetPrimitiveTopology(Data.Topology);

	DeviceContext->VSSetShader(SimpleVertexShader, nullptr, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, &TransformConstantBuffer);

	DeviceContext->RSSetState(DefaultRasterizerState);

	DeviceContext->PSSetShader(SimplePixelShader, nullptr, 0);

	DeviceContext->OMSetDepthStencilState(GizmoDepthStencilState, 0);
	// BindMaterial(Data.Material); 

	DeviceContext->DrawIndexed(Data.IndexCount, 0, 0);
}
