#pragma once
#include <vector>

// D3D11 headers
#include <d3d11.h>
#include <d3dcompiler.h>

//#include "UEngine"
#include "Engine/Renderer/FPrimitiveRenderData.h"
#include "GDevice.h"
#include "Core/Math/Matrix.h"
#include "Core/Math/FQuaternion.h"
//#include "../FVertexSimple.h"
#include "Engine/Resource/FMeshResource.h"
#include "Core/Container/TArray.h"
#include "Engine/Resource/FTexture.h"

// debug
#include "Engine/Resource/FFontAtlas.h"

//struct FVertexSimple;
struct FConstants
{
	FMatrix MVP;
};
struct FGridConstants
{
	FVector CameraPos;
	int GridPlaneType;
};
class UScene;
class FEditor;
struct FPrimitiveRenderData;

#include <cmath>


class FRenderer
{
public:
	GDevice* Device = nullptr;
	ID3D11DeviceContext* DeviceContext = nullptr;
	ID3D11Device* D3DDevice = nullptr;

	ID3D11RasterizerState* DefaultRasterizerState = nullptr;
	ID3D11RasterizerState* CullFrontRasterizerState = nullptr;
	ID3D11RasterizerState* CullNoneRasterizerState = nullptr;

	ID3D11DepthStencilState* DefaultDepthStencilState = nullptr;
	ID3D11DepthStencilState* GizmoDepthStencilState = nullptr;
	ID3D11DepthStencilState* HighlightDepthStencilState = nullptr;

	ID3D11BlendState* AlphaBlendState = nullptr;

	ID3D11Buffer* TransformConstantBuffer = nullptr;
	ID3D11Buffer* GridConstantBuffer = nullptr;


	FLOAT                   ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	D3D11_VIEWPORT          ViewportInfo;

	ID3D11VertexShader* SimpleVertexShader = nullptr;
	ID3D11PixelShader* SimplePixelShader = nullptr;
	ID3D11InputLayout* SimpleInputLayout = nullptr;
	ID3D11VertexShader* HighlightVertexShader = nullptr;
	ID3D11PixelShader* HighlightPixelShader = nullptr;
	ID3D11VertexShader* GridVertexShader = nullptr;
	ID3D11PixelShader* GridPixelShader = nullptr;

	ID3D11VertexShader* TextVertexShader = nullptr;
	ID3D11PixelShader* TextPixelShader = nullptr;
	ID3D11InputLayout* TextInputLayout = nullptr;
	FTexture DebugTexture;
	ID3D11SamplerState* PointSampler = nullptr;

	ID3D11Buffer* DebugQuadVB = nullptr;
	ID3D11Buffer* DebugQuadIB = nullptr;
	FFontAtlas* DebugFont=nullptr;
	void RenderDebugTextQuad(const FMatrix& ViewProj);

	bool bImGuiContextCreated = false;
	bool bImGuiWin32Initialized = false;
	bool bImGuiDX11Initialized = false;
	void Create(HWND HWnd, GDevice* InDevice);
	void Shutdown();

	bool CreateShaders();
	bool CompileShader(const WCHAR* FilePath, const LPCSTR EntryPoint, const LPCSTR ShaderModel, ID3DBlob** OutBlob);
	void PrepareRTVDSV();
	void ReleaseShaders();

	ID3D11Buffer* CreateVertexBuffer(FVertexSimple* vertices, UINT byteWidth);
	void ReleaseVertexBuffer(ID3D11Buffer* vertexBuffer);

	void CreateConstantBuffer();
	void UpdateTransformConstantBuffer(const FMatrix& WorldMatrix);
	void UpdateGridConstantBuffer(const FGridConstants& GridConstants);
	void ReleaseConstantBuffer();

	void CreateRasterizerState();
	void ReleaseRasterizerState();

	void CreateAlphaBlendState();
	void ReleaseAlphaBlendState();

	void CreateDepthStencilStates();
	void ReleaseDepthStencilStates();

	void CreateDebugTextResources();
	void ReleaseDebugTextResources();

	void BeginFrame();
	void EndFrame();

	void Render(float DeltaTime, FEditor* Editor, UScene* Scene);
	void RenderPrimitive(const FPrimitiveRenderData& Data);
	void RenderHighlight(const FPrimitiveRenderData& Data);
	void RenderGrid(FMeshResource* Data);
	void RenderGizmo(const FPrimitiveRenderData& Data);
};
