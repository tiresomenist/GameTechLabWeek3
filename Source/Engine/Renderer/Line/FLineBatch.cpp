#include "pch.h"
#include "Engine/Renderer/Line/FLineBatch.h"
#include "Engine/Renderer/FPrimitiveRenderData.h"
#include <cstring>
#include <format>
#include <limits>
#include <stdexcept>

namespace
{
    void CheckMeshCapacity(uint64 VertexCount, uint64 IndexCount)
    {
        const uint64 MaxArrayCount = (std::numeric_limits<int>::max)();
        const uint64 MaxBufferBytes = (std::numeric_limits<UINT>::max)();
        if (VertexCount > MaxArrayCount || IndexCount > MaxArrayCount ||
            VertexCount > MaxBufferBytes / sizeof(FVertexSimple) ||
            IndexCount > MaxBufferBytes / sizeof(uint32))
        {
            throw std::length_error("Line batch exceeds buffer size limits");
        }
    }

    void CheckLineResult(HRESULT Result, const char* Operation)
    {
        if (FAILED(Result))
        {
            throw std::runtime_error(std::format(
                "Line batch: {} failed (0x{:08X})",
                Operation, static_cast<unsigned long>(Result)));
        }
    }

    void UploadBuffer(ID3D11Device* Device, ID3D11DeviceContext* Context,
        Microsoft::WRL::ComPtr<ID3D11Buffer>& Buffer, UINT& CapacityBytes,
        UINT BindFlags, const void* Data, UINT ByteCount)
    {
        if (!Buffer || CapacityBytes < ByteCount)
        {
            const uint64 DesiredCapacity = (std::max)(
                static_cast<uint64>(ByteCount), uint64(CapacityBytes) * 2);

            D3D11_BUFFER_DESC Desc{};
            Desc.ByteWidth = static_cast<UINT>((std::min)(DesiredCapacity,
                uint64((std::numeric_limits<UINT>::max)())));
            Desc.Usage = D3D11_USAGE_DYNAMIC;
            Desc.BindFlags = BindFlags;
            Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            Microsoft::WRL::ComPtr<ID3D11Buffer> NewBuffer;
            CheckLineResult(Device->CreateBuffer(&Desc, nullptr,
                NewBuffer.GetAddressOf()), "CreateBuffer");
            Buffer = std::move(NewBuffer);
            CapacityBytes = Desc.ByteWidth;
        }

        D3D11_MAPPED_SUBRESOURCE Mapped{};
        CheckLineResult(Context->Map(Buffer.Get(), 0,
            D3D11_MAP_WRITE_DISCARD, 0, &Mapped), "Map");
        std::memcpy(Mapped.pData, Data, ByteCount);
        Context->Unmap(Buffer.Get(), 0);
    }
}

// 리퀘스트 초기화
void FLineBatch::Reset()
{
    Requests.Empty();
    PendingVertexCount = 0;
    PendingIndexCount = 0;
    Vertices.Empty();
    Indices.Empty();
    IndexCount = 0;
}

void FLineBatch::AddLine(const FLineDrawRequest& Request)
{
    //예외처리
    if (Request.Indices.Num() % 2 != 0)
        throw std::invalid_argument("Line request requires an even index count");
    for (uint32 Index : Request.Indices)
    {
        if (Index >= static_cast<uint32>(Request.Points.Num()))
            throw std::invalid_argument("Line request index is outside its points");
    }
    if (Request.Indices.IsEmpty()) return;

    const uint64 VertexCount = uint64(PendingVertexCount) + Request.Points.Num();
    const uint64 TotalIndexCount = uint64(PendingIndexCount) + Request.Indices.Num();
    CheckMeshCapacity(VertexCount, TotalIndexCount);

    Requests.Add(Request);
    PendingVertexCount = static_cast<uint32>(VertexCount);
    PendingIndexCount = static_cast<uint32>(TotalIndexCount);
}

FPrimitiveRenderData FLineBatch::BuildRenderData(ID3D11Device* Device, ID3D11DeviceContext* Context)
{
	//메쉬 빌드->버퍼 업데이트 후 최종적으로 렌더데이터 만들기
	IndexCount = 0;
	BuildMesh();
	UpdateBuffers(Device, Context);
	return CreateRenderData();
}


//렌더 데이터 생성
FPrimitiveRenderData FLineBatch::CreateRenderData() const
{
    FPrimitiveRenderData Data{};
    if (!VertexBuffer || !IndexBuffer || IndexCount == 0) return Data;

    Data.VertexBuffer = VertexBuffer.Get();
    Data.IndexBuffer = IndexBuffer.Get();
    Data.Stride = sizeof(FVertexSimple);
    Data.IndexCount = IndexCount;
    Data.Topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
    return Data;
}

void FLineBatch::Release()
{
    Reset();
    VertexBuffer.Reset();
    IndexBuffer.Reset();
    VertexCapacityBytes = 0;
    IndexCapacityBytes = 0;
}

FLineBatch::FLineBatch() = default;

FLineBatch::~FLineBatch() = default;


void FLineBatch::BuildMesh()
{
    Vertices.SetNum(PendingVertexCount);
    Indices.SetNum(PendingIndexCount);

    uint32 VertexOffset = 0;
    uint32 IndexOffset = 0;
    for (const FLineDrawRequest& Request : Requests)
    {
        const uint32 VertexBase = VertexOffset;
        for (const FVector& Point : Request.Points)
        {
            Vertices[VertexOffset++] = { Point.X, Point.Y, Point.Z,
                Request.R, Request.G, Request.B, Request.A };
        }
        for (uint32 Index : Request.Indices)
            Indices[IndexOffset++] = VertexBase + Index;
    }
}

void FLineBatch::UpdateBuffers(ID3D11Device* Device, ID3D11DeviceContext* Context)
{
    IndexCount = 0;
    if (Vertices.IsEmpty() || Indices.IsEmpty()) return;

    if (!Device || !Context)
        throw std::runtime_error("Line batch requires an initialized D3D11 device");

    UploadBuffer(Device, Context, VertexBuffer, VertexCapacityBytes,
        D3D11_BIND_VERTEX_BUFFER, Vertices.GetData(),
        static_cast<UINT>(size_t(Vertices.Num()) * sizeof(FVertexSimple)));
    UploadBuffer(Device, Context, IndexBuffer, IndexCapacityBytes,
        D3D11_BIND_INDEX_BUFFER, Indices.GetData(),
        static_cast<UINT>(size_t(Indices.Num()) * sizeof(uint32)));

    IndexCount = static_cast<UINT>(Indices.Num());
}
