#pragma once
#include <d3d11.h>
#include "Core/Container/TArray.h"
#include <wrl/client.h>
#include "Engine/Renderer/Line/FLineDrawRequest.h"
#include "Engine/Renderer/FVertexSimple.h"
#include "Engine/Renderer/FPrimitiveRenderData.h"

class FLineBatch
{
public:
    void Reset();

    // Copies one indexed line group. No shape-specific handling is needed.
    void AddLine(const FLineDrawRequest& Request);

    FPrimitiveRenderData BuildRenderData(ID3D11Device* Device, ID3D11DeviceContext* Context);

    void Release();

	FLineBatch();
	~FLineBatch();
    FLineBatch(const FLineBatch&) = delete;
    FLineBatch& operator=(const FLineBatch&) = delete;

private:
    void BuildMesh();

    void UpdateBuffers(ID3D11Device* Device, ID3D11DeviceContext* Context);
    FPrimitiveRenderData CreateRenderData() const;

    TArray<FLineDrawRequest> Requests;
    uint32 PendingVertexCount = 0;
    uint32 PendingIndexCount = 0;
    TArray<FVertexSimple> Vertices;
    TArray<uint32> Indices;

    Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;

    UINT VertexCapacityBytes = 0;
    UINT IndexCapacityBytes = 0;
    UINT IndexCount = 0;
};

