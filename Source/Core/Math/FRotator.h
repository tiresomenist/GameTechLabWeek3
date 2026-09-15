#pragma once
#include "Core/Math/FVector.h"
#include "Core/Math/FQuaternion.h"

struct FRotator
{
    // 도 단위 회전값
    float Pitch = 0.0f;
    float Yaw = 0.0f;
    float Roll = 0.0f;

    FRotator() = default;
    FRotator(const float& InPitch, const float& InYaw, const float& InRoll);

    bool IsFinite() const;

    // XYZ 순서의 도 단위 각도로 변환함
    FVector ToEulerDegrees() const;
    static FRotator FromEulerDegrees(const FVector& Degrees);

    FQuaternion ToQuaternion() const;
    static FRotator FromQuaternion(const FQuaternion& Quaternion);

    // 기준 각도에 가까운 값을 구하기 위함
    FRotator GetUnwoundNear(const FRotator& Reference) const;
};