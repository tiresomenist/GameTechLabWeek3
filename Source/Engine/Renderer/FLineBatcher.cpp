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

void FLineBatcher::AddGrid(FGrid Grid, FVector CameraPos)
{
	if (Grid.LineNum < 2 || Grid.Interval <= 0.0f)
	{
		return;
	}

	const FVector GridColor(1.0f, 1.0f, 1.0f);
	const float Length = (Grid.LineNum - 1) * Grid.Interval;
	const float Half = Length * 0.5f;
	const float HalfSqured = Half * Half;
	const float ZSquared = CameraPos.Z * CameraPos.Z;

	// 카메라를 Interval 배수로 스냅해야 카메라가 움직여도 격자가 미끄러지지 않고,
	// 선들이 Interval의 정수배 위치에 놓여서 아래 축 판정이 성립한다.
	const int32 HalfCount = static_cast<int32>(Grid.LineNum) / 2;
	const float MinX = floor(CameraPos.X / Grid.Interval) * Grid.Interval - HalfCount * Grid.Interval;
	const float MinY = floor(CameraPos.Y / Grid.Interval) * Grid.Interval - HalfCount * Grid.Interval;

	// 부동소수 누적 오차를 감안한 허용 오차. 간격의 1%면 인접 선과 헷갈릴 일이 없다.
	const float AxisTolerance = Grid.Interval * 0.01f;

	for (uint32 i = 0; i < Grid.LineNum; ++i)
	{
		const float Offset = i * Grid.Interval;
		const float X = MinX + Offset;
		const float Y = MinY + Offset;

		const float Offset2 = Half - Offset;
		const float Dist = sqrt(Offset2 * Offset2 + Half * Half);
		const float Alpha = (std::min)(2.0f / Dist, 1.0f);
		const float Alpha2 = (std::min)(2.0f / fabsf(Offset2), 1.0f);

		// X가 고정이고 Y가 변하는 선 = Y축과 평행. X == 0이면 그게 Y축이므로 건너뛰고,
		// 아래에서 원점 기준으로 따로 그린다. (Y가 고정인 선은 그 반대)
		if (fabs(X) > AxisTolerance)
		{
			AddLine({ X, MinY,          0.0f, GridColor.X, GridColor.Y, GridColor.Z, Alpha },
				{ X, MinY + Half, 0.0f, GridColor.X, GridColor.Y, GridColor.Z, Alpha2 });
			AddLine({ X, MinY + Half,          0.0f, GridColor.X, GridColor.Y, GridColor.Z, Alpha2 },
				{ X, MinY + Length, 0.0f, GridColor.X, GridColor.Y, GridColor.Z, Alpha });
		}

		if (fabs(Y) > AxisTolerance)
		{
			AddLine({ MinX,          Y, 0.0f, GridColor.X, GridColor.Y, GridColor.Z, Alpha },
				{ MinX + Half, Y, 0.0f, GridColor.X, GridColor.Y, GridColor.Z, Alpha2 });
			AddLine({ MinX + Half,          Y, 0.0f, GridColor.X, GridColor.Y, GridColor.Z, Alpha2 },
				{ MinX + Length, Y, 0.0f, GridColor.X, GridColor.Y, GridColor.Z, Alpha });
		}
	}

	// 축은 그리드와 달리 카메라를 따라가지 않고 항상 원점에 고정한다.
	// 다만 위 루프에서 건너뛴 선을 대신 메워야 하므로 패치 끝까지 늘리고,
	// 원점이 패치 밖에 있어도 원점까지는 닿게 한다.
	// 양 끝 정점의 색이 같아야 보간되지 않고 단색으로 나온다.
	const float AxisMinX = MinX;
	const float AxisMaxX = MinX + Length;
	const float AxisMinY = MinY;
	const float AxisMaxY = MinY + Length;
	const float AxisMinZ = CameraPos.Z - Length;
	const float AxisMaxZ = CameraPos.Z + Length;

	AddLine({ AxisMinX, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ AxisMaxX, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f });
	AddLine({ 0.0f, AxisMinY, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f },
		{ 0.0f, AxisMaxY, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f });
	AddLine({ 0.0f, 0.0f, AxisMinZ, 0.0f, 0.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f,  AxisMaxZ, 0.0f, 0.0f, 1.0f, 1.0f });
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
