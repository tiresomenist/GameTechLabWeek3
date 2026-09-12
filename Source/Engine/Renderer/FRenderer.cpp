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
	LineBatcher.Release();

	if (DeviceContext) DeviceContext->ClearState();
	ReleaseConstantBuffer();
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

	// Grid Shader (VS & PS)
	if (!CompileShader(L"Assets/Shaders/GridShader.hlsl", "VS_Grid", "vs_5_0", shaderBlob.ReleaseAndGetAddressOf())) return false;
	CheckHR(D3DDevice->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &GridVertexShader));
	shaderBlob.Reset();

	if (!CompileShader(L"Assets/Shaders/GridShader.hlsl", "PS_Grid", "ps_5_0", shaderBlob.ReleaseAndGetAddressOf())) return false;
	CheckHR(D3DDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &GridPixelShader));
	shaderBlob.Reset();

	if (!CompileShader(L"Assets/Shaders/BatchLineShader.hlsl", "mainVS", "vs_5_0", shaderBlob.ReleaseAndGetAddressOf())) return false;
	CheckHR(D3DDevice->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &BatchLineVertexShader));
	shaderBlob.Reset();

	if (!CompileShader(L"Assets/Shaders/BatchLineShader.hlsl", "mainPS", "ps_5_0", shaderBlob.ReleaseAndGetAddressOf())) return false;
	CheckHR(D3DDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &BatchLinePixelShader));
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
	if (BatchLineVertexShader)
	{
		BatchLineVertexShader->Release();
		BatchLineVertexShader = nullptr;
	}
	if (BatchLinePixelShader)
	{
		BatchLinePixelShader->Release();
		BatchLinePixelShader = nullptr;
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
	// 일반 메시: 뒷면 컬링
	D3D11_RASTERIZER_DESC SolidDesc = {};
	SolidDesc.FillMode = D3D11_FILL_SOLID;
	SolidDesc.CullMode = D3D11_CULL_BACK;
	CheckHR(D3DDevice->CreateRasterizerState(&SolidDesc, &DefaultRasterizerState));

	// 그리드: 양면 다 그림
	D3D11_RASTERIZER_DESC CullNoneDesc = SolidDesc;
	CullNoneDesc.CullMode = D3D11_CULL_NONE;
	CheckHR(D3DDevice->CreateRasterizerState(&CullNoneDesc, &CullNoneRasterizerState));

	// 하이라이트 외곽선: 1.05배로 부풀린 껍데기의 뒷면만 그린다 (inverted hull)
	D3D11_RASTERIZER_DESC CullFrontDesc = SolidDesc;
	CullFrontDesc.CullMode = D3D11_CULL_FRONT;
	CheckHR(D3DDevice->CreateRasterizerState(&CullFrontDesc, &CullFrontRasterizerState));

	// 와이어프레임 뷰 모드: 채우기만 끄고 컬링은 일반 메시와 동일
	D3D11_RASTERIZER_DESC WireDesc = SolidDesc;
	WireDesc.FillMode = D3D11_FILL_WIREFRAME;
	CheckHR(D3DDevice->CreateRasterizerState(&WireDesc, &WireframeRasterizerState));
}

void FRenderer::ReleaseRasterizerState()
{
	if (DefaultRasterizerState)
	{
		DefaultRasterizerState->Release();
		DefaultRasterizerState = nullptr;
	}
	if (CullNoneRasterizerState)
	{
		CullNoneRasterizerState->Release();
		CullNoneRasterizerState = nullptr;
	}
	if (CullFrontRasterizerState)
	{
		CullFrontRasterizerState->Release();
		CullFrontRasterizerState = nullptr;
	}
	if (WireframeRasterizerState)
	{
		WireframeRasterizerState->Release();
		WireframeRasterizerState = nullptr;
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

	bWireframe = Editor->IsWireframe();
	BeginFrame();

	UCameraComponent* Camera = Editor->GetEditorCamera();

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
			LineBatcher.AddBoundBox(Item.Min, Item.Max, *Item.WorldMatrix);
		}
		RenderPrimitive(Item);
	}

	// Render Windows
	for (auto Item : Editor->GetWindows())
	{
		Item->Render(DeltaTime);
	}

	// Render Grid
#if 0
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
#else
	LineBatcher.AddGrid(Grid, Camera->GetWorldLocation());
#endif

	// BatchLine
	RenderBatchLine(ViewProjMatrix);

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

	DeviceContext->RSSetState(bWireframe ? WireframeRasterizerState : DefaultRasterizerState);

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

void FRenderer::RenderBatchLine(const FMatrix& ViewProj)
{
	const UINT VertexCount = LineBatcher.GetVertexCount();
	const UINT IndexCount = LineBatcher.GetIndexCount();

	if (VertexCount == 0 || IndexCount == 0)
	{
		return;
	}

	if (!LineBatcher.Build())
	{
		LineBatcher.Clear();
		return;
	}

	UpdateTransformConstantBuffer(ViewProj);

	ID3D11Buffer* VB = LineBatcher.GetVertexBuffer();
	ID3D11Buffer* IB = LineBatcher.GetIndexBuffer();
	UINT Stride = sizeof(FVertexSimple);
	UINT Offset = 0;

	DeviceContext->IASetInputLayout(SimpleInputLayout);
	DeviceContext->IASetVertexBuffers(0, 1, &VB, &Stride, &Offset);
	DeviceContext->IASetIndexBuffer(IB, DXGI_FORMAT_R32_UINT, 0);
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	DeviceContext->VSSetShader(BatchLineVertexShader, nullptr, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, &TransformConstantBuffer);

	DeviceContext->RSSetState(DefaultRasterizerState);

	DeviceContext->PSSetShader(BatchLinePixelShader, nullptr, 0);

	float BlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	UINT SampleMask = 0xffffffff;
	DeviceContext->OMSetBlendState(AlphaBlendState, BlendFactor, SampleMask);

	DeviceContext->OMSetDepthStencilState(DefaultDepthStencilState, 0);

	DeviceContext->DrawIndexed(IndexCount, 0, 0);

	// 나중에 같은 데이터로 여러번 그리려면 Clear() 분리가 필요할 수 있음
	LineBatcher.Clear();
}
