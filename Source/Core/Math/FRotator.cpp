#include "pch.h"
#include "Core/Math/FRotator.h"


FRotator::FRotator(const float& InPitch, const float& InYaw, const float& InRoll)
	:Pitch(InPitch),Yaw(InYaw),Roll(InRoll)
{
}

//
bool FRotator::IsFinite() const
{
	return std::isfinite(Pitch) && std::isfinite(Yaw) && std::isfinite(Roll);
}

FVector FRotator::ToEulerDegrees() const
{
	return FVector(Roll, Pitch, Yaw);
}

FRotator FRotator::FromEulerDegrees(const FVector& Degrees)
{
	return FRotator(Degrees.Y, Degrees.Z, Degrees.X);
}

FQuaternion FRotator::ToQuaternion() const
{
	const FVector EulerRadians = FVector(Roll, Pitch, Yaw) * (PI / 180.0f);
	return FQuaternion::FromEuler(EulerRadians);
}

FRotator FRotator::FromQuaternion(const FQuaternion& Quaternion)
{
	const FVector Degrees = FQuaternion::ToEuler(Quaternion) * (180.0f / PI);

	return FRotator(Degrees.Y, Degrees.Z, Degrees.X);
}

FRotator FRotator::GetUnwoundNear(const FRotator& Reference) const
{
    const auto AdjustAngle = [](float Angle, float Previous)
        {
            // 이전 각도에 가장 가까운 회전 바퀴 수 계산함
			const double Turns = std::round((static_cast<double>(Previous) - Angle) / 360.0);
            return static_cast<float>(Angle + 360.0 * Turns);
        };

    return FRotator(AdjustAngle(Pitch, Reference.Pitch),AdjustAngle(Yaw, Reference.Yaw),AdjustAngle(Roll, Reference.Roll));
}
