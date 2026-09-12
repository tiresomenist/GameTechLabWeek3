#include "pch.h"
#include "Engine/Renderer/GDevice.h"
#include "FLineBatcher.h"

void FLineBatcher::AddLine(const FVertexSimple& A, const FVertexSimple& B)
{
	Vertices.Add(A);
	Vertices.Add(B);
}

bool FLineBatcher::Build()
{
	size_t N = Vertices.Num();

	if (N > BufferCapacity)
	{
		ReserveBuffer(N);
	}

	if (!VertexBuffer)
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
	return true;
}

void FLineBatcher::Clear()
{
	Vertices.Empty();
}

void FLineBatcher::Release()
{
	if (VertexBuffer)
	{
		VertexBuffer->Release();
		VertexBuffer = nullptr;
	}
	BufferCapacity = 0;
}

ID3D11Buffer* FLineBatcher::GetVertexBuffer() const
{
	return VertexBuffer;
}

UINT FLineBatcher::GetVertexCount() const
{
	return static_cast<UINT>(Vertices.Num());
}

bool FLineBatcher::ReserveBuffer(size_t Capacity)
{
	if (VertexBuffer)
	{
		VertexBuffer->Release();
		VertexBuffer = nullptr;
	}

	size_t NewCapacity = (std::max)(Capacity, BufferCapacity * 2);

	D3D11_BUFFER_DESC Desc = {};
	Desc.ByteWidth = static_cast<UINT>(sizeof(FVertexSimple) * NewCapacity);
	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ID3D11Device* Device = GDevice::GetInstance()->GetDevice();
	if (FAILED(Device->CreateBuffer(&Desc, nullptr, &VertexBuffer)))
	{
		BufferCapacity = 0;
		return false;
	}

	BufferCapacity = NewCapacity;
	return true;
}
