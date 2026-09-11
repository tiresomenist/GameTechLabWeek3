#include "pch.h"
#pragma once
#include "FRenderer.h"
#include "Core/Math/Matrix.h"
#include "Engine/Renderer/FVertexSimple.h"
#include "Editor/Window/UEditorWindow.h"
#include "Engine/Renderer/GDevice.h"
#include "Engine/Scene/UScene.h"
#include "Editor/FEditor.h"
#include "Editor/UGrid.h"
#include "Editor/Gizmo/UGizmo.h"
#include "Engine/Renderer/RenderUtil.h"
#include "Core/Core.h"
#include "Engine/Component/UCameraComponent.h"
#include "Engine/Resource/FMeshResource.h"
#include "Engine/Log.h"
#include "Engine/Renderer/FVertexText.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include <format>
#include <filesystem>
#include <wrl/client.h>
#include <stdexcept>

namespace
{
	void CheckHR(HRESULT Result)
	{
		if (FAILED(Result))
			throw std::runtime_error(std::format("D3D resource creation failed: {}", Result));
	}
}


void FRenderer::RenderDebugTextQuad(const FMatrix& ViewProj)
{
	UpdateTransformConstantBuffer(ViewProj);           // 월드 행렬 = 단위 행렬 → MVP = ViewProj

	UINT Stride = sizeof(FVertexText);
	UINT Offset = 0;
	DeviceContext->IASetInputLayout(TextInputLayout);
	DeviceContext->IASetVertexBuffers(0, 1, &DebugQuadVB, &Stride, &Offset);
	DeviceContext->IASetIndexBuffer(DebugQuadIB, DXGI_FORMAT_R32_UINT, 0);
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DeviceContext->VSSetShader(TextVertexShader, nullptr, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, &TransformConstantBuffer);

	ID3D11ShaderResourceView* SRV = DebugTexture.GetSRV();
	DeviceContext->PSSetShader(TextPixelShader, nullptr, 0);
	DeviceContext->PSSetShaderResources(0, 1, &SRV);   // t0
	DeviceContext->PSSetSamplers(0, 1, &PointSampler); // s0

	DeviceContext->RSSetState(CullNoneRasterizerState); // 앞뒤 어느 쪽에서 봐도 보이게
	DeviceContext->DrawIndexed(6, 0, 0);
	DeviceContext->RSSetState(DefaultRasterizerState);  // 다음 드로우를 위해 되돌리기
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
	CreateDebugTextResources();

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
	ReleaseShaders();
	ReleaseRasterizerState();
	ReleaseAlphaBlendState();
	ReleaseDepthStencilStates();
	ReleaseDebugTextResources();

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

	// Grid Shader (VS & PS)
	if (!CompileShader(L"Assets/Shaders/GridShader.hlsl", "VS_Grid", "vs_5_0", shaderBlob.ReleaseAndGetAddressOf())) return false;
	CheckHR(D3DDevice->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &GridVertexShader));
	shaderBlob.Reset();

	if (!CompileShader(L"Assets/Shaders/GridShader.hlsl", "PS_Grid", "ps_5_0", shaderBlob.ReleaseAndGetAddressOf())) return false;
	CheckHR(D3DDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &GridPixelShader));
	shaderBlob.Reset();

	// Text Shader (VS & PS)  ← 여기부터 추가
	if (!CompileShader(L"Assets/Shaders/TextShader.hlsl", "mainVS", "vs_5_0", shaderBlob.ReleaseAndGetAddressOf())) return false;
	CheckHR(D3DDevice->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &TextVertexShader));

	D3D11_INPUT_ELEMENT_DESC TextLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	CheckHR(D3DDevice->CreateInputLayout(TextLayout, ARRAYSIZE(TextLayout), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &TextInputLayout));
	shaderBlob.Reset();

	if (!CompileShader(L"Assets/Shaders/TextShader.hlsl", "mainPS", "ps_5_0", shaderBlob.ReleaseAndGetAddressOf())) return false;
	CheckHR(D3DDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &TextPixelShader));
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
	if (GridVertexShader)
	{
		GridVertexShader->Release();
		GridVertexShader = nullptr;
	}
	if (GridPixelShader)
	{
		GridPixelShader->Release();
		GridPixelShader = nullptr;
	}
	if (TextVertexShader) { TextVertexShader->Release(); TextVertexShader = nullptr; }
	if (TextPixelShader) { TextPixelShader->Release();  TextPixelShader = nullptr; }
	if (TextInputLayout) { TextInputLayout->Release();  TextInputLayout = nullptr; }
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

	D3D11_BUFFER_DESC gridconstantbufferdesc = {};
	gridconstantbufferdesc.ByteWidth = (sizeof(FGridConstants) + 0xf) & 0xfffffff0;
	gridconstantbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
	gridconstantbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	gridconstantbufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	CheckHR(D3DDevice->CreateBuffer(&gridconstantbufferdesc, nullptr, &GridConstantBuffer));
}

void FRenderer::ReleaseConstantBuffer()
{
	if (TransformConstantBuffer)
	{
		TransformConstantBuffer->Release();
		TransformConstantBuffer = nullptr;
	}
	if (GridConstantBuffer)
	{
		GridConstantBuffer->Release();
		GridConstantBuffer = nullptr;
	}
}

void FRenderer::CreateRasterizerState()
{
	D3D11_RASTERIZER_DESC rasterizerdesc = {};
	rasterizerdesc.FillMode = D3D11_FILL_SOLID;
	rasterizerdesc.CullMode = D3D11_CULL_BACK;  // 백 페이스 컬링
	CheckHR(D3DDevice->CreateRasterizerState(&rasterizerdesc, &DefaultRasterizerState));

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

void FRenderer::CreateDebugTextResources()
{
	constexpr UINT Size = 8;
	uint32_t Pixels[Size * Size];

	// R8G8B8A8_UNORM은 메모리에 R,G,B,A 순서 → uint32로는 0xAABBGGRR
	constexpr uint32_t White = 0xFFFFFFFF;
	constexpr uint32_t Gray = 0xFF404040;
	constexpr uint32_t Red = 0xFF0000FF;

	for (UINT y = 0; y < Size; ++y)
	{
		for (UINT x = 0; x < Size; ++x)
		{
			// (x + y)가 짝수/홀수냐로 번갈아 칠한다
			Pixels[y * Size + x] = ((x + y) % 2 == 0) ? White : Gray;
		}
	}
	Pixels[0] = Red;   // 좌상단 (0,0)만 빨강 → 뒤집힘 확인용

	if (!DebugTexture.Create(D3DDevice, Size, Size, DXGI_FORMAT_R8G8B8A8_UNORM, 4, Pixels))
		throw std::runtime_error("Debug texture creation failed");

	D3D11_SAMPLER_DESC SamplerDesc = {};
	SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;   // 가장 가까운 텍셀 하나만 → 칸 경계가 칼같이
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;      // UV가 0~1 밖이면 가장자리 색 유지
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	CheckHR(D3DDevice->CreateSamplerState(&SamplerDesc, &PointSampler));

	UE_LOG("[FRenderer] Debug texture OK: {}x{}", DebugTexture.GetWidth(), DebugTexture.GetHeight());

	// 그리드가 XY 평면(Z가 위)이라 X = 0인 YZ 평면에 세운다. 한 변 2 유닛
//   0 ── 1      UV (0,0) ── (1,0)
//   │ ╲  │         │          │
//   2 ── 3      UV (0,1) ── (1,1)
	FVertexText QuadVertices[] =
	{
		{ 0.f, -1.f,  1.f,   0.f, 0.f,   1.f, 1.f, 1.f, 1.f },
		{ 0.f,  1.f,  1.f,   1.f, 0.f,   1.f, 1.f, 1.f, 1.f },
		{ 0.f, -1.f, -1.f,   0.f, 1.f,   1.f, 1.f, 1.f, 1.f },
		{ 0.f,  1.f, -1.f,   1.f, 1.f,   1.f, 1.f, 1.f, 1.f },
	};
	uint32_t QuadIndices[] = { 0, 1, 2,   2, 1, 3 };

	D3D11_BUFFER_DESC VBDesc = {};
	VBDesc.ByteWidth = sizeof(QuadVertices);
	VBDesc.Usage = D3D11_USAGE_IMMUTABLE;
	VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA VBData = {};
	VBData.pSysMem = QuadVertices;
	CheckHR(D3DDevice->CreateBuffer(&VBDesc, &VBData, &DebugQuadVB));

	// 인덱스 버퍼도 같은 방식 (BindFlags = D3D11_BIND_INDEX_BUFFER) — 직접 써 보세요
	D3D11_BUFFER_DESC IBDesc = {};
	IBDesc.ByteWidth = sizeof(QuadIndices);
	IBDesc.Usage = D3D11_USAGE_IMMUTABLE;
	IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA IBData = {};
	IBData.pSysMem = QuadIndices;
	CheckHR(D3DDevice->CreateBuffer(&IBDesc, &IBData, &DebugQuadIB));
}

void FRenderer::ReleaseDebugTextResources()
{
	DebugTexture.Release();
	if (PointSampler) { PointSampler->Release(); PointSampler = nullptr; }
	if (DebugQuadVB)
	{
		DebugQuadVB->Release();
		DebugQuadVB = nullptr;
	}
	if (DebugQuadIB)
	{
		DebugQuadIB->Release();
		DebugQuadIB = nullptr;
	}
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

	Camera->SetAspectRatio(Device->GetViewport().Width / Device->GetViewport().Height);
	FMatrix ViewProjMatrix = Camera->GetViewMatrix() * Camera->GetProjectionMatrix();
	TArray<FPrimitiveRenderData> RenderList = RenderUtil::GetRenderList(Editor, Scene);

	for (auto& Item : RenderList)
	{
		FMatrix MVP = (*Item.WorldMatrix) * ViewProjMatrix;
		UpdateTransformConstantBuffer(MVP);
		if (Item.isSelected)
		{
			RenderHighlight(Item);
		}
		RenderPrimitive(Item);
	}

	// 디버그
	RenderDebugTextQuad(ViewProjMatrix);

	// Render Windows
	for (auto Item : Editor->GetWindows())
	{
		Item->Render(DeltaTime);
	}

	// Render Grid
	UpdateTransformConstantBuffer(ViewProjMatrix);
	for (auto Item : Editor->GetGrids())
	{
		// XY 평면용 월드 행렬 세팅 및 렌더링
		FMatrix XY_WorldMatrix = FMatrix::Identity;

		FVector WorldCameraPos = Camera->GetWorldLocation();
		FVector LocalCameraPosXY = XY_WorldMatrix.Inverse().TransformPosition(WorldCameraPos);

		FGridConstants ConstantsXY;
		ConstantsXY.CameraPos = LocalCameraPosXY;
		ConstantsXY.GridPlaneType = 0;
		UpdateGridConstantBuffer(ConstantsXY);

		UpdateTransformConstantBuffer(XY_WorldMatrix * ViewProjMatrix);
		RenderGrid(Item->GetMeshResource());

		// 기존 판을 Y축 기준으로 90도(PI/2) 회전 (YZ 평면)
		float theta = atan2f(Camera->GetWorldLocation().Y, Camera->GetWorldLocation().X);
		FMatrix Z_WorldMatrix =
			FMatrix::MakeScaleMatrix(FVector(5.0f, 1.0f, 1.0f)) *
			FMatrix::MakeRotationYMatrix(PI / 2.0f) *
			FMatrix::MakeRotationZMatrix(theta);

		// 카메라의 월드 위치를 Z평면의 로컬 공간(Local Space)으로 변환
		//FVector WorldCameraPos = Camera->GetWorldLocation();
		FVector LocalCameraPosZ = Z_WorldMatrix.Inverse().TransformPosition(WorldCameraPos);

		// 셰이더 상수 버퍼에 '로컬 카메라 위치'를 전달
		FGridConstants ConstantsZ;
		ConstantsZ.CameraPos = LocalCameraPosZ;
		ConstantsZ.GridPlaneType = 1;
		UpdateGridConstantBuffer(ConstantsZ);

		UpdateTransformConstantBuffer(Z_WorldMatrix * ViewProjMatrix);
		RenderGrid(Item->GetMeshResource());
	}

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

void FRenderer::UpdateGridConstantBuffer(const FGridConstants& GridConstants)
{
	if (!DeviceContext || !GridConstantBuffer)
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE constantbufferMSR{};

	HRESULT hr = DeviceContext->Map(GridConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
	if (SUCCEEDED(hr))
	{
		FGridConstants* constants = (FGridConstants*)constantbufferMSR.pData;
		if (constants)
		{
			constants->CameraPos = GridConstants.CameraPos;
			constants->GridPlaneType = GridConstants.GridPlaneType;
		}
		DeviceContext->Unmap(GridConstantBuffer, 0);
	}
	else
	{
		UE_LOG("[FRenderer] Failed to Map GridConstantBuffer. HRESULT: {}\n", hr);
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

	DeviceContext->RSSetState(DefaultRasterizerState);

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

	DeviceContext->RSSetState(CullFrontRasterizerState);

	DeviceContext->PSSetShader(HighlightPixelShader, nullptr, 0);

	DeviceContext->OMSetDepthStencilState(HighlightDepthStencilState, 0);

	DeviceContext->DrawIndexed(Data.IndexCount, 0, 0);
}

void FRenderer::RenderGrid(FMeshResource* Data)
{
	if (!Data) return;
	ID3D11Buffer* VertexBuffer = Data->GetVertexBuffer();
	const UINT Stride = Data->GetStride();
	UINT Offset = 0;
	DeviceContext->IASetInputLayout(SimpleInputLayout);
	DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);
	DeviceContext->IASetIndexBuffer(Data->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DeviceContext->VSSetShader(GridVertexShader, nullptr, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, &TransformConstantBuffer);
	DeviceContext->VSSetConstantBuffers(1, 1, &GridConstantBuffer);

	DeviceContext->RSSetState(CullNoneRasterizerState);

	DeviceContext->PSSetShader(GridPixelShader, nullptr, 0);
	DeviceContext->PSSetConstantBuffers(1, 1, &GridConstantBuffer);

	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	UINT sampleMask = 0xffffffff;
	DeviceContext->OMSetBlendState(AlphaBlendState, blendFactor, sampleMask);
	DeviceContext->OMSetDepthStencilState(DefaultDepthStencilState, 0);

	DeviceContext->DrawIndexed(Data->GetIndexCount(), 0, 0);
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
