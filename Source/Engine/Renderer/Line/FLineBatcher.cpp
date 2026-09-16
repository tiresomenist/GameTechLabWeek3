#include "pch.h"
#include "Engine/Renderer/GDevice.h"
#include "FLineBatcher.h"
#include "Engine/Log.h"
#include <cmath>
#include <limits>

namespace
{
	bool ReserveLineBuffer(ID3D11Buffer*& Buffer,size_t& Capacity,size_t Need,UINT ElementSize,UINT BindFlags)
	{
		if (Buffer && Need <= Capacity)
		{
			return true;
		}

		const size_t MaxCount =	static_cast<size_t>((std::numeric_limits<UINT>::max)()) / ElementSize;
		
		if (Need == 0 || Need > MaxCount)
		{
			return false;
		}

		const size_t GrownCapacity = Capacity > MaxCount / 2 ? MaxCount : Capacity * 2;

		const size_t NewCapacity = (std::max)(Need, GrownCapacity);

		D3D11_BUFFER_DESC Desc{};
		Desc.ByteWidth = static_cast<UINT>(NewCapacity * ElementSize);
		Desc.Usage = D3D11_USAGE_DYNAMIC;
		Desc.BindFlags = BindFlags;
		Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		ID3D11Buffer* NewBuffer = nullptr;

		const HRESULT Result = GDevice::GetInstance()->GetDevice()->CreateBuffer(&Desc, nullptr, &NewBuffer);
		if (FAILED(Result))
		{
			return false;
		}
		if (Buffer)
		{
			Buffer->Release();
		}

		Buffer = NewBuffer;
		Capacity = NewCapacity;
		return true;
	}
}

//void FLineBatcher::AddLine(const FVertexSimple& A, const FVertexSimple& B)
//{
//	const uint32 N = static_cast<uint32>(Vertices.Num());
//	Vertices.Add(A);
//	Vertices.Add(B);
//	Indexes.Add(N);
//	Indexes.Add(N + 1);
//}
//
//void FLineBatcher::AddBoundBox(const FVector& Min, const FVector& Max, const FMatrix& World)
//{
//	const FVector4 BoxColor(1.0f, 1.0f, 0.0f, 1.0f);
//	const uint32 Base = static_cast<uint32>(Vertices.Num());
//
//	// 코너 인덱스의 비트 0/1/2 = X/Y/Z가 Min인지 Max인지.
//	// 로컬에서 코너를 조합한 뒤에 World로 옮겨야 회전한 물체에서도 맞는다.
//	for (int i = 0; i < 8; ++i)
//	{
//		const FVector4 Local(
//			(i & 1) ? Max.X : Min.X,
//			(i & 2) ? Max.Y : Min.Y,
//			(i & 4) ? Max.Z : Min.Z,
//			1.0f);
//
//		const FVector Corner = (Local * World).getXYZ();
//		Vertices.Add({ Corner.X, Corner.Y, Corner.Z, BoxColor.X, BoxColor.Y, BoxColor.Z, BoxColor.W });
//	}
//
//	// 두 코너가 비트 하나만 다르면 그게 엣지. 정점 8개를 12개 엣지가 공유한다
//	for (uint32 i = 0; i < 8; ++i)
//	{
//		for (uint32 Bit = 1; Bit <= 4; Bit <<= 1)
//		{
//			if (!(i & Bit))
//			{
//				Indexes.Add(Base + i);
//				Indexes.Add(Base + (i | Bit));
//			}
//		}
//	}
//}
//
//void FLineBatcher::AddWorldAxis(FGrid Grid, FVector CameraPos) {
//	// 축은 그리드와 달리 카메라를 따라가지 않고 항상 원점에 고정한다.
//	// 위 루프에서 건너뛴 선을 대신 메우도록 패치 끝까지 그린다.
//	// 양 끝 정점의 색이 같아야 보간되지 않고 단색으로 나온다.
//
//	const float Half = Grid.Extent;
//	const float Length = Half * 2.0f;
//	const double Interval = Grid.Interval;
//	const double CenterX = std::floor(static_cast<double>(CameraPos.X) / Interval) * Interval;
//	const double CenterY = std::floor(static_cast<double>(CameraPos.Y) / Interval) * Interval;
//	const float MinX = static_cast<float>(CenterX - Half);
//	const float MinY = static_cast<float>(CenterY - Half);
//	const float MaxX = static_cast<float>(CenterX + Half);
//	const float MaxY = static_cast<float>(CenterY + Half);
//
//	const float AxisMinX = MinX;
//	const float AxisMaxX = MaxX;
//	const float AxisMinY = MinY;
//	const float AxisMaxY = MaxY;
//	const float AxisMinZ = CameraPos.Z - Length;
//	const float AxisMaxZ = CameraPos.Z + Length;
//
//	AddLine({ AxisMinX, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f },
//		{ AxisMaxX, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f });
//	AddLine({ 0.0f, AxisMinY, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f },
//		{ 0.0f, AxisMaxY, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f });
//	AddLine({ 0.0f, 0.0f, AxisMinZ, 0.0f, 0.0f, 1.0f, 1.0f },
//		{ 0.0f, 0.0f,  AxisMaxZ, 0.0f, 0.0f, 1.0f, 1.0f });
//}
//
//void FLineBatcher::AddGrid(FGrid Grid, FVector CameraPos)
//{
//	if (!std::isfinite(Grid.Interval) || Grid.Interval < FGrid::MinInterval ||
//		!std::isfinite(Grid.Extent) || Grid.Extent <= 0.0f ||
//		!std::isfinite(CameraPos.X) || !std::isfinite(CameraPos.Y) || !std::isfinite(CameraPos.Z))
//	{
//		return;
//	}
//
//	const FVector GridColor(1.0f, 1.0f, 1.0f);
//	const float Half = Grid.Extent;
//	const float Length = Half * 2.0f;
//	const double Interval = Grid.Interval;
//	const double CenterX = std::floor(static_cast<double>(CameraPos.X) / Interval) * Interval;
//	const double CenterY = std::floor(static_cast<double>(CameraPos.Y) / Interval) * Interval;
//	const float MinX = static_cast<float>(CenterX - Half);
//	const float MinY = static_cast<float>(CenterY - Half);
//	const float MaxX = static_cast<float>(CenterX + Half);
//	const float MaxY = static_cast<float>(CenterY + Half);
//
//	// Fixed world bounds, with only the number of grid lines changing with spacing.
//	const double FirstX = std::ceil((CenterX - Half) / Interval);
//	const double FirstY = std::ceil((CenterY - Half) / Interval);
//	const double CountX = std::floor((CenterX + Half) / Interval) - FirstX + 1.0;
//	const double CountY = std::floor((CenterY + Half) / Interval) - FirstY + 1.0;
//	// Each coordinate produces at most two segments (four vertices and indices).
//	const double MaxVertices = (std::numeric_limits<UINT>::max)() / sizeof(FVertexSimple);
//	if (!std::isfinite(Length) || CountX < 0 || CountY < 0 ||
//		4.0 * (CountX + CountY) + 6.0 + Vertices.Num() > MaxVertices)
//	{
//		return;
//	}
//
//	// 부동소수 누적 오차를 감안한 허용 오차. 간격의 1%면 인접 선과 헷갈릴 일이 없다.
//	const float AxisTolerance = Grid.Interval * 0.01f;
//
//	const auto GetAlpha = [Half](double Distance)
//	{
//		double FadeEnd = Half;
//		double FadeStart = Half * 0.7;
//		double T = std::clamp((Distance - FadeStart) / (FadeEnd - FadeStart),0.0, 1.0);
//		const double Fade = T * T * (3.0 - 2.0 * T);
//		return static_cast<float>(1.0 - Fade);
//	};
//
//	for (uint32 i = 0; i < static_cast<uint32>(CountX); ++i)
//	{
//		const float X = static_cast<float>((FirstX + i) * Interval);
//		const double Offset = X - CenterX;
//		const float Alpha = GetAlpha(std::hypot(Offset, Half));
//		const float Alpha2 = GetAlpha(std::abs(Offset));
//		// The colored world axes replace the grid lines at X == 0 / Y == 0.
//		if (std::abs(X) > AxisTolerance)
//		{
//			AddLine({ X, MinY,          0.0f, GridColor.X, GridColor.Y, GridColor.Z, Alpha },
//				{ X, static_cast<float>(CenterY), 0.0f, GridColor.X, GridColor.Y, GridColor.Z, Alpha2 });
//			AddLine({ X, static_cast<float>(CenterY), 0.0f, GridColor.X, GridColor.Y, GridColor.Z, Alpha2 },
//				{ X, MaxY, 0.0f, GridColor.X, GridColor.Y, GridColor.Z, Alpha });
//		}
//	}
//
//	for (uint32 i = 0; i < static_cast<uint32>(CountY); ++i)
//	{
//		const float Y = static_cast<float>((FirstY + i) * Interval);
//		const double Offset = Y - CenterY;
//		const float Alpha = GetAlpha(std::hypot(Offset, Half));
//		const float Alpha2 = GetAlpha(std::abs(Offset));
//		if (std::abs(Y) > AxisTolerance)
//		{
//			AddLine({ MinX,          Y, 0.0f, GridColor.X, GridColor.Y, GridColor.Z, Alpha },
//				{ static_cast<float>(CenterX), Y, 0.0f, GridColor.X, GridColor.Y, GridColor.Z, Alpha2 });
//			AddLine({ static_cast<float>(CenterX), Y, 0.0f, GridColor.X, GridColor.Y, GridColor.Z, Alpha2 },
//				{ MaxX, Y, 0.0f, GridColor.X, GridColor.Y, GridColor.Z, Alpha });
//		}
//	}
//}

bool FLineBatcher::AddRequest(const FLineDrawRequest& Request)
{
	const size_t AddedVertexCount = static_cast<size_t>(Request.Vertices.Num());

	const size_t AddedIndexCount = static_cast<size_t>(Request.Indices.Num());

	if (AddedVertexCount == 0 && AddedIndexCount == 0) return true;
	if (AddedVertexCount == 0 || AddedIndexCount == 0 || AddedIndexCount % 2 != 0) {
		UE_LOG("선을 그리는 인덱스의 개수가 짝수가 아닙니다.\n");
		return false;
	}
	const size_t MaxVertices = (std::min)(static_cast<size_t>((std::numeric_limits<int>::max)()),static_cast<size_t>((std::numeric_limits<UINT>::max)())/sizeof(FVertexSimple));
	const size_t MaxIndices = (std::min)(static_cast<size_t>((std::numeric_limits<int>::max)()),static_cast<size_t>((std::numeric_limits<UINT>::max)())/ sizeof(uint32));

	const size_t CurrentVertexCount = static_cast<size_t>(Vertices.Num());
	const size_t CurrentIndexCount = static_cast<size_t>(Indices.Num());

	if (CurrentVertexCount > MaxVertices || AddedVertexCount > MaxVertices - CurrentVertexCount ||
		CurrentIndexCount > MaxIndices || AddedIndexCount > MaxIndices - CurrentIndexCount)
	{
		UE_LOG("한번에 너무 많은 라인 드로우 요청\n");
		return false;
	}

	for (uint32 Index : Request.Indices){
		if (Index >= AddedVertexCount){
			UE_LOG("인덱스가 잘못된 버텍스를 가리키고 있습니다.\n");
			return false;
		}
	}
	for (const FVertexSimple& Vertex : Request.Vertices)
	{
		if (!std::isfinite(Vertex.x) ||	!std::isfinite(Vertex.y) ||	!std::isfinite(Vertex.z) ||
			!std::isfinite(Vertex.r) ||	!std::isfinite(Vertex.g) ||	!std::isfinite(Vertex.b) ||
			!std::isfinite(Vertex.a))
		{
			UE_LOG("잘못된 버텍스 정보\n");
			return false;
		}
	}
	try
	{
		// 두 배열의 공간을 먼저 확보하여 부분 추가를 방지함
		Vertices.Reserve(CurrentVertexCount + AddedVertexCount);
		Indices.Reserve(CurrentIndexCount + AddedIndexCount);
	}
		catch (const std::bad_alloc&)
	{
			UE_LOG("라인 배처 잘못된 메모리 할당\n");
		return false;
	}

	const uint32 VertexBase = static_cast<uint32>(CurrentVertexCount);

	for (const FVertexSimple& Vertex : Request.Vertices)
	{
		Vertices.Add(Vertex);
	}

	for (uint32 Index : Request.Indices)
	{
		// 요청 내부 번호를 배치 전체 번호로 변환함
		Indices.Add(VertexBase + Index);
	}
	return true;
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

	size_t M = Indices.Num();

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
	memcpy(Mapped.pData, Indices.GetData(), sizeof(uint32) * M);
	Context->Unmap(IndexBuffer, 0);

	return true;
}

void FLineBatcher::Clear()
{
	Vertices.Empty();
	Indices.Empty();
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
	return static_cast<UINT>(Indices.Num());
}

bool FLineBatcher::ReserveVertexBuffer(size_t Need)
{
	return ReserveLineBuffer(VertexBuffer, VertexBufferCapacity, Need, sizeof(FVertexSimple), D3D11_BIND_VERTEX_BUFFER);
}

bool FLineBatcher::ReserveIndexBuffer(size_t Need)
{
	return ReserveLineBuffer(IndexBuffer,IndexBufferCapacity,Need,sizeof(uint32),D3D11_BIND_INDEX_BUFFER);
}
