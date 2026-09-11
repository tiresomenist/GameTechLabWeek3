#include "pch.h"
#include "FTextMesh.h"
#include "Core/Container/Utf8.h"

#include <cstring>

namespace
{
	// CPU가 나중에 다시 쓸 수 있는 동적 버퍼. 초기 데이터 없이 용량만 잡는다.
	ID3D11Buffer* CreateDynamicBuffer(ID3D11Device* Device, UINT ByteWidth, UINT BindFlags)
	{
		D3D11_BUFFER_DESC Desc = {};
		Desc.ByteWidth = ByteWidth;
		Desc.Usage = D3D11_USAGE_DYNAMIC;
		Desc.BindFlags = BindFlags;
		Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		ID3D11Buffer* Buffer = nullptr;
		if (FAILED(Device->CreateBuffer(&Desc, nullptr, &Buffer))) return nullptr;
		return Buffer;
	}

	// WRITE_DISCARD: 이전 내용은 버리고 새로 쓴다.
	// GPU가 이전 내용으로 그리는 중이어도 드라이버가 새 메모리를 줘서 기다리지 않는다.
	bool WriteBuffer(ID3D11DeviceContext* Context, ID3D11Buffer* Buffer, const void* Data, size_t Bytes)
	{
		D3D11_MAPPED_SUBRESOURCE Mapped = {};
		if (FAILED(Context->Map(Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped))) return false;
		std::memcpy(Mapped.pData, Data, Bytes);
		Context->Unmap(Buffer, 0);
		return true;
	}

	float LineOffset(float LineWidth, ETextAlign Align)
	{
		switch (Align)
		{
		case ETextAlign::Left:
			return 0.0f;
		case ETextAlign::Center:
			return -LineWidth / 2.0f;
		case ETextAlign::Right:
			return -LineWidth;
		default:
			return 0.0f;
		}
	}
}

void FTextMesh::Build(const FFontAtlas& Font, const FString& Text, const FTextStyle& Style)
{
	Vertices.Empty();
	Indices.Empty();
	bHasBounds = false;   // 빈 문자열이면 이전 문구의 Bounds가 남지 않게
	if (Text.empty()) return;

	const TArray<uint32> Codepoints = Utf8::Decode(Text);
	float Scale = Style.Size / Font.GetBakedPixelHeight();

	TArray<float> LineWidth;
	float W = 0;
	for (uint32 Cp : Codepoints)
	{
		if (Cp == '\n')
		{
			LineWidth.Add(W);
			W = 0.0f;
			continue;
		}
		else if (Cp == '\r')
		{
			continue;
		}

		const FGlyph* Glyph = Font.FindGlyph(Cp);
		if (!Glyph) Glyph = Font.FindGlyph('?');
		if (!Glyph) continue;

		W += Glyph->XAdvance;
	}
	LineWidth.Add(W);

	UINT Line = 0;
	float PenX = LineOffset(LineWidth[0], Style.Align);
	float PenY = Font.GetAscent();

	for (uint32 Cp : Codepoints)
	{
		if (Cp == '\n')
		{
			Line++;
			PenX = LineOffset(LineWidth[Line], Style.Align);
			PenY += Font.GetLineHeight();
			continue;
		}
		else if (Cp == '\r')

		{
			continue;
		}

		const FGlyph* Glyph = Font.FindGlyph(Cp);
		if (!Glyph) Glyph = Font.FindGlyph('?');
		if (!Glyph) continue;

		if (Glyph->XOff2 - Glyph->XOff == 0.0f || Glyph->YOff2 - Glyph->YOff == 0.0f)
		{
			PenX += Glyph->XAdvance;
			continue;
		}

		float X0 = PenX + Glyph->XOff;
		float X1 = PenX + Glyph->XOff2;
		float Y0 = PenY + Glyph->YOff;
		float Y1 = PenY + Glyph->YOff2;

		// 여기까지는 stb 레이아웃 좌표 (px, Y 아래로 +)
		// ↓ 월드로 변환: 스케일 적용 + Y 뒤집기 (이 한 곳에서만)

		FVertexText p0;
		p0.x = 0.0f * Scale;
		p0.y = X0 * Scale;
		p0.z = -Y0 * Scale;
		p0.u = Glyph->U0;
		p0.v = Glyph->V0;
		p0.r = Style.Color.X;
		p0.g = Style.Color.Y;
		p0.b = Style.Color.Z;
		p0.a = Style.Color.W;

		FVertexText p1;
		p1.x = 0.0f * Scale;
		p1.y = X1 * Scale;
		p1.z = -Y0 * Scale;
		p1.u = Glyph->U1;
		p1.v = Glyph->V0;
		p1.r = Style.Color.X;
		p1.g = Style.Color.Y;
		p1.b = Style.Color.Z;
		p1.a = Style.Color.W;

		FVertexText p2;
		p2.x = 0.0f * Scale;
		p2.y = X0 * Scale;
		p2.z = -Y1 * Scale;
		p2.u = Glyph->U0;
		p2.v = Glyph->V1;
		p2.r = Style.Color.X;
		p2.g = Style.Color.Y;
		p2.b = Style.Color.Z;
		p2.a = Style.Color.W;

		FVertexText p3;
		p3.x = 0.0f * Scale;
		p3.y = X1 * Scale;
		p3.z = -Y1 * Scale;
		p3.u = Glyph->U1;
		p3.v = Glyph->V1;
		p3.r = Style.Color.X;
		p3.g = Style.Color.Y;
		p3.b = Style.Color.Z;
		p3.a = Style.Color.W;

		// emplace_back ?
		int n = Vertices.Num();

		Vertices.Add(p0);
		Vertices.Add(p1);
		Vertices.Add(p2);
		Vertices.Add(p3);

		// 로컬 Bounds 갱신 (피킹용). 쿼드는 YZ 평면의 사각형이라 p0(좌상단)과 p3(우하단)이면 충분하다
		const FVector QuadMin(0.0f, p0.y, p3.z);
		const FVector QuadMax(0.0f, p3.y, p0.z);
		if (!bHasBounds)
		{
			BoundsMin = QuadMin;
			BoundsMax = QuadMax;
			bHasBounds = true;
		}
		else
		{
			BoundsMin.Y = (std::min)(BoundsMin.Y, QuadMin.Y);
			BoundsMin.Z = (std::min)(BoundsMin.Z, QuadMin.Z);
			BoundsMax.Y = (std::max)(BoundsMax.Y, QuadMax.Y);
			BoundsMax.Z = (std::max)(BoundsMax.Z, QuadMax.Z);
		}

		Indices.Add(n + 0);
		Indices.Add(n + 1);
		Indices.Add(n + 2);
		Indices.Add(n + 2);
		Indices.Add(n + 1);
		Indices.Add(n + 3);

		PenX += Glyph->XAdvance;
	}
}

bool FTextMesh::Upload(ID3D11Device* Device, ID3D11DeviceContext* Context)
{
	// 실패하면 그리지 않도록, 성공한 뒤에만 IndexCount를 채운다
	IndexCount = 0;
	if (!Device || !Context) return false;

	// 그릴 게 없음 (빈 문자열, 공백만). ByteWidth 0인 버퍼는 만들 수 없다.
	if (Indices.Num() == 0) return true;

	const UINT VertexNum = static_cast<UINT>(Vertices.Num());
	const UINT IndexNum = static_cast<UINT>(Indices.Num());

	// 용량이 모자랄 때만 다시 만든다. 두 배씩 늘려서 한 글자씩 늘 때마다 재생성하지 않게 한다.
	if (VertexNum > VBCapacity)
	{
		if (VB) { VB->Release(); VB = nullptr; }
		const UINT NewCapacity = (std::max)(VertexNum, VBCapacity * 2);
		VB = CreateDynamicBuffer(Device, NewCapacity * sizeof(FVertexText), D3D11_BIND_VERTEX_BUFFER);
		VBCapacity = VB ? NewCapacity : 0;
		if (!VB) return false;
	}

	if (IndexNum > IBCapacity)
	{
		if (IB) { IB->Release(); IB = nullptr; }
		const UINT NewCapacity = (std::max)(IndexNum, IBCapacity * 2);
		IB = CreateDynamicBuffer(Device, NewCapacity * sizeof(uint32), D3D11_BIND_INDEX_BUFFER);
		IBCapacity = IB ? NewCapacity : 0;
		if (!IB) return false;
	}

	// 버퍼는 용량만큼 크지만, 실제 개수만큼만 채우고 그만큼만 그린다
	if (!WriteBuffer(Context, VB, Vertices.GetData(), VertexNum * sizeof(FVertexText))) return false;
	if (!WriteBuffer(Context, IB, Indices.GetData(), IndexNum * sizeof(uint32))) return false;

	IndexCount = IndexNum;
	return true;
}

void FTextMesh::Release()
{
	if (VB)
	{
		VB->Release();
		VB = nullptr;
	}
	if (IB)
	{
		IB->Release();
		IB = nullptr;
	}
	VBCapacity = 0;
	IBCapacity = 0;
	IndexCount = 0;
}
