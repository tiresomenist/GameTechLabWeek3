#include "pch.h"
#include "Engine/Component/Light/USpotLightComponent.h"

#include "Core/Container/TArray.h"
#include "Engine/Object/FArchive.h"
#include "Engine/Resource/GResourceManager.h"
#include "Engine/Resource/FTextureResource.h"
#include "Engine/Component/UCameraComponent.h"
#include "Engine/Resource/FMeshResource.h"

void USpotLightComponent::Initialize()
{
    Super::Initialize();

    GResourceManager* Resources = GResourceManager::GetInstance();

    // 초기화 시 등록한 공유 아이콘 메시 조회함
    IconMesh = Resources->GetPrimitive("SpotLightIcon");

    if (!IconMesh)
    {
        throw std::runtime_error("SpotLight icon mesh is not registered");
    }

    // 최초 요청 시 로드하고 이후 요청에서는 캐시된 텍스처를 공유함
    IconTexture = Resources->GetOrLoadTexture("Assets/Textures/SpotLightIcon.png");

    if (!IconTexture || !IconTexture->GetSRV())
    {
        throw std::runtime_error("SpotLight icon texture is not available");
    }
}

void USpotLightComponent::SetLightColor(const FVector& Value)
{
    if (!std::isfinite(Value.X) || !std::isfinite(Value.Y) || !std::isfinite(Value.Z))
    {
        return;
    }
    LightColor = FVector(std::clamp(Value.X, 0.0f, 1.0f), std::clamp(Value.Y, 0.0f, 1.0f), std::clamp(Value.Z, 0.0f, 1.0f));
}

//최소값 0.01을 내부적으로 적용
void USpotLightComponent::SetConeRadius(float Value)
{
    if (!std::isfinite(Value))
    {
        return;
    }
    ConeRadius = (std::max)(Value, 0.01f);
}

//최소값 0.01을 내부적으로 적용
void USpotLightComponent::SetConeLength(float Value)
{
    if (!std::isfinite(Value))
    {
        return;
    }
    ConeLength = (std::max)(Value, 0.01f);
}

void USpotLightComponent::Serialize(FArchive& Archive)
{
    Super::Serialize(Archive);

    const TArray<float> Color{LightColor.X, LightColor.Y, LightColor.Z};

    Archive.SetArray<float>("LightColor", Color);
    Archive.SetFloat("ConeRadius", ConeRadius);
    Archive.SetFloat("ConeLength", ConeLength);
}

void USpotLightComponent::Deserialize(FArchive& Archive)
{
    Super::Deserialize(Archive);

    if (Archive.Contains("LightColor"))
    {
        const TArray<float> Color = Archive.GetArray<float>("LightColor");

        if (Color.Num() == 3)
        {
            SetLightColor(FVector(Color[0], Color[1], Color[2]));
        }
    }

    if (Archive.Contains("ConeRadius"))
    {
        SetConeRadius(Archive.GetFloat("ConeRadius"));
    }

    if (Archive.Contains("ConeLength"))
    {
        SetConeLength(Archive.GetFloat("ConeLength"));
    }
}

const FMatrix& USpotLightComponent::GetIconWorldMatrix(const UCameraComponent* Camera) const
{
    if (!Camera){return GetWorldMatrix();}

    // 부모 변환까지 반영된 월드 위치 조회함
    const FMatrix& World = GetWorldMatrix();
    const FVector Center = World.GetOrigin();

    // 월드 행렬의 축 길이를 아이콘 크기에 반영함
    const FVector Scale(
        World.GetAxis(0).Length() * IconSize,
        World.GetAxis(1).Length() * IconSize,
        World.GetAxis(2).Length() * IconSize
    );

    // 카메라 기준의 가로와 세로 방향 조회함
    const FVector Right = Camera->GetRight();
    const FVector Up = Camera->GetUp();
    const FVector Forward = Camera->GetForward();

    // 로컬 XY 평면을 카메라의 Right와 Up 방향에 대응시킴
    const FMatrix BillboardRotation(
        Right.X, Right.Y, Right.Z, 0.0f,
        Up.X, Up.Y, Up.Z, 0.0f,
        Forward.X, Forward.Y, Forward.Z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    // 크기 적용 후 카메라 방향으로 정렬하고 월드 위치로 이동함
    IconWorldMatrix = FMatrix::MakeScaleMatrix(Scale) * BillboardRotation * FMatrix::MakeTranslationMatrix(Center);

    return IconWorldMatrix;
}

FPrimitiveRenderData USpotLightComponent::BuildIconRenderData(const UCameraComponent* Camera, bool bSelected) const
{
    FPrimitiveRenderData Data{};

    if (!Camera || !IconMesh || !IconTexture) { return Data; }

    // 공유 아이콘 메시의 GPU 버퍼 연결함
    Data.VertexBuffer = IconMesh->GetVertexBuffer();
    Data.IndexBuffer = IconMesh->GetIndexBuffer();
    Data.Stride = IconMesh->GetStride();
    Data.IndexCount = IconMesh->GetIndexCount();
    Data.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // 공유 텍스처와 컴포넌트 소유 행렬 연결함
    Data.Material = IconTexture->GetSRV();
    Data.WorldMatrix = &GetIconWorldMatrix(Camera);

    Data.Min = IconMesh->GetBoundsMin();
    Data.Max = IconMesh->GetBoundsMax();

    Data.Pipeline = EPrimitivePipeline::Texture;
    Data.BlendMode = EPrimitiveBlendMode::Opaque;

    // 흰색 아이콘에 광원 색상을 곱함
    Data.TextureTint = FVector4(LightColor.X, LightColor.Y, LightColor.Z, 1.0f);

    // 투명 배경의 픽셀을 제거함
    Data.AlphaCutoff = 0.1f;

    // 아이콘 쿼드의 일반 메시 외곽선 출력을 차단함
    Data.isSelected = bSelected;
    Data.bAllowOutline = false;

    return Data;
}

FLineDrawRequest USpotLightComponent::BuildConeLineDrawRequest() const
{
    FLineDrawRequest Request;

    constexpr uint32 SegmentCount = 64;
    
    // 꼭짓점 1개와 밑면 정점 64개를 위한 공간 확보함
    Request.Vertices.Reserve(1 + SegmentCount);

    // 밑면 선 64개와 옆선 64개에 필요한 인덱스 공간 확보함
    Request.Indices.Reserve(SegmentCount * 4);

    // 빌보드 행렬 대신 실제 광원의 월드 행렬 사용함
    const FMatrix& World = GetWorldMatrix();

    // 로컬 정점을 월드 공간으로 변환하고 광원 색상을 적용함
    const auto AddVertex = [&](const FVector& LocalPosition)
        {
            const FVector Position = World.TransformPosition(LocalPosition);
            Request.Vertices.Add(FVertexSimple{Position.X,Position.Y,Position.Z,LightColor.X,LightColor.Y,LightColor.Z,1.0f});
        };

    // 0번 정점에 원뿔 꼭짓점 저장함
    AddVertex(FVector(0.0f, 0.0f, 0.0f));

    // 로컬 +X 방향의 밑면 원을 구성함
    for (uint32 Index = 0; Index < SegmentCount; ++Index)
    {
        const float Angle = 2 * PI * static_cast<float>(Index) / static_cast<float>(SegmentCount);

        AddVertex(FVector(ConeLength,ConeRadius * std::cos(Angle),ConeRadius * std::sin(Angle)));
    }

    // 밑면 테두리와 꼭짓점 방향의 옆선을 구성함
    for (uint32 Index = 0; Index < SegmentCount; ++Index)
    {
        const uint32 Current = 1 + Index;
        const uint32 Next = 1 + ((Index + 1) % SegmentCount);

        // 인접한 밑면 정점을 연결함
        Request.Indices.Add(Current);
        Request.Indices.Add(Next);

        // 모든 밑면 정점을 꼭짓점과 연결함
        Request.Indices.Add(0);
        Request.Indices.Add(Current);
    }

    return Request;
}