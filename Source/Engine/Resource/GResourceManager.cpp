#include "pch.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "GResourceManager.h"
#include "Engine/Renderer/FVertexSimple.h"
#include "Engine/Resource/MeshData/Sphere.h"
#include "Engine/Resource/MeshData/Cube.h"
#include "Engine/Resource/MeshData/Plane.h"
#include "Engine/Resource/MeshData/Flame.h"
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
#include "Engine/Resource/FTextureResource.h"
#include "FGeometryGenerator.h"

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
    RegisterDefaultPrimitives(InDevice);
    RegisterTexturePrimitives(InDevice);
}

FMeshResource* GResourceManager::CreateMesh(const FString& MeshName,
    std::span<const FVertexSimple> Vertices, std::span<const uint32> Indices)
{
    auto Existing = PrimitiveCache.find(MeshName);
    if (Existing != PrimitiveCache.end()) return Existing->second;
    if (Vertices.empty() || Indices.empty()) return nullptr;
    const size_t MaxBytes = (std::numeric_limits<UINT>::max)();
    if (Vertices.size() > MaxBytes / sizeof(FVertexSimple) || Indices.size() > MaxBytes / sizeof(uint32))
        return nullptr;
    for (const auto& V : Vertices)
        if (!std::isfinite(V.x) || !std::isfinite(V.y) || !std::isfinite(V.z)) return nullptr;
    if (!Device || !Device->GetDevice()) return nullptr;
    for (uint32 Index : Indices)
        if (Index >= Vertices.size()) return nullptr;

    auto Mesh = std::make_unique<FMeshResource>();
    // CPU 피킹과 바운딩 계산에 필요한 로컬 위치만 보관함
    for (const auto& Vertex : Vertices)
        Mesh->Positions.Add(FVector(Vertex.x, Vertex.y, Vertex.z));
    Mesh->indexes.GetVector().assign(Indices.begin(), Indices.end());
    Mesh->VertexCount = static_cast<UINT>(Vertices.size());
    Mesh->IndexCount = static_cast<UINT>(Indices.size());
    Mesh->Stride = sizeof(FVertexSimple);
    // GPU에는 색상 정보를 포함한 원본 정점을 업로드함
    Mesh->VertexBuffer = Device->CreateVertexBuffer(Vertices.data(), Mesh->Stride * Mesh->VertexCount);
    if (!Mesh->VertexBuffer) return nullptr;
    Mesh->IndexBuffer = Device->CreateIndexBuffer(&Mesh->indexes[0], sizeof(uint32) * Mesh->IndexCount);
    if (!Mesh->IndexBuffer) return nullptr;
    Mesh->bHasBounds = false;
    if (Mesh->Positions.Num() > 0)
    {
        Mesh->BoundsMin = Mesh->Positions[0];
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

FMeshResource* GResourceManager::CreateTexturedMesh(const FString& MeshName,
    const TArray<FVertexTexture>& Vertices, const TArray<uint32>& Indices)
{
    // 같은 이름의 텍스처 메시를 재사용하며 다른 정점 형식과의 충돌을 거부함
    const auto Existing = PrimitiveCache.find(MeshName);
    if (Existing != PrimitiveCache.end())
        return Existing->second->GetStride() == sizeof(FVertexTexture) ? Existing->second : nullptr;

    if (!Device || !Device->GetDevice()) return nullptr;
    if (Vertices.IsEmpty() || Indices.IsEmpty() || Indices.Num() % 3 != 0) return nullptr;

    // 버퍼 크기를 UINT로 변환하기 전에 곱셈 오버플로를 검사함
    const size_t VertexCount = static_cast<size_t>(Vertices.Num());
    const size_t IndexCount = static_cast<size_t>(Indices.Num());
    const size_t MaxBytes = (std::numeric_limits<UINT>::max)();
    if (VertexCount > MaxBytes / sizeof(FVertexTexture) || IndexCount > MaxBytes / sizeof(uint32))
        return nullptr;

    for (const auto& Vertex : Vertices)
    {
        if (!std::isfinite(Vertex.x) || !std::isfinite(Vertex.y) || !std::isfinite(Vertex.z) ||
            !std::isfinite(Vertex.u) || !std::isfinite(Vertex.v))
            return nullptr;
    }
    for (uint32 Index : Indices)
        if (Index >= VertexCount) return nullptr;

    // 생성 도중 실패하면 이미 생성된 버퍼도 메시 소멸자에서 해제함
    auto Mesh = std::make_unique<FMeshResource>();
    Mesh->VertexCount = static_cast<UINT>(VertexCount);
    Mesh->IndexCount = static_cast<UINT>(IndexCount);
    Mesh->Stride = sizeof(FVertexTexture);
    Mesh->indexes = Indices;

    // CPU에는 피킹과 바운딩 계산에 사용하는 위치만 보관함
    Mesh->Positions.SetNum(VertexCount);
    for (size_t Index = 0; Index < VertexCount; ++Index)
    {
        const auto& Vertex = Vertices[Index];
        Mesh->Positions[Index] = FVector(Vertex.x, Vertex.y, Vertex.z);
    }

    // GPU에는 위치와 UV가 포함된 원본 정점을 업로드함
    Mesh->VertexBuffer = Device->CreateVertexBuffer(
        &Vertices[0], static_cast<UINT>(VertexCount * sizeof(FVertexTexture)));
    if (!Mesh->VertexBuffer) return nullptr;

    Mesh->IndexBuffer = Device->CreateIndexBuffer(
        &Mesh->indexes[0], static_cast<UINT>(IndexCount * sizeof(uint32)));
    if (!Mesh->IndexBuffer) return nullptr;

    // 로컬 위치의 축별 최솟값과 최댓값으로 바운딩 박스를 계산함
    Mesh->BoundsMin = Mesh->Positions[0];
    Mesh->BoundsMax = Mesh->Positions[0];
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

    // 등록 성공 시 리소스 매니저가 메시 소유권을 넘겨받음
    const auto [It, Inserted] = PrimitiveCache.emplace(MeshName, Mesh.get());
    if (Inserted) Mesh.release();
    return It->second;
}

void GResourceManager::Shutdown()
{
    for (auto& [type, mesh] : PrimitiveCache)
    {
        delete mesh;
    }
    for (auto& [Path, Texture] : TextureCache)
    {
        delete Texture;
    }
    TextureCache.Empty();
    PrimitiveCache.clear();
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

FTextureResource* GResourceManager::GetOrLoadTexture(const FString& FilePath)
{
    // 이미 로드한 경로라면 기존 리소스를 반환함
    if (FTextureResource** Existing = TextureCache.Find(FilePath))
    {
        return *Existing;
    }

    if (!Device || !Device->GetDevice())
    {
        throw std::runtime_error("Texture device is not initialized");
    }

    // 로딩이나 캐시 등록이 실패하면 임시 객체를 자동 해제함
    auto NewTexture = std::make_unique<FTextureResource>();

    NewTexture->Load(Device->GetDevice(), FilePath);

    // 로딩에 성공한 리소스만 캐시에 등록함
    if (!TextureCache.Add(FilePath, NewTexture.get()))
    {
        // 같은 키가 이미 존재하면 임시 객체를 해제하고 기존 값을 반환함
        return *TextureCache.Find(FilePath);
    }

    // 캐시 등록 후 소유권을 리소스 매니저로 이전함
    return NewTexture.release();
}

FShaderResource* GResourceManager::GetShader(const std::wstring& FilePath, const std::string& VSEntry, const std::string& PSEntry, const D3D11_INPUT_ELEMENT_DESC* Layout, UINT LayoutCount)
{
	return nullptr;
}

void GResourceManager::RegisterDefaultPrimitives(GDevice* InDevice)
{
    //if (!CreateMesh("Sphere", sphere_vertices, sphere_indices)) throw std::runtime_error("Required mesh creation failed");
    //if (!CreateMesh("Cube", cube_vertices, cube_indices)) throw std::runtime_error("Required mesh creation failed");
    if (!CreateMesh("Triangle", triangle_vertices, triangle_indices)) throw std::runtime_error("Required mesh creation failed");
    //if (!CreateMesh("Plane", plane_vertices, plane_indices)) throw std::runtime_error("Required mesh creation failed");
    if (!CreateTexturedMesh("Flame", flame_vertices, flame_indices)) throw std::runtime_error("Flame mesh creation failed");
    // UV가 포함된 경량화 페페를 텍스처 정점 형식으로 등록함
    if (!CreateTexturedMesh("Pepe", pepe_vertices, pepe_indices)) throw std::runtime_error("Pepe mesh creation failed");
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


void GResourceManager::RegisterTexturePrimitives(GDevice* InDevice)
{
    TArray<FVertexTexture> CubeVertices;
    TArray<uint32> CubeIndices;
    FGeometryGenerator::CreateCube(2.0f, 2.0f, 2.0f, CubeVertices, CubeIndices);

    if (!CreateTexturedMesh("Cube", CubeVertices, CubeIndices))
    {
        throw std::runtime_error("TexturedCube mesh creation failed");
    }

    TArray<FVertexTexture> SphereVertices;
    TArray<uint32> SphereIndices;
    // 반지름, 세로로 자르는 개수, 가로로 자르는 개수
    FGeometryGenerator::CreateSphere(1.0f, 64, 32, SphereVertices, SphereIndices);

    if (!CreateTexturedMesh("Sphere", SphereVertices, SphereIndices))
    {
        throw std::runtime_error("TexturedCube mesh creation failed");
    }

    TArray<FVertexTexture> PlaneVertices;
    TArray<uint32> PlaneIndices;
    FGeometryGenerator::CreatePlane(1.0f, 1.0f, 1, 1, PlaneVertices, PlaneIndices);

    if (!CreateTexturedMesh("Plane", PlaneVertices, PlaneIndices))
    {
        throw std::runtime_error("TexturedCube mesh creation failed");
    }

    // SpotLight의 빌보드 아이콘용 정점데이터
    const TArray<FVertexTexture> IconVertices
    {
        { -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f }, //좌하단
        { 0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },  //우하단
        { 0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f },  //우상단
        { -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f }  //좌상단
    };
    const TArray<uint32> IconIndices
    {
        0, 1, 2,
        0, 2, 3
    };
    // 모든 SpotLight가 재사용할 아이콘 메시 등록함
    if (!CreateTexturedMesh("SpotLightIcon", IconVertices, IconIndices))
    {
        throw std::runtime_error("SpotLight icon mesh creation failed");
    }
}
