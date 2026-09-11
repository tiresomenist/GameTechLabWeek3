#pragma once
#include "FVector.h"
#include "Matrix.h"

struct FQuaternion
{
    float X = 0, Y = 0, Z = 0, W = 1;

    // Axis is normalized internally. A zero/non-finite axis or angle gives identity.
    static FQuaternion FromAxisAngle(const FVector& Axis, float Radian);

    // X/Y/Z angles in radians. Matches FMatrix::MakeRotationMatrix (Rz * Ry * Rx).
    // Returns identity if any angle is non-finite.
    static FQuaternion FromEuler(const FVector& EulerRadians);
    // Inverse of FromEuler: radians, Y in [-pi/2, pi/2], X/Z in [-pi, pi].
    // At gimbal lock choose Z = 0. Equivalent rotations may have different angles.
    static FVector ToEuler(const FQuaternion& InQuaternion);

    // Shortest rotation between directions; invalid/zero directions give identity.
    static FQuaternion FromToRotation(const FVector& From, const FVector& To);

    // Hamilton product: Other is applied first, then *this.
    // For row-vector matrices: Matrix(A * B) = Matrix(B) * Matrix(A).
    FQuaternion operator*(const FQuaternion& Other) const;
    // Zero/non-finite quaternions fall back to identity.
    void Normalize();

    // Preserve local +X (forward), align local +Z toward WorldUp.
    // At a vertical view the horizon is undefined; retain the current orientation.
    FQuaternion GetWithoutRoll(const FVector& WorldUp = FVector(0, 0, 1)) const;
    // Upright camera pose, with elevation limited to +/-89 degrees.
    FQuaternion GetUprightCameraRotation() const;

    // Row-vector convention (Vector * Matrix); normalizes a copy.
    FMatrix ToRotationMatrix() const;
};
