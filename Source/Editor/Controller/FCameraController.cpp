#include "pch.h"
#include "FCameraController.h"
#include "Engine/Input/GInputManager.h"
#include "Engine/Component/UCameraComponent.h"
#include "Engine/Log.h"




void FCameraController::SetCamera(UCameraComponent* InCamera)
{
    Camera = InCamera;
    if (Camera) Camera->ConstrainEditorRotation();
    //이전 카메라의 물리량을 반영하면 안됨
    int32 DeltaX = 0, DeltaY = 0;
    GInputManager::GetInstance()->ConsumeRightDragDelta(DeltaX, DeltaY);
}

void FCameraController::Tick(float DeltaTime)
{
    if (!Camera) return;

    auto& Input = *GInputManager::GetInstance();
    
    //카메라 회전부
    int32 DeltaX = 0, DeltaY = 0;
    Input.ConsumeRightDragDelta(DeltaX, DeltaY);

    Camera->ConstrainEditorRotation();
    if (Input.GetKey(GInputManager::EI_RMOUSE) && (DeltaX != 0 || DeltaY != 0))
    {
        const float RadiansPerPixel = RotationSensitivity * PI / 180.0f;
        const float YawAngle = DeltaX * RadiansPerPixel;
        const float PitchAngle = DeltaY * RadiansPerPixel;
        // Small steps stop even a large input at the limit BEFORE crossing a pole.
        constexpr float MaxStep = 0.05f * PI / 180.0f;
        const float LargestAngle = (std::max)(std::fabs(YawAngle), std::fabs(PitchAngle));
        const int Steps = (std::max)(1, int(std::ceil(LargestAngle / MaxStep)));
        const FQuaternion Yaw = FQuaternion::FromAxisAngle(FVector(0, 0, 1), YawAngle / Steps);
        const FQuaternion Pitch = FQuaternion::FromAxisAngle(FVector(0, 1, 0), PitchAngle / Steps);
        FQuaternion Rotation = Camera->GetRelativeRotation();
        for (int Step = 0; Step < Steps; ++Step)
        {
            Rotation = (Yaw * Rotation).GetUprightCameraRotation();
            Rotation = (Rotation * Pitch).GetUprightCameraRotation();
        }
        Camera->SetRelativeRotation(Rotation);
    }

    //카메라 이동부
    const float Forward = float(Input.GetKey(GInputManager::EI_W)) - float(Input.GetKey(GInputManager::EI_S));
    const float Right = float(Input.GetKey(GInputManager::EI_D)) - float(Input.GetKey(GInputManager::EI_A));
    const float Up = float(Input.GetKey(GInputManager::EI_E)) - float(Input.GetKey(GInputManager::EI_Q));
    Camera->MoveCamera(Forward, Right, Up, DeltaTime);
}
