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

#include "FLineBatcher.h"
#include "Texture/FTexture.h"
#include "Mesh/FGeometryGenerator.h"

// Add a mode here to generate both its enum value and its UI entry.
#define VIEW_MODE_LIST(X) \
	X(Lit) \
	X(Unlit) \
	X(Wireframe)

enum class EViewModeIndex : uint32
{
#define MAKE_VIEW_MODE_ENUM(Name) VMI_##Name,
	VIEW_MODE_LIST(MAKE_VIEW_MODE_ENUM)
#undef MAKE_VIEW_MODE_ENUM
};

struct FViewModeEntry
{
	EViewModeIndex Mode;
	const char* Name;
};

inline constexpr FViewModeEntry ViewModeEntries[] =
{
#define MAKE_VIEW_MODE_ENTRY(Name) { EViewModeIndex::VMI_##Name, #Name },
	VIEW_MODE_LIST(MAKE_VIEW_MODE_ENTRY)
#undef MAKE_VIEW_MODE_ENTRY
};
#undef VIEW_MODE_LIST

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
	FLineBatcher LineBatcher;

	ID3D11RasterizerState* DefaultRasterizerState = nullptr;
	ID3D11RasterizerState* CullFrontRasterizerState = nullptr;
	ID3D11RasterizerState* CullNoneRasterizerState = nullptr;
	ID3D11RasterizerState* WireframeRasterizerState = nullptr;

	EViewModeIndex ViewMode = EViewModeIndex::VMI_Unlit;

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
	ID3D11VertexShader* BatchLineVertexShader = nullptr;
	ID3D11PixelShader* BatchLinePixelShader = nullptr;

	// ---- Text Billboard ----
	ID3D11VertexShader* TextVertexShader = nullptr;
	ID3D11PixelShader* TextPixelShader = nullptr;
	ID3D11InputLayout* TextInputLayout = nullptr;

	// Texture
	FTexture Texture;
	ID3D11VertexShader* TextureVertexShader = nullptr;
	ID3D11PixelShader* TexturePixelShader = nullptr;
	ID3D11InputLayout* TextureInputLayout = nullptr;
	ID3D11Buffer* PlaneVertexBuffer = nullptr;
	ID3D11Buffer* PlaneIndexBuffer = nullptr;
	UINT PlaneIndexCount = 0;

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
	void RenderBatchLine(const FMatrix& ViewProj);

	void CreateTextResources();
	void ReleaseTextResources();
	void UpdateTextVertexBuffer(TArray<FVertexText>& Vertices);
	void RenderText(UINT IndexCount);

	void CreateTexturePlaneResources();
	void ReleaseTexturePlaneResources();
	void RenderTexturePlane(const FMatrix& ViewProj);
};
