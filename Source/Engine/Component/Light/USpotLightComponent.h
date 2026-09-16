#pragma once

#include "Engine/Component/Light/ULightComponent.h"
#include "Core/Math/FVector.h"
#include "Core/Math/Matrix.h"
#include "Engine/Renderer/FPrimitiveRenderData.h"
#include "Engine/Renderer/Line/FLineDrawRequest.h"

class FArchive;
class FMeshResource;
class FTextureResource;
class UCameraComponent;


class USpotLightComponent:public ULightComponent
{
	UCLASS(USpotLightComponent, "SpotLight", ULightComponent)

public:
    void Initialize() override;
    const FVector& GetLightColor() const
    {
        return LightColor;
    }

    float GetConeRadius() const
    {
        return ConeRadius;
    }

    float GetConeLength() const
    {
        return ConeLength;
    }
    const FMatrix& GetIconWorldMatrix(const UCameraComponent* Camera) const;
    FPrimitiveRenderData BuildIconRenderData(const UCameraComponent* Camera, bool bSelected) const;
    void SetLightColor(const FVector& Value);
    void SetConeRadius(float Value);
    void SetConeLength(float Value);

    void Serialize(FArchive& Archive) override;
    void Deserialize(FArchive& Archive) override;
    FLineDrawRequest BuildConeLineDrawRequest() const;

private:
    // 아이콘과 원뿔에 적용할 RGB 색상
    FVector LightColor{ 1.0f, 1.0f, 1.0f };

    // 로컬 공간에서의 원뿔 밑면 반지름
    float ConeRadius = 2.0f;

    // 로컬 공간에서의 꼭짓점과 밑면 중심 사이 거리
    float ConeLength = 5.0f;

    FMeshResource* IconMesh = nullptr;
    FTextureResource* IconTexture = nullptr;

    // 단위 쿼드에 적용할 기본 아이콘 크기
    float IconSize = 0.5f;

    // 렌더 데이터가 참조할 행렬을 컴포넌트에 보관함
    mutable FMatrix IconWorldMatrix = FMatrix::Identity;

};
