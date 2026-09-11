#include "pch.h"
#include "UCameraComponent.h"
#include "Engine/Object/UObject.h"

FVector UCameraComponent::GetForward() const
{
	FVector4 ForwardVector(1.0f, 0.0f, 0.0f, 0.0f);

	return FVector(ForwardVector * GetCameraRotationMatrix());
}

FVector UCameraComponent::GetRight() const
{
	FVector4 RightVector(0.0f, 1.0f, 0.0f, 0.0f);
	return FVector(RightVector * GetCameraRotationMatrix());
}

FVector UCameraComponent::GetUp() const
{
	FVector4 UpVector(0.0f, 0.0f, 1.0f, 0.0f);
	return FVector(UpVector * GetCameraRotationMatrix());
}

void UCameraComponent::RemoveRoll()
{
    SetRelativeRotation(RelativeRotation.GetWithoutRoll());
}

void UCameraComponent::ConstrainEditorRotation()
{
    SetRelativeRotation(RelativeRotation.GetUprightCameraRotation());
}

FMatrix UCameraComponent::GetViewMatrix() const
{
	FMatrix RotationMatrix = GetCameraRotationMatrix();
	FMatrix InverseTranslationMatrix = FMatrix::MakeTranslationMatrix(RelativeLocation * -1.0f);
	return (InverseTranslationMatrix) * (RotationMatrix.Transpose());
}

FMatrix UCameraComponent::GetProjectionMatrix() const
{
	if (bIsPerspective)
	{
		return GetPerspectiveProjectionMatrix();
	}
	else
	{
		return GetOrthographicProjectionMatrix();
	}
}



float UCameraComponent::GetFOV() const
{
	return FOV;
}

float UCameraComponent::GetAspectRatio() const
{
	return AspectRatio;
}

float UCameraComponent::GetNearZ() const
{
	return NearZ;
}

float UCameraComponent::GetFarZ() const
{
	return FarZ;
}

bool UCameraComponent::GetIsPerspective() const
{
	return bIsPerspective;
}

void UCameraComponent::SetIsPerspective(bool Value)
{
	bIsPerspective = Value;
}

void UCameraComponent::SetFOVByRadian(const float& InRadian)
{
	FOV = InRadian;
}

void UCameraComponent::SetFOVByDegree(const float& InDegree)
{
	//도 단위의 각도를 라디안으로 자동으로 변환해서 세팅해줌.
	FOV = InDegree * PI / 180.f;
}

void UCameraComponent::SetAspectRatio(const float& InRatio)
{
	AspectRatio = InRatio;
}

void UCameraComponent::LookAt(const FVector& InTargetPosition)
{
	FVector Forward = (InTargetPosition - RelativeLocation);
	if (Forward.Length() <= EPSILON) {
		//카메라의 위치를 바라보는 경우
		return;
	}
    const FQuaternion Delta = FQuaternion::FromToRotation(GetForward(), Forward);
    // Align the view, then keep the horizon upright without changing the target.
    AddWorldRotation(Delta);
    RemoveRoll();

}

void UCameraComponent::Serialize(FArchive& Archive)
{
	Super::Serialize(Archive);

	Archive.SetFloat("FOV", FOV);
	Archive.SetFloat("AspectRatio", AspectRatio);
	Archive.SetFloat("Near", NearZ);
	Archive.SetFloat("Far", FarZ);
	Archive.SetFloat("MoveSpeed", MoveSpeed);
	Archive.SetFloat("OrthogonalHeight", OrthoHeight);
	Archive.SetBool("Perspective", bIsPerspective);
}

void UCameraComponent::Deserialize(FArchive& Archive)
{
	Super::Deserialize(Archive);

	FOV = Archive.GetFloat("FOV");
	AspectRatio = Archive.GetFloat("AspectRatio");
	NearZ = Archive.GetFloat("Near");
	FarZ = Archive.GetFloat("Far");
	MoveSpeed = Archive.GetFloat("MoveSpeed");
	OrthoHeight = Archive.GetFloat("OrthogonalHeight");
	bIsPerspective = Archive.GetBool("Perspective");
}

float UCameraComponent::GetOrthoHeight() const
{
	return OrthoHeight;
}

void UCameraComponent::SetOrthoHeight(float InHeight)
{
	if (!std::isfinite(InHeight) || InHeight <= 0.0f)
	{
		return;
	}
	OrthoHeight = InHeight;
}

FMatrix UCameraComponent::GetOrthographicProjectionMatrix() const
{
	assert(std::isfinite(OrthoHeight) && OrthoHeight > 0.0f);
	assert(std::isfinite(AspectRatio) && AspectRatio > 0.0f);
	assert(std::isfinite(NearZ) && std::isfinite(FarZ) && FarZ > NearZ);

	const float VerticalScale = 2.0f / OrthoHeight;
	const float HorizontalScale = VerticalScale / AspectRatio;
	const float DepthScale = 1.0f / (FarZ - NearZ);

	// +X forward, +Y right, +Z up; row vectors, depth 0..1, W = 1.
	return FMatrix(
		0.0f, 0.0f, DepthScale, 0.0f,
		HorizontalScale, 0.0f, 0.0f, 0.0f,
		0.0f, VerticalScale, 0.0f, 0.0f,
		0.0f, 0.0f, -NearZ * DepthScale, 1.0f);
}

FMatrix UCameraComponent::GetPerspectiveProjectionMatrix() const
{
	//시야각, 가로세로 비율,  가시경계 범위 체크
	assert(std::isfinite(FOV) && FOV > 0.0f && FOV < PI);
	assert(std::isfinite(AspectRatio) && AspectRatio > 0.0f);
	assert(std::isfinite(NearZ) && std::isfinite(FarZ) && NearZ > 0.0f && FarZ > NearZ);

	const float VerticalScale = 1.0f / std::tan(FOV * 0.5f);
	const float HorizontalScale = VerticalScale / AspectRatio;
	const float DepthScale = FarZ / (FarZ - NearZ);

	//일반적인 투영행렬과 다른이유 : 기준 축이 달라서 축변환 적용
	return FMatrix(
		0.0f, 0.0f, DepthScale, 1.0f,
		HorizontalScale, 0.0f, 0.0f, 0.0f,
		0.0f, VerticalScale, 0.0f, 0.0f,
		0.0f, 0.0f, -NearZ * DepthScale, 0.0f);
}

void UCameraComponent::MoveCamera(const float& InForward, const float& InRight, const float& InUp, const float& InDeltaTime)
{
	FVector InVelocity = GetForward() * InForward + GetRight() * InRight + GetUp()*InUp;
	if (InVelocity.Length() < EPSILON) return;
	InVelocity.Normalize();
	SetRelativeLocation(RelativeLocation + InVelocity * MoveSpeed * InDeltaTime);
}

FMatrix UCameraComponent::GetCameraRotationMatrix() const
{
    return RelativeRotation.ToRotationMatrix();
}
