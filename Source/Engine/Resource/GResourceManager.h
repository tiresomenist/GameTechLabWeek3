#ifndef NOMINMAX
#define NOMINMAX
#endif

#pragma once
#include <unordered_map>
#include <map>
#include <string>
#include <span>
#include "Engine/Renderer/GDevice.h"
#include "Core/Core.h"
#include "Core/Container/FString.h"
#include "Core/Container/TArray.h"
#include "Engine/Renderer/FVertexSimple.h"
#include "Engine/Resource/FMeshResource.h"
#include "Engine/Renderer/Text/FFontAtlas.h"
#include "Core/Container/TMap.h"

class FTextureResource;
struct FShaderResource
{
	ID3D11VertexShader* VertexShader = nullptr;
	ID3D11PixelShader* PixelShader = nullptr;
	ID3D11InputLayout* InputLayout = nullptr;
};


class GResourceManager
{
public:
	static GResourceManager* GetInstance();

	void Initialize(GDevice* InDevice);
	void Shutdown();
	FMeshResource* CreateMesh(const FString& MeshName, std::span<const FVertexSimple> Vertices, std::span<const uint32> Indices);
	// 위치와 UV 정점으로 삼각형 메시를 생성하며 실패 시 nullptr을 반환함
	FMeshResource* CreateTexturedMesh(const FString& MeshName, const TArray<FVertexTexture>& Vertices, const TArray<uint32>& Indices);
	FMeshResource* GetPrimitive(const FString& Type);
	FFontAtlas* GetDefaultFont() { return DefaultFont.GetSRV() ? &DefaultFont : nullptr; }
	FTextureResource* GetOrLoadTexture(const FString& FilePath);
	FShaderResource* GetShader(
		const std::wstring& FilePath,
		const std::string& VSEntry,
		const std::string& PSEntry,
		const D3D11_INPUT_ELEMENT_DESC* Layout,
		UINT LayoutCount
	);

private:
	GResourceManager() = default;
	~GResourceManager() = default;
	GResourceManager(const GResourceManager&) = delete;
	GResourceManager& operator=(const GResourceManager&) = delete;

	GDevice* Device = nullptr;

	std::unordered_map<std::string, FMeshResource*> PrimitiveCache;
	TMap<FString, FTextureResource*> TextureCache;
	FFontAtlas DefaultFont;
	//std::unordered_map<std::string, FShaderResource*> ShaderCache;	// 일단 Renderer에서 - 셰이더 무조건 하나만 쓰니까..
	//std::map<std::pair<D3D11_FILL_MODE, D3D11_CULL_MODE>, ID3D11RasterizerState*> RasterizerStateCache;
};

