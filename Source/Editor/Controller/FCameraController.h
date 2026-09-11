#pragma once

class UCameraComponent;

// Scene owns the camera; this controller only updates the assigned camera.
class FCameraController
{
public:
    void SetCamera(UCameraComponent* InCamera);
    void Tick(float DeltaTime);

    float RotationSensitivity = 0.5f; // Degrees per pixel.

private:
    UCameraComponent* Camera = nullptr;
};
