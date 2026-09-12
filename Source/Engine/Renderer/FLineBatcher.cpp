#include <pch.h>
#include "FLineBatcher.h"


void FLineBatcher::Initialize(ID3D11Device* Device)
{
	if (!Device) return;

	D3D11_BUFFER_DESC VbDesc = {};
	VbDesc.Usage = D3D11_USAGE_DYNAMIC;
	VbDesc.ByteWidth = sizeof(FVertexSimple) * MaxVertices;
	VbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	VbDesc.MiscFlags = 0;
	VbDesc.StructureByteStride = 0;

	HRESULT Hr = Device->CreateBuffer(&VbDesc, nullptr, &VertexBuffer);

	if (FAILED(Hr)) return;

	D3D11_BUFFER_DESC IbDesc = {};
	IbDesc.Usage = D3D11_USAGE_DYNAMIC;
	IbDesc.ByteWidth = sizeof(uint32) * MaxIndices;
	IbDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	IbDesc.MiscFlags = 0;
	IbDesc.StructureByteStride = 0;

	Hr = Device->CreateBuffer(&IbDesc, nullptr, &IndexBuffer);

	if (FAILED(Hr)) return;

	Vertices.reserve(MaxVertices);
	Indices.reserve(MaxIndices);
}

void FLineBatcher::AddLine(const FVertexSimple& Start, const FVertexSimple& End)
{
	uint32 StartIndex = static_cast<uint32>(Vertices.Num());

	FVertexSimple StartVertex = Start;
	FVertexSimple EndVertex = End;

	Vertices.Add(StartVertex);
	Vertices.Add(EndVertex);

	Indices.Add(StartIndex);
	Indices.Add(StartIndex + 1);
}

void FLineBatcher::AddBoundingBox(const FVector& Max, const FVector& Min, const FMatrix& WorldMatrix, const FVector4& Color)
{
	uint32 StartIndex = static_cast<uint32>(Vertices.Num());

	FVector LocalPoints[8] = {
		FVector(Min.X, Min.Y, Min.Z),
		FVector(Max.X, Min.Y, Min.Z),
		FVector(Max.X, Max.Y, Min.Z),
		FVector(Min.X, Max.Y, Min.Z),
		FVector(Min.X, Min.Y, Max.Z),
		FVector(Max.X, Min.Y, Max.Z),
		FVector(Max.X, Max.Y, Max.Z),
		FVector(Min.X, Max.Y, Max.Z)
	};

	for (int i = 0;i < 8;++i)
	{
		FVector WorldPoint = WorldMatrix.TransformPosition(LocalPoints[i]);

		Vertices.Add(FVertexSimple(
			WorldPoint.X, WorldPoint.Y, WorldPoint.Z,
			Color.X, Color.Y, Color.Z, Color.W
		));
	}

	Indices.Add(StartIndex + 0); Indices.Add(StartIndex + 1);
	Indices.Add(StartIndex + 1); Indices.Add(StartIndex + 2);
	Indices.Add(StartIndex + 2); Indices.Add(StartIndex + 3);
	Indices.Add(StartIndex + 3); Indices.Add(StartIndex + 0);

	Indices.Add(StartIndex + 4); Indices.Add(StartIndex + 5);
	Indices.Add(StartIndex + 5); Indices.Add(StartIndex + 6);
	Indices.Add(StartIndex + 6); Indices.Add(StartIndex + 7);
	Indices.Add(StartIndex + 7); Indices.Add(StartIndex + 4);

	Indices.Add(StartIndex + 0); Indices.Add(StartIndex + 4);
	Indices.Add(StartIndex + 1); Indices.Add(StartIndex + 5);
	Indices.Add(StartIndex + 2); Indices.Add(StartIndex + 6);
	Indices.Add(StartIndex + 3); Indices.Add(StartIndex + 7);
}

void FLineBatcher::AddWorldGizmo(float GridSize, float Step)
{
	if (GridSize <= 0 and Step <= 0) return;

	float HalfSize = GridSize * 0.5f;

	const FVector4 XAxisColor(1.0f, 0.2f, 0.2f, 1.0f);
	const FVector4 YAxisColor(0.2f, 1.0f, 0.2f, 1.0f);
	const FVector4 ZAxisColor(0.2f, 0.2f, 1.0f, 1.0f);
	const FVector4 GridColor(0.25f, 0.25f, 0.25f, 1.0f);

	// X축
	FVertexSimple XAxisStart = FVertexSimple(0.0f, 0.0f, 0.0f, XAxisColor.X, XAxisColor.Y, XAxisColor.Z, XAxisColor.W);
	FVertexSimple XAxisEnd = FVertexSimple(HalfSize, 0.0f, 0.0f, XAxisColor.X, XAxisColor.Y, XAxisColor.Z, XAxisColor.W);
	FLineBatcher::AddLine(XAxisStart, XAxisEnd);

	XAxisStart = FVertexSimple(0.0f, 0.0f, 0.0f, GridColor.X, GridColor.Y, GridColor.Z, GridColor.W);
	XAxisEnd = FVertexSimple(-HalfSize, 0.0f, 0.0f, GridColor.X, GridColor.Y, GridColor.Z, GridColor.W);
	FLineBatcher::AddLine(XAxisStart, XAxisEnd);

	// Y축
	FVertexSimple YAxisStart = FVertexSimple(0.0f, 0.0f, 0.0f, YAxisColor.X, YAxisColor.Y, YAxisColor.Z, YAxisColor.W);
	FVertexSimple YAxisEnd = FVertexSimple(0.0f, HalfSize, 0.0f, YAxisColor.X, YAxisColor.Y, YAxisColor.Z, YAxisColor.W);
	FLineBatcher::AddLine(YAxisStart, YAxisEnd);

	YAxisStart = FVertexSimple(0.0f, 0.0f, 0.0f, GridColor.X, GridColor.Y, GridColor.Z, GridColor.W);
	YAxisEnd = FVertexSimple(0.0f, -HalfSize, 0.0f, GridColor.X, GridColor.Y, GridColor.Z, GridColor.W);
	FLineBatcher::AddLine(YAxisStart, YAxisEnd);

	// Z축
	FVertexSimple ZAxisStart = FVertexSimple(0.0f, 0.0f, 0.0f, ZAxisColor.X, ZAxisColor.Y, ZAxisColor.Z, ZAxisColor.W);
	FVertexSimple ZAxisEnd = FVertexSimple(0.0f, 0.0f, HalfSize, ZAxisColor.X, ZAxisColor.Y, ZAxisColor.Z, ZAxisColor.W);
	FLineBatcher::AddLine(ZAxisStart, ZAxisEnd);
}

void FLineBatcher::AddGrid(FVector CamPos, float GridSize, float Step)
{
	if (GridSize <= 0 and Step <= 0) return;

	FLineBatcher::AddWorldGizmo(GridSize + CamPos.Distance(FVector(0, 0, 0)), Step);

	uint32 StartIndex = static_cast<uint32>(Vertices.Num());

	float HalfSize = GridSize * 0.5f;

	float SnapX = std::floor(CamPos.X / Step) * Step;
	float SnapY = std::floor(CamPos.Y / Step) * Step;

	const FVector4 GridColor(0.25f, 0.25f, 0.25f, 1.0f);

	for (float Pos = -HalfSize; Pos <= +HalfSize; Pos += Step)
	{
		if (fabs(Pos) < 0.001f) continue;

		FLineBatcher::AddLine(FVertexSimple(
			SnapX - HalfSize, SnapY + Pos, 0.0f,
			GridColor.X, GridColor.Y, GridColor.Z, GridColor.W
		), FVertexSimple(
			SnapX + HalfSize, SnapY + Pos, 0.0f,
			GridColor.X, GridColor.Y, GridColor.Z, GridColor.W
		));
		FLineBatcher::AddLine(FVertexSimple(
			SnapX + Pos, SnapY - HalfSize, 0.0f,
			GridColor.X, GridColor.Y, GridColor.Z, GridColor.W
		), FVertexSimple(
			SnapX + Pos, SnapY + HalfSize, 0.0f,
			GridColor.X, GridColor.Y, GridColor.Z, GridColor.W
		));
	}
}

UINT FLineBatcher::UpdateBuffers(ID3D11DeviceContext* Context)
{
	if (!Context || Vertices.Num() == 0 || Indices.Num() == 0)
		return 0;

	const UINT IndexCount = static_cast<UINT>(Indices.Num());

	D3D11_MAPPED_SUBRESOURCE VbMapped{};
	if (SUCCEEDED(Context->Map(VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &VbMapped)))
	{
		memcpy(VbMapped.pData, Vertices.GetData(), sizeof(FVertexSimple) * Vertices.Num());
		Context->Unmap(VertexBuffer, 0);
	}

	D3D11_MAPPED_SUBRESOURCE IbMapped{};
	if (SUCCEEDED(Context->Map(IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &IbMapped)))
	{
		memcpy(IbMapped.pData, Indices.GetData(), sizeof(uint32) * IndexCount);
		Context->Unmap(IndexBuffer, 0);
	}

	Vertices.Empty();
	Indices.Empty();

	return IndexCount;
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
}