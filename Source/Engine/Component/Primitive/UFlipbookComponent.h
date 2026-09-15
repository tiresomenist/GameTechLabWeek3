#pragma once

#include "UPrimitiveComponent.h"
#include "Engine/Resource/GResourceManager.h"
#include "Engine/Object/FClassType.h"

class UCameraComponent;

class UFlipbookComponent : public UPrimitiveComponent
{

	UCLASS(UFlipbookComponent, "Flame", UPrimitiveComponent)

public:
    virtual void Initialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual FPrimitiveRenderData CreateRenderData(bool bSelected = false) const override;
    virtual void Serialize(FArchive& Archive) override;
    virtual void Deserialize(FArchive& Archive) override;

    // 프레임은 좌측 상단부터 행 순서로 재생하며 0이면 전체 칸을 사용함
    void SetAtlasGrid(int32 InColumns, int32 InRows, int32 InFrameCount = 0);
    void SetFramesPerSecond(float Value);
    void SetPlayRate(float Value);
    void SetLooping(bool Value) { bLoop = Value; }
    void SetPlaying(bool Value) { bPlaying = Value; }
    void SetCurrentFrame(int32 Value);
    void Restart();
    virtual const FMatrix& GetRenderWorldMatrix(const UCameraComponent* Camera) const override;

    int32 GetColumns() const { return Columns; }
    int32 GetRows() const { return Rows; }
    int32 GetFrameCount() const { return FrameCount; }
    int32 GetCurrentFrame() const { return static_cast<int32>(FramePosition); }
    float GetFramesPerSecond() const { return FramesPerSecond; }
    float GetPlayRate() const { return PlayRate; }
    bool IsLooping() const { return bLoop; }
    bool IsPlaying() const { return bPlaying; }
    FTextureUVTransform GetUVTransform() const;

private:
    // 리소스 매니저가 소유하는 텍스처를 참조함
    FTextureResource* Texture = nullptr;

    int32 Columns = 6;
    int32 Rows = 6;
    int32 FrameCount = 36;
    float FramesPerSecond = 24.0f;
    float PlayRate = 1.0f;
    bool bLoop = true;
    bool bPlaying = true;
    // 소수 부분을 유지하여 프레임 시간의 누적 오차를 줄임
    double FramePosition = 0.0;

    mutable FMatrix BillboardWorldMatrix;

};
