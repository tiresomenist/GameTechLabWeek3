#include "pch.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "GResourceManager.h"
#include "Engine/Renderer/FVertexSimple.h"
#include "WICTextureLoader/WICTextureLoader11.h"
#include "Engine/Resource/MeshData/Sphere.h"
#include "Engine/Resource/MeshData/Cube.h"
#include "Engine/Resource/MeshData/Plane.h"
#include "Engine/Resource/MeshData/Triangle.h"
#include "Engine/Resource/MeshData/PePe.h"
#include "Engine/Resource/MeshData/Octopus.h"
#include "Engine/Resource/MeshData/ArrowRed.h"
#include "Engine/Resource/MeshData/ArrowGreen.h"
#include "Engine/Resource/MeshData/ArrowBlue.h"
#include "Engine/Resource/MeshData/MoveRed.h"
#include "Engine/Resource/MeshData/MoveGreen.h"
#include "Engine/Resource/MeshData/MoveBlue.h"
#include "Engine/Resource/MeshData/ScaleRed.h"
#include "Engine/Resource/MeshData/ScaleGreen.h"
#include "Engine/Resource/MeshData/ScaleBlue.h"
#include "Engine/Resource/MeshData/RotateRed.h"
#include "Engine/Resource/MeshData/RotateGreen.h"
#include "Engine/Resource/MeshData/RotateBlue.h"
#include "Engine/Resource/MeshData/Grid.h"
#include "Core/Math/FVector.h"
#include "Engine/Log.h"
#include <memory>
#include <limits>
#include <stdexcept>
#include <cmath>


GResourceManager* GResourceManager::GetInstance()
{
	static GResourceManager Instance;
	return &Instance;
}

void GResourceManager::Initialize(GDevice* InDevice)
{
    Device = InDevice;
	if (!Device || !Device->GetDevice() || !DefaultFont.Build(Device->GetDevice(), "Assets/Fonts/Pretendard-Regular.ttf", 24.0f))
		throw std::runtime_error("Default font atlas build failed");
    if (!CreateMesh("Sphere", sphere_vertices, sphere_indices)) throw std::runtime_error("Required mesh creation failed");
    if (!CreateMesh("Cube", cube_vertices, cube_indices)) throw std::runtime_error("Required mesh creation failed");
    if (!CreateMesh("Triangle", triangle_vertices, triangle_indices)) throw std::runtime_error("Required mesh creation failed");
    if (!CreateMesh("Plane", plane_vertices, plane_indices)) throw std::runtime_error("Required mesh creation failed");
    if (!CreateMesh("Pepe", pepe_vertices, pepe_indices)) throw std::runtime_error("Required mesh creation failed");
    if (!CreateMesh("Octopus", octopus_vertices, octopus_indices)) throw std::runtime_error("Required mesh creation failed");
    if (!CreateMesh("ArrowRed", arrow_red_vertices, arrow_red_indices)) throw std::runtime_error("Required mesh creation failed");
    if (!CreateMesh("ArrowGreen", arrow_green_vertices, arrow_green_indices)) throw std::runtime_error("Required mesh creation failed");
    if (!CreateMesh("ArrowBlue", arrow_blue_vertices, arrow_blue_indices)) throw std::runtime_error("Required mesh creation failed");
    if (!CreateMesh("MoveRed", move_red_vertices, move_red_indices)) throw std::runtime_error("Required mesh creation failed");
    if (!CreateMesh("MoveGreen", move_green_vertices, move_green_indices)) throw std::runtime_error("Required mesh creation failed");
    if (!CreateMesh("MoveBlue", move_blue_vertices, move_blue_indices)) throw std::runtime_error("Required mesh creation failed");
    if (!CreateMesh("ScaleRed", scale_red_vertices, scale_red_indices)) throw std::runtime_error("Required mesh creation failed");
    if (!CreateMesh("ScaleGreen", scale_green_vertices, scale_green_indices)) throw std::runtime_error("Required mesh creation failed");
    if (!CreateMesh("ScaleBlue", scale_blue_vertices, scale_blue_indices)) throw std::runtime_error("Required mesh creation failed");
    if (!CreateMesh("RotateRed", rotate_red_vertices, rotate_red_indices)) throw std::runtime_error("Required mesh creation failed");
    if (!CreateMesh("RotateGreen", rotate_green_vertices, rotate_green_indices)) throw std::runtime_error("Required mesh creation failed");
    if (!CreateMesh("RotateBlue", rotate_blue_vertices, rotate_blue_indices)) throw std::runtime_error("Required mesh creation failed");
    if (!CreateMesh("Grid", grid_vertices, grid_indices)) throw std::runtime_error("Required mesh creation failed");
}

FMeshResource* GResourceManager::CreateMesh(const FString& MeshName,
    std::span<const FVertexSimple> Vertices, std::span<const uint32> Indices)
{
    std::vector<FVector> Positions;
    Positions.reserve(Vertices.size());
    for (const FVertexSimple& Vertex : Vertices)
    {
        Positions.emplace_back(Vertex.x, Vertex.y, Vertex.z);
    }
    return CreateMeshInternal(MeshName, Vertices.data(), Vertices.size(), sizeof(FVertexSimple), Positions, Indices);
}

FMeshResource* GResourceManager::CreateMesh(const FString& MeshName,
    std::span<const FVertexTexture> Vertices, std::span<const uint32> Indices)
{
    std::vector<FVector> Positions;
    Positions.reserve(Vertices.size());
    for (const FVertexTexture& Vertex : Vertices)
    {
        Positions.emplace_back(Vertex.x, Vertex.y, Vertex.z);
    }
    return CreateMeshInternal(MeshName, Vertices.data(), Vertices.size(), sizeof(FVertexTexture), Positions, Indices);
}

FMeshResource* GResourceManager::CreateMeshInternal(const FString& MeshName, const void* VertexData, size_t VertexCount,
    UINT VertexStride, std::span<const FVector> Positions, std::span<const uint32> Indices)
{
    auto Existing = PrimitiveCache.find(MeshName);
    if (Existing != PrimitiveCache.end()) return Existing->second;
    if (VertexData == nullptr || VertexCount == 0 || Positions.size() != VertexCount || Indices.empty()) return nullptr;
    const size_t MaxBytes = (std::numeric_limits<UINT>::max)();
    if (VertexCount > MaxBytes / VertexStride || Indices.size() > MaxBytes / sizeof(uint32))
        return nullptr;
    for (const FVector& Position : Positions)
		if (!std::isfinite(Position.X) || !std::isfinite(Position.Y) || !std::isfinite(Position.Z)) return nullptr;
    if (!Device || !Device->GetDevice()) return nullptr;
    for (uint32 Index : Indices)
        if (Index >= VertexCount) return nullptr;

    auto Mesh = std::make_unique<FMeshResource>();
	Mesh->Positions.GetVector().assign(Positions.begin(), Positions.end());
    Mesh->indexes.GetVector().assign(Indices.begin(), Indices.end());
    Mesh->VertexCount = static_cast<UINT>(VertexCount);
    Mesh->IndexCount = static_cast<UINT>(Indices.size());
    Mesh->Stride = VertexStride;
    Mesh->VertexBuffer = Device->CreateVertexBuffer(VertexData, Mesh->Stride * Mesh->VertexCount);
    if (!Mesh->VertexBuffer) return nullptr;
    Mesh->IndexBuffer = Device->CreateIndexBuffer(&Mesh->indexes[0], sizeof(uint32) * Mesh->IndexCount);
    if (!Mesh->IndexBuffer) return nullptr;
    Mesh->bHasBounds = false;
    if (Mesh->Positions.Num() > 0)
    {
        const FVector& First = Mesh->Positions[0];

		Mesh->BoundsMin = First;
        Mesh->BoundsMax = Mesh->BoundsMin;

		for (const FVector& Position : Mesh->Positions)
        {
            Mesh->BoundsMin.X = (std::min)(Mesh->BoundsMin.X, Position.X);
            Mesh->BoundsMin.Y = (std::min)(Mesh->BoundsMin.Y, Position.Y);
            Mesh->BoundsMin.Z = (std::min)(Mesh->BoundsMin.Z, Position.Z);

            Mesh->BoundsMax.X = (std::max)(Mesh->BoundsMax.X, Position.X);
            Mesh->BoundsMax.Y = (std::max)(Mesh->BoundsMax.Y, Position.Y);
            Mesh->BoundsMax.Z = (std::max)(Mesh->BoundsMax.Z, Position.Z);
        }

        Mesh->bHasBounds = true;
    }

    const auto [It, Inserted] = PrimitiveCache.emplace(MeshName, Mesh.get());
    if (Inserted) Mesh.release();
    return It->second;
    
}   

FTextureResource* GResourceManager::LoadTexture(FStringView FilePath)
{
    const FString Key{ FilePath };
    if (const auto Existing = TextureCache.find(Key); Existing != TextureCache.end()) return Existing->second;
    if (!Device || !Device->GetDevice() || !Device->GetContext()) return nullptr;

    auto Texture = std::make_unique<FTextureResource>();
    const std::wstring WidePath = std::filesystem::absolute(std::filesystem::path(Key)).wstring();
    const HRESULT Result = DirectX::CreateWICTextureFromFile(
        Device->GetDevice(), Device->GetContext(), WidePath.c_str(), nullptr, &Texture->ShaderResourceView);
    if (FAILED(Result))
    {
        UE_LOG("[Resource] 텍스처 로드 실패: {} ({})", Key, static_cast<uint32>(Result));
        return nullptr;
    }

    const auto [It, Inserted] = TextureCache.emplace(Key, Texture.get());
    if (Inserted) Texture.release();
    return It->second;
}

void GResourceManager::Shutdown()
{
    for (auto& [type, mesh] : PrimitiveCache)
    {
        delete mesh;
    }
    PrimitiveCache.clear();
	for (auto& [path, texture] : TextureCache)
	{
		delete texture;
	}
	TextureCache.clear();
	DefaultFont.Release();
    Device = nullptr;

    //for (auto& [path, shader] : ShaderCache)
    //{
    //    if (shader->VertexShader) shader->VertexShader->Release();
    //    if (shader->PixelShader)  shader->PixelShader->Release();
    //    if (shader->InputLayout)  shader->InputLayout->Release();
    //    delete shader;
    //}
    //ShaderCache.clear();

    //for (auto& [key, state] : RasterizerStateCache)
    //    state->Release();
    //RasterizerStateCache.clear();
}

FMeshResource* GResourceManager::GetPrimitive(const FString& Type)
{
    auto Item = PrimitiveCache.find(Type);

    if (Item != PrimitiveCache.end())
    {
        return Item->second;
    }
    else
    {
        return nullptr;
    }
}

FShaderResource* GResourceManager::GetShader(const std::wstring& FilePath, const std::string& VSEntry, const std::string& PSEntry, const D3D11_INPUT_ELEMENT_DESC* Layout, UINT LayoutCount)
{
	return nullptr;
}
