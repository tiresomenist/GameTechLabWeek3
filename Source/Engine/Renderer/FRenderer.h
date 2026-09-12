#pragma once
#include <vector>

// D3D11 headers
#include <d3d11.h>
#include <d3dcompiler.h>

//#include "UEngine"
#include "Engine/Renderer/FPrimitiveRenderData.h"
#include "Engine/Renderer/FVertexSimple.h"
#include "GDevice.h"
#include "Core/Math/Matrix.h"
#include "Core/Math/FQuaternion.h"
//#include "../FVertexSimple.h"
#include "Engine/Resource/FMeshResource.h"
#include "Core/Container/TArray.h"
#include "Engine/Renderer/Text/FFontAtlas.h"
#include "Engine/Renderer/Text/FVertexText.h"
#include "Engine/Renderer/Text/FTextMeshBuilder.h"


struct FConstants
{
	FMatrix MVP;
};
class UScene;
class FEditor;
class UCameraComponent;
class UPrimitiveComponent;
struct FPrimitiveRenderData;

#include <cmath>


class FRenderer
{
public:
	GDevice* Device = nullptr;
	ID3D11DeviceContext* DeviceContext = nullptr;
	ID3D11Device* D3DDevice = nullptr;

	ID3D11RasterizerState* DefaultRasterizerState = nullptr;
	ID3D11RasterizerState* WireframeRasterizerState = nullptr;
	ID3D11RasterizerState* CullFrontRasterizerState = nullptr;
	ID3D11RasterizerState* CullNoneRasterizerState = nullptr;

	ID3D11DepthStencilState* DefaultDepthStencilState = nullptr;
	ID3D11DepthStencilState* GizmoDepthStencilState = nullptr;
	ID3D11DepthStencilState* HighlightDepthStencilState = nullptr;

	ID3D11BlendState* AlphaBlendState = nullptr;

	ID3D11Buffer* TransformConstantBuffer = nullptr;


	FLOAT                   ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	D3D11_VIEWPORT          ViewportInfo;

	ID3D11VertexShader* SimpleVertexShader = nullptr;
	ID3D11PixelShader* SimplePixelShader = nullptr;
	ID3D11InputLayout* SimpleInputLayout = nullptr;
	ID3D11VertexShader* HighlightVertexShader = nullptr;
	ID3D11PixelShader* HighlightPixelShader = nullptr;
	ID3D11VertexShader* LineVertexShader = nullptr;
	ID3D11PixelShader* LinePixelShader = nullptr;
	ID3D11InputLayout* LineInputLayout = nullptr;
	ID3D11Buffer* LineVertexBuffer = nullptr;
	ID3D11Buffer* LineIndexBuffer = nullptr;
	TArray<FVertexSimple> LineVertices;
	TArray<uint32> LineIndices;
	static const UINT MaxLineCount = 8192;
	bool bLineBufferOverflowLogged = false;
	bool bWireframeMode = false;

	// 카메라를 따라다니는 월드 그리드 캐시 (카메라가 그리드 한 칸을 벗어날 때만 재생성)
	TArray<FVertexSimple> CachedGridVertices;
	TArray<uint32> CachedGridIndices;
	bool bGridCacheValid = false;
	float CachedGridCenterX = 0.0f;
	float CachedGridCenterY = 0.0f;

	// ---- Text Billboard ----
	ID3D11VertexShader* TextVertexShader = nullptr;
	ID3D11PixelShader* TextPixelShader = nullptr;
	ID3D11InputLayout* TextInputLayout = nullptr;

	ID3D11SamplerState* FontSamplerState = nullptr;
	ID3D11DepthStencilState* TextDepthStencilState = nullptr;

	ID3D11Buffer* TextVertexBuffer = nullptr;
	ID3D11Buffer* TextIndexBuffer = nullptr;
	static const UINT MaxTextVertices = 8192;

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
	void RenderGizmo(const FPrimitiveRenderData& Data);

	void CreateLineResources();
	void ReleaseLineResources();
	void BeginLineBatch();
	void AddLine(const FVector& Start, const FVector& End, const FVector4& Color);
	void AddWorldGrid(const FVector& CameraWorldPos);
	void RebuildGridCache(const FVector& GridCenter, const FVector& CameraWorldPos);
	void AddBoundingBox(const UPrimitiveComponent* Primitive, const UCameraComponent* Camera);
	void RenderLineBatch(const FMatrix& ViewProj);

	void CreateTextResources();
	void ReleaseTextResources();
	void UpdateTextVertexBuffer(TArray<FVertexText>& Vertices);
	void RenderText(UINT IndexCount);
};
