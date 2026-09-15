#include "pch.h"
#include "UFlipbookComponent.h"
#include "Engine/Resource/GResourceManager.h"
#include "Engine/Resource/FTextureResource.h"
#include "Engine/Object/FArchive.h"
#include "Engine/Component/UCameraComponent.h"

#include <algorithm>
#include <cmath>

void UFlipbookComponent::Initialize()
{
	Super::Initialize();
	Texture = GResourceManager::GetInstance()->GetOrLoadTexture("Assets/Textures/FlameTexture.png");
    SetAtlasGrid(Columns, Rows, FrameCount);
    Restart();
}

void UFlipbookComponent::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!bPlaying || !std::isfinite(DeltaTime) || DeltaTime <= 0.0f) return;

    const double NextPosition = FramePosition +
        static_cast<double>(DeltaTime) * FramesPerSecond * PlayRate;
    if (bLoop)
    {
        // 긴 프레임에서도 경과한 모든 프레임을 반영함
        FramePosition = std::fmod(NextPosition, static_cast<double>(FrameCount));
    }
    else if (NextPosition >= FrameCount)
    {
        // 반복하지 않으면 마지막 프레임을 유지하고 정지함
        FramePosition = FrameCount - 1;
        bPlaying = false;
    }
    else
    {
        FramePosition = NextPosition;
    }
}

const FMatrix& UFlipbookComponent::GetRenderWorldMatrix(const UCameraComponent* Camera) const
{
    // 카메라가 없으면 기존 월드 행렬 사용함
    if (!Camera)
    {
        return GetWorldMatrix();
    }

    // 카메라 기준의 가로·세로·깊이 방향 조회함
    const FVector Right = Camera->GetRight();
    const FVector Up = Camera->GetUp();
    const FVector Forward = Camera->GetForward();

    // 빌보드 방향은 카메라를 따르되, 위치와 크기는 Attach 부모까지 반영한다.
    const FMatrix& WorldMatrix = GetWorldMatrix();
    const FVector Scale(
        WorldMatrix.GetAxis(0).Length(),
        WorldMatrix.GetAxis(1).Length(),
        WorldMatrix.GetAxis(2).Length());
    const FVector Center = WorldMatrix.GetOrigin();

    // Flame의 로컬 XY 평면을 카메라의 Right-Up 평면에 대응시킴
    const FMatrix BillboardRotation(
        Right.X, Right.Y, Right.Z, 0.0f,
        Up.X, Up.Y, Up.Z, 0.0f,
        Forward.X, Forward.Y, Forward.Z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    // 크기 적용 → 빌보드 방향 적용 → 월드 위치 이동
    BillboardWorldMatrix =
        FMatrix::MakeScaleMatrix(Scale)
        * BillboardRotation
        * FMatrix::MakeTranslationMatrix(Center);

    return BillboardWorldMatrix;
}

void UFlipbookComponent::SetAtlasGrid(int32 InColumns, int32 InRows, int32 InFrameCount)
{
    // 과도한 격자와 픽셀보다 작은 칸을 방지함
    const int32 MaxColumns = Texture ? static_cast<int32>(Texture->GetWidth()) : 16384;
    const int32 MaxRows = Texture ? static_cast<int32>(Texture->GetHeight()) : 16384;
    Columns = std::clamp(InColumns, 1, MaxColumns);
    Rows = std::clamp(InRows, 1, MaxRows);
    const int32 CellCount = Columns * Rows;
    FrameCount = InFrameCount == 0 ? CellCount : std::clamp(InFrameCount, 1, CellCount);
    SetCurrentFrame(GetCurrentFrame());
}

void UFlipbookComponent::SetFramesPerSecond(float Value)
{
    if (std::isfinite(Value)) FramesPerSecond = (std::max)(Value, 0.0f);
}

void UFlipbookComponent::SetPlayRate(float Value)
{
    if (std::isfinite(Value)) PlayRate = (std::max)(Value, 0.0f);
}

void UFlipbookComponent::SetCurrentFrame(int32 Value)
{
    FramePosition = std::clamp(Value, 0, FrameCount - 1);
}

void UFlipbookComponent::Restart()
{
    FramePosition = 0.0;
    bPlaying = true;
}

FTextureUVTransform UFlipbookComponent::GetUVTransform() const
{
    const int32 Frame = GetCurrentFrame();
    // 선형 필터가 인접 프레임을 섞지 않도록 양쪽 경계를 반 텍셀씩 줄임
    const float InsetU = Texture ? 0.5f / Texture->GetWidth() : 0.0f;
    const float InsetV = Texture ? 0.5f / Texture->GetHeight() : 0.0f;
    return {
        (std::max)(1.0f / Columns - 2.0f * InsetU, 0.0f),
        (std::max)(1.0f / Rows - 2.0f * InsetV, 0.0f),
        static_cast<float>(Frame % Columns) / Columns + InsetU,
        static_cast<float>(Frame / Columns) / Rows + InsetV,
    };
}

FPrimitiveRenderData UFlipbookComponent::CreateRenderData(bool bSelected) const
{
    if (!Texture || !Texture->GetSRV())
    {
        return {};
    }

    FPrimitiveRenderData Data = Super::CreateRenderData(bSelected);

    Data.Pipeline = EPrimitivePipeline::Texture;
    Data.Material = Texture->GetSRV();
    Data.UVTransform = GetUVTransform();
    Data.BlendMode = EPrimitiveBlendMode::Additive;
    return Data;
}

void UFlipbookComponent::Serialize(FArchive& Archive)
{
    Super::Serialize(Archive);
    Archive.SetInt32("SubUVColumns", Columns);
    Archive.SetInt32("SubUVRows", Rows);
    Archive.SetInt32("SubUVFrameCount", FrameCount);
    Archive.SetFloat("SubUVFPS", FramesPerSecond);
    Archive.SetFloat("SubUVPlayRate", PlayRate);
    Archive.SetBool("SubUVLoop", bLoop);
}

void UFlipbookComponent::Deserialize(FArchive& Archive)
{
    Super::Deserialize(Archive);
    // 기존 씬은 기본 설정을 사용하고 저장된 재생 설정이 있으면 복원함
    SetAtlasGrid(
        Archive.Contains("SubUVColumns") ? Archive.GetInt32("SubUVColumns") : 6,
        Archive.Contains("SubUVRows") ? Archive.GetInt32("SubUVRows") : 6,
        Archive.Contains("SubUVFrameCount") ? Archive.GetInt32("SubUVFrameCount") : 0);
    SetFramesPerSecond(Archive.Contains("SubUVFPS") ? Archive.GetFloat("SubUVFPS") : 24.0f);
    SetPlayRate(Archive.Contains("SubUVPlayRate") ? Archive.GetFloat("SubUVPlayRate") : 1.0f);
    bLoop = Archive.Contains("SubUVLoop") ? Archive.GetBool("SubUVLoop") : true;
    // 재생 진행도는 저장하지 않으며 씬 로드 시 처음부터 재생함
    Restart();
}
