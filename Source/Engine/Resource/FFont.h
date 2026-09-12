#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "Core/Container/FString.h"
#include "Core/Container/TArray.h"
#include "Core/Container/TMap.h"
#include "Core/Math/FVector.h"
#include "Engine/Renderer/FPrimitiveRenderData.h"
#include "Engine/Renderer/FVertexSimple.h"

struct FCharacterInfo {
    float u = 0;
    float v = 0;
    float width = 0;
    float height = 0;
};

struct FFontMeshData
{
    TArray<FFontVertex> Vertices;
    TArray<uint32_t> Indices;
};

struct FTextDrawRequest
{
    FString Text;
    FVector WorldPosition;
    float CharacterHeight = 0.35f;
};

// Atlas + character mapping + world-space text mesh. FRenderer binds and draws.
class FFont
{
public:
    FFont() = default;
    FFont(const FFont&) = delete;
    FFont& operator=(const FFont&) = delete;
    void Initialize(ID3D11Device* Device);
    const FCharacterInfo& GetCharacterInfo(uint32 CharacterCode) const;
    // Center is the center of the first line. Each line is horizontally centered.
    // CameraRight/Up must be the current camera's unit basis vectors.
    FFontMeshData BuildMesh(FStringView Text, const FVector& Center,
        const FVector& CameraRight, const FVector& CameraUp, float CharacterHeight) const;
    void UpdateMesh(const FFontMeshData& Mesh);
    // Borrowed GPU references: valid until the next UpdateMesh/Release.
    FPrimitiveRenderData CreateRenderData() const;
    UINT GetWidth() const { return TextureWidth; }
    UINT GetHeight() const { return TextureHeight; }
    void Release();
    FPrimitiveRenderData BuildRenderList(const TArray<FTextDrawRequest>& InRequests,FVector InRight,FVector InUP);
private:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureView;
    Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;
    TMap<uint32, FCharacterInfo> Characters;
    UINT VertexCapacityBytes = 0;
    UINT IndexCapacityBytes = 0;
    UINT IndexCount = 0;
    UINT TextureWidth = 0;
    UINT TextureHeight = 0;
};
