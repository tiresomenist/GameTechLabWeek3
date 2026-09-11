#include "pch.h"
#include "UTextComponent.h"
#include "Engine/Object/FArchive.h"

void UTextComponent::SetText(const FString& InText)
{
	if (Text == InText) return;
	Text = InText;
	bMeshDirty = true;
}

void UTextComponent::SetFontName(const FString& InFontName)
{
	if (FontName == InFontName) return;
	FontName = InFontName;
	bMeshDirty = true;
}

void UTextComponent::SetSize(float InSize)
{
	if (Size == InSize) return;
	Size = InSize;
	bMeshDirty = true;
}

void UTextComponent::SetColor(const FVector4& InColor)
{
	// FVector4에는 == 연산자가 없어서 성분별로 비교한다
	if (Color.X == InColor.X && Color.Y == InColor.Y && Color.Z == InColor.Z && Color.W == InColor.W) return;
	Color = InColor;
	bMeshDirty = true;   // 색은 정점에 들어가므로 색만 바뀌어도 메시를 다시 만든다
}

void UTextComponent::SetAlign(ETextAlign InAlign)
{
	if (Align == InAlign) return;
	Align = InAlign;
	bMeshDirty = true;
}

FTextStyle UTextComponent::MakeStyle() const
{
	FTextStyle Style;
	Style.Size = Size;
	Style.Color = Color;
	Style.Align = Align;
	return Style;
}

FPrimitiveRenderData UTextComponent::CreateRenderData(bool bSelected) const
{
	FPrimitiveRenderData Out = {};
	FFontAtlas* Font = GResourceManager::GetInstance()->GetFont(FontName);

	if (!Font)
	{
		return Out;
	}

	if (bMeshDirty)
	{
		Mesh.Build(*Font, Text, MakeStyle());
		GDevice* Device = GDevice::GetInstance();

		if (!Mesh.Upload(Device->GetDevice(), Device->GetContext()))
		{
			bMeshDirty = true;
			return Out;
		}

		bMeshDirty = false;
	}

	Out.VertexBuffer = Mesh.GetVertexBuffer();
	Out.IndexBuffer = Mesh.GetIndexBuffer();
	Out.IndexCount = Mesh.GetIndexCount();
	Out.Stride = sizeof(FVertexText);
	Out.WorldMatrix = &GetWorldMatrix();
	Out.isSelected = bSelected;
	Out.Material = Font->GetTexture().GetSRV();
	Out.Pass = ERenderPass::Text;

	return Out;
}

bool UTextComponent::GetLocalBounds(FVector& OutMin, FVector& OutMax) const
{
	// 메시는 렌더 때(CreateRenderData) dirty면 다시 만들어진다.
	// 문구를 바꾼 바로 그 프레임에 렌더 전에 클릭하면 이전 문구의 Bounds로 검사한다 (한 프레임 지연).
	if (!Mesh.HasBounds()) return false;

	OutMin = Mesh.GetBoundsMin();
	OutMax = Mesh.GetBoundsMax();
	return true;
}

void UTextComponent::Serialize(FArchive& Archive)
{
	Super::Serialize(Archive);   // Type, Location, Rotation, Scale

	Archive.SetString("Text", Text);
	Archive.SetString("Font", FontName);   // 포인터가 아니라 이름으로 저장. 로드할 때 GResourceManager에서 다시 빌린다
	Archive.SetFloat("Size", Size);
	Archive.SetArray<float>("Color", TArray<float>{ Color.X, Color.Y, Color.Z, Color.W });
	Archive.SetInt32("Align", static_cast<int32>(Align));
}

void UTextComponent::Deserialize(FArchive& Archive)
{
	Super::Deserialize(Archive);

	// 키가 없거나 값이 이상하면 기본값을 유지한다.
	// FArchive의 Get은 키가 없으면 예외를 던지므로, 필드가 나중에 추가됐거나
	// 사람이 고친 파일 때문에 씬 전체가 로드 실패하지 않도록 먼저 확인한다.
	// 값은 멤버에 직접 넣지 않고 Setter를 거쳐 메시가 다시 만들어지게 한다.
	if (Archive.Contains("Text")) SetText(Archive.GetString("Text"));
	if (Archive.Contains("Font")) SetFontName(Archive.GetString("Font"));

	if (Archive.Contains("Size"))
	{
		const float InSize = Archive.GetFloat("Size");
		if (std::isfinite(InSize) && InSize > 0.0f) SetSize(InSize);
	}

	if (Archive.Contains("Color"))
	{
		const TArray<float> InColor = Archive.GetArray<float>("Color");
		if (InColor.Num() == 4) SetColor(FVector4(InColor[0], InColor[1], InColor[2], InColor[3]));
	}

	if (Archive.Contains("Align"))
	{
		const int32 InAlign = Archive.GetInt32("Align");
		if (InAlign >= static_cast<int32>(ETextAlign::Left) && InAlign <= static_cast<int32>(ETextAlign::Right))
			SetAlign(static_cast<ETextAlign>(InAlign));
	}
}
