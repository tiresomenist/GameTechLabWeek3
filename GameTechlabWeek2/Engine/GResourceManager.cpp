#include "pch.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "GResourceManager.h"
#include "Engine/Renderer/FVertexSimple.h"
#include "Models/Sphere.h"
#include "Models/Cube.h"
#include "Models/Plane.h"
#include "Models/Triangle.h"
#include "Models/PePe.h"
#include "Models/Octopus.h"
#include "Models/ArrowRed.h"
#include "Models/ArrowGreen.h"
#include "Models/ArrowBlue.h"
#include "Models/MoveRed.h"
#include "Models/MoveGreen.h"
#include "Models/MoveBlue.h"
#include "Models/ScaleRed.h"
#include "Models/ScaleGreen.h"
#include "Models/ScaleBlue.h"
#include "Models/RotateRed.h"
#include "Models/RotateGreen.h"
#include "Models/RotateBlue.h"
#include "Models/Grid.h"
#include "FVector.h"
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
    Mesh->vertexs.GetVector().assign(Vertices.begin(), Vertices.end());
    Mesh->indexes.GetVector().assign(Indices.begin(), Indices.end());
    Mesh->VertexCount = static_cast<UINT>(Vertices.size());
    Mesh->IndexCount = static_cast<UINT>(Indices.size());
    Mesh->Stride = sizeof(FVertexSimple);
    Mesh->VertexBuffer = Device->CreateVertexBuffer(&Mesh->vertexs[0], Mesh->Stride * Mesh->VertexCount);
    if (!Mesh->VertexBuffer) return nullptr;
    Mesh->IndexBuffer = Device->CreateIndexBuffer(&Mesh->indexes[0], sizeof(uint32) * Mesh->IndexCount);
    if (!Mesh->IndexBuffer) return nullptr;
    Mesh->bHasBounds = false;
    if (Mesh->vertexs.Num() > 0)
    {
        const auto& First = Mesh->vertexs[0];

        Mesh->BoundsMin = FVector(First.x, First.y, First.z);   
        Mesh->BoundsMax = Mesh->BoundsMin;

        for (const auto& Vertex : Mesh->vertexs)
        {
            Mesh->BoundsMin.X = (std::min)(Mesh->BoundsMin.X, Vertex.x);
            Mesh->BoundsMin.Y = (std::min)(Mesh->BoundsMin.Y, Vertex.y);
            Mesh->BoundsMin.Z = (std::min)(Mesh->BoundsMin.Z, Vertex.z);

            Mesh->BoundsMax.X = (std::max)(Mesh->BoundsMax.X, Vertex.x);
            Mesh->BoundsMax.Y = (std::max)(Mesh->BoundsMax.Y, Vertex.y);
            Mesh->BoundsMax.Z = (std::max)(Mesh->BoundsMax.Z, Vertex.z);
        }

        Mesh->bHasBounds = true;
    }

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
    PrimitiveCache.clear();
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
