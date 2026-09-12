#include "pch.h"
#include "Engine/Renderer/GDevice.h"
#include "FLineBatcher.h"
#include "Engine/Log.h"

void FLineBatcher::AddLine(const FVertexSimple& A, const FVertexSimple& B)
{
	const uint32 N = static_cast<uint32>(Vertices.Num());
	Vertices.Add(A);
	Vertices.Add(B);
	Indexes.Add(N);
	Indexes.Add(N + 1);
}

void FLineBatcher::AddBoundBox(const FVector& Min, const FVector& Max, const FMatrix& World)
{
	const FVector4 BoxColor(1.0f, 1.0f, 0.0f, 1.0f);
	const uint32 Base = static_cast<uint32>(Vertices.Num());

	// 코너 인덱스의 비트 0/1/2 = X/Y/Z가 Min인지 Max인지.
	// 로컬에서 코너를 조합한 뒤에 World로 옮겨야 회전한 물체에서도 맞는다.
	for (int i = 0; i < 8; ++i)
	{
		const FVector4 Local(
			(i & 1) ? Max.X : Min.X,
			(i & 2) ? Max.Y : Min.Y,
			(i & 4) ? Max.Z : Min.Z,
			1.0f);

		const FVector Corner = (Local * World).getXYZ();
		Vertices.Add({ Corner.X, Corner.Y, Corner.Z, BoxColor.X, BoxColor.Y, BoxColor.Z, BoxColor.W });
	}

	// 두 코너가 비트 하나만 다르면 그게 엣지. 정점 8개를 12개 엣지가 공유한다
	for (uint32 i = 0; i < 8; ++i)
	{
		for (uint32 Bit = 1; Bit <= 4; Bit <<= 1)
		{
			if (!(i & Bit))
			{
				Indexes.Add(Base + i);
				Indexes.Add(Base + (i | Bit));
			}
		}
	}
}

bool FLineBatcher::Build()
{
	size_t N = Vertices.Num();

	if (N > VertexBufferCapacity)
	{
		if (!ReserveVertexBuffer(N))
		{
			return false;
		}
	}

	size_t M = Indexes.Num();

	if (M > IndexBufferCapacity)
	{
		if (!ReserveIndexBuffer(M))
		{
			return false;
		}
	}

	if (!VertexBuffer || !IndexBuffer)
	{
		return false;
	}

	ID3D11DeviceContext* Context = GDevice::GetInstance()->GetContext();
	D3D11_MAPPED_SUBRESOURCE Mapped = {};
	if (FAILED(Context->Map(VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped)))
	{
		return false;
	}
	memcpy(Mapped.pData, Vertices.GetData(), sizeof(FVertexSimple) * N);
	Context->Unmap(VertexBuffer, 0);

	Mapped = {};
	if (FAILED(Context->Map(IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped)))
	{
		return false;
	}
	memcpy(Mapped.pData, Indexes.GetData(), sizeof(uint32) * M);
	Context->Unmap(IndexBuffer, 0);

	return true;
}

void FLineBatcher::Clear()
{
	Vertices.Empty();
	Indexes.Empty();
}

void FLineBatcher::Release()
{
	if (VertexBuffer)
	{
		VertexBuffer->Release();
		VertexBuffer = nullptr;
	}
	if (IndexBuffer)
	{
		IndexBuffer->Release();
		IndexBuffer = nullptr;
	}
	VertexBufferCapacity = 0;
	IndexBufferCapacity = 0;
}

ID3D11Buffer* FLineBatcher::GetVertexBuffer() const
{
	return VertexBuffer;
}

ID3D11Buffer* FLineBatcher::GetIndexBuffer() const
{
	return IndexBuffer;
}

UINT FLineBatcher::GetVertexCount() const
{
	return static_cast<UINT>(Vertices.Num());
}

UINT FLineBatcher::GetIndexCount() const
{
	return static_cast<UINT>(Indexes.Num());
}

bool FLineBatcher::ReserveVertexBuffer(size_t Need)
{
	if (VertexBuffer)
	{
		VertexBuffer->Release();
		VertexBuffer = nullptr;
	}

	const size_t NewCapacity = (std::max)(Need, VertexBufferCapacity * 2);

	D3D11_BUFFER_DESC Desc = {};
	Desc.ByteWidth = static_cast<UINT>(sizeof(FVertexSimple) * NewCapacity);
	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ID3D11Device* Device = GDevice::GetInstance()->GetDevice();
	const HRESULT hr = Device->CreateBuffer(&Desc, nullptr, &VertexBuffer);
	if (FAILED(hr))
	{
		UE_LOG("[FLineBatcher] Failed to create Vertex Buffer. HRESULT: {}\n", hr);
		VertexBufferCapacity = 0;
		return false;
	}

	VertexBufferCapacity = NewCapacity;
	return true;
}

bool FLineBatcher::ReserveIndexBuffer(size_t Need)
{
	if (IndexBuffer)
	{
		IndexBuffer->Release();
		IndexBuffer = nullptr;
	}

	const size_t NewCapacity = (std::max)(Need, IndexBufferCapacity * 2);

	D3D11_BUFFER_DESC Desc = {};
	Desc.ByteWidth = static_cast<UINT>(sizeof(uint32) * NewCapacity);
	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ID3D11Device* Device = GDevice::GetInstance()->GetDevice();
	const HRESULT hr = Device->CreateBuffer(&Desc, nullptr, &IndexBuffer);
	if (FAILED(hr))
	{
		UE_LOG("[FLineBatcher] Failed to create Index Buffer. HRESULT: {}\n", hr);
		IndexBufferCapacity = 0;
		return false;
	}

	IndexBufferCapacity = NewCapacity;
	return true;
}
