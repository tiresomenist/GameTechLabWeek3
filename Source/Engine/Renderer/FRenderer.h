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
#include "Engine/Renderer/Text/FFontAtlas.h"
#include "Engine/Renderer/Text/FVertexText.h"
#include "Engine/Renderer/Text/FTextMeshBuilder.h"
#include "Engine/Renderer/FLineBatcher.h"


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
enum class EViewModeIndex : uint32
{
	VMI_Lit,
	VMI_Unlit,
	VMI_Wireframe,
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
	ID3D11RasterizerState* WireframeRasterizerState = nullptr;

	ID3D11DepthStencilState* DefaultDepthStencilState = nullptr;
	ID3D11DepthStencilState* GizmoDepthStencilState = nullptr;
	ID3D11DepthStencilState* HighlightDepthStencilState = nullptr;

	ID3D11BlendState* AlphaBlendState = nullptr;

	ID3D11Buffer* TransformConstantBuffer = nullptr;
	ID3D11Buffer* GridConstantBuffer = nullptr;

	FLineBatcher* LineBatcher = nullptr;

	FLOAT                   ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	D3D11_VIEWPORT          ViewportInfo;

	ID3D11VertexShader* SimpleVertexShader = nullptr;
	ID3D11PixelShader* SimplePixelShader = nullptr;
	ID3D11InputLayout* SimpleInputLayout = nullptr;
	ID3D11VertexShader* HighlightVertexShader = nullptr;
	ID3D11PixelShader* HighlightPixelShader = nullptr;
	ID3D11VertexShader* GridVertexShader = nullptr;
	ID3D11PixelShader* GridPixelShader = nullptr;

	// ---- Text Billboard ----
	ID3D11VertexShader* TextVertexShader = nullptr;
	ID3D11PixelShader* TextPixelShader = nullptr;
	ID3D11InputLayout* TextInputLayout = nullptr;

	ID3D11SamplerState* FontSamplerState = nullptr;
	ID3D11DepthStencilState* TextDepthStencilState = nullptr;

	ID3D11Buffer* TextVertexBuffer = nullptr;
	ID3D11Buffer* TextIndexBuffer = nullptr;
	static const UINT MaxTextVertices = 8192;
	
	EViewModeIndex CurrentViewMode = EViewModeIndex::VMI_Lit;
	void SetViewMode(EViewModeIndex ViewMode) { CurrentViewMode = ViewMode; }
	EViewModeIndex GetViewMode() { return CurrentViewMode; }

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

	void BeginFrame();
	void EndFrame();

	void Render(float DeltaTime, FEditor* Editor, UScene* Scene);
	void RenderPrimitive(const FPrimitiveRenderData& Data);
	void RenderHighlight(const FPrimitiveRenderData& Data);
	void RenderGrid(FMeshResource* Data);
	void RenderGizmo(const FPrimitiveRenderData& Data);
	void RenderLines(FLineBatcher* Batcher);

	void CreateTextResources();
	void ReleaseTextResources();
	void UpdateTextVertexBuffer(TArray<FVertexText>& Vertices);
	void RenderText(UINT IndexCount);
};
