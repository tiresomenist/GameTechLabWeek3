#pragma once

#include "Core/Math/Matrix.h"
#include "Core/Core.h"
#include "Engine/Component/USceneComponent.h"

class FArchive;

class UCameraComponent : public USceneComponent
{

    UCLASS(UCameraComponent, "CameraComponent", USceneComponent)

public:
    FVector GetForward() const;
    FVector GetRight() const;
    FVector GetUp() const;
    void RemoveRoll();
    void ConstrainEditorRotation();

    FMatrix GetViewMatrix() const;
    FMatrix GetProjectionMatrix() const;
    FMatrix GetOrthographicProjectionMatrix() const;
    FMatrix GetPerspectiveProjectionMatrix() const;
    void MoveCamera(const float& InForward, const float& InRight, const float& InUp, const float& InDeltaTime);
    float GetOrthoHeight() const;
    void SetOrthoHeight(float InHeight);

    float GetFOV()const;
    float GetAspectRatio()const;
    float GetNearZ()const;
    float GetFarZ()const;
    bool GetIsPerspective() const;
    void SetIsPerspective(bool Value);

    void SetFOVByRadian(const float& InRadian);
    void SetFOVByDegree(const float& InDegree);

    void SetAspectRatio(const float& InRatio);

    //카메라가 원하는 지점을 바라보도록 하는 함수
    void LookAt(const FVector& InTargetPosition);
    
    void SetMoveSpeed(const float& InMoveSpeed);
    float GetMoveSpeed() const;


    virtual void Serialize(FArchive& Archive) override;
    virtual void Deserialize(FArchive& Archive) override;

private:
    FMatrix GetCameraRotationMatrix() const;

    bool bIsPerspective = true;
    float FOV = 60.0f * PI / 180.0f;   //세로 시야각. 저장단위 라디안
    float AspectRatio = 1.0f;          //뷰포트 가로/세로 비율
    float NearZ = 0.1f;
    float OrthoHeight = 10.0f;         //직교 투영의 전체 세로 범위 (월드 단위)
    float FarZ = 1000.0f;
    float MoveSpeed = 5.0f;

};
