#include "pch.h"
#include "FQuaternion.h"

FQuaternion FQuaternion::FromAxisAngle(const FVector& Axis, float Radian)
{
    const double Length = std::hypot(double(Axis.X), double(Axis.Y), double(Axis.Z));
    if (!std::isfinite(Length) || Length == 0.0 || !std::isfinite(Radian))
        return {};

    const double HalfAngle = double(Radian) * 0.5;
    const double Scale = std::sin(HalfAngle) / Length;
    return { float(Axis.X * Scale), float(Axis.Y * Scale),
        float(Axis.Z * Scale), float(std::cos(HalfAngle)) };
}
 
FQuaternion FQuaternion::FromEuler(const FVector& EulerRadians)
{
    if (!std::isfinite(EulerRadians.X) || !std::isfinite(EulerRadians.Y)
        || !std::isfinite(EulerRadians.Z))
        return {};

    const FQuaternion QX = FromAxisAngle(FVector(1, 0, 0), -EulerRadians.X);
    const FQuaternion QY = FromAxisAngle(FVector(0, 1, 0), -EulerRadians.Y);
    const FQuaternion QZ = FromAxisAngle(FVector(0, 0, 1), EulerRadians.Z);
    // Hamilton multiplication reverses the order of the row-vector matrices.
    FQuaternion Result = QZ * QY * QX;
    Result.Normalize();
    return Result;
}

FVector FQuaternion::ToEuler(const FQuaternion& InQuaternion)
{
    // QZ*QY*QX로부터 오일러각을 복원하는 함수
    // Pitch=90도일때 yaw,roll 값이 급격하게 튈 수 있음.
    const double X = InQuaternion.X, Y = InQuaternion.Y, Z = InQuaternion.Z, W = InQuaternion.W;
    const double LengthSquared = X * X + Y * Y + Z * Z + W * W;
    if (!std::isfinite(LengthSquared) || LengthSquared == 0.0) return FVector();

    const double S = 2 / LengthSquared;
    const double R00 = 1.0 - S * (Y * Y + Z * Z);
    const double R01 = S * (X * Y + Z * W);
    const double R02 = S * (X * Z - Y * W);
    const double R10 = S * (X * Y - Z * W);
    const double R11 = 1.0 - S * (X * X + Z * Z);
    const double R12 = S * (Y * Z + X * W);
    const double R22 = 1.0 - S * (X * X + Y * Y);

    const double CosPitch = std::hypot(R00, R01);
    const double Pitch = std::atan2(R02, CosPitch);

    double Roll = 0.0;
    double Yaw = 0.0;

    if (CosPitch > EPSILON)
    {
        Roll = std::atan2(-R12, R22);
        Yaw = std::atan2(R01, R00);
    }
    else
    {
        // Pitch가 ±90도 부근이면 Roll과 Yaw를 독립적으로 정할 수 없음
        // Roll을 0으로 정하고 결합된 회전을 Yaw로 표현함
        Roll = 0.0;
        Yaw = std::atan2(-R10, R11);
    }

    return FVector(static_cast<float>(Roll),static_cast<float>(Pitch),static_cast<float>(Yaw));
}

FQuaternion FQuaternion::FromToRotation(const FVector& From, const FVector& To)
{
    const double FromLength = std::hypot(double(From.X), double(From.Y), double(From.Z));
    const double ToLength = std::hypot(double(To.X), double(To.Y), double(To.Z));
    if (!std::isfinite(FromLength) || !std::isfinite(ToLength)
        || FromLength == 0.0 || ToLength == 0.0) return {};

    const double AX = From.X / FromLength, AY = From.Y / FromLength, AZ = From.Z / FromLength;
    const double BX = To.X / ToLength, BY = To.Y / ToLength, BZ = To.Z / ToLength;
    const double CX = AY * BZ - AZ * BY, CY = AZ * BX - AX * BZ, CZ = AX * BY - AY * BX;
    const double CrossLength = std::hypot(CX, CY, CZ);
    const double Dot = AX * BX + AY * BY + AZ * BZ;
    if (CrossLength < 1e-12)
    {
        if (Dot >= 0.0) return {};
        // At 180 degrees any perpendicular axis is valid; pick a stable one.
        const FVector A{float(AX), float(AY), float(AZ)};
        const FVector Basis = std::fabs(AX) < 0.9 ? FVector(1, 0, 0) : FVector(0, 1, 0);
        return FromAxisAngle(A.Cross(Basis), 3.14159265358979323846f);
    }
    return FromAxisAngle(FVector(float(CX / CrossLength), float(CY / CrossLength), float(CZ / CrossLength)),
        float(std::atan2(CrossLength, Dot)));
}

FQuaternion FQuaternion::operator*(const FQuaternion& Other) const
{
    return {
        W * Other.X + X * Other.W + Y * Other.Z - Z * Other.Y,
        W * Other.Y - X * Other.Z + Y * Other.W + Z * Other.X,
        W * Other.Z + X * Other.Y - Y * Other.X + Z * Other.W,
        W * Other.W - X * Other.X - Y * Other.Y - Z * Other.Z
    };
}

void FQuaternion::Normalize()
{
    // Double intermediates avoid overflow/underflow for finite float components.
    const double Length = std::sqrt(double(X) * X + double(Y) * Y
        + double(Z) * Z + double(W) * W);
    if (!std::isfinite(Length) || Length == 0.0)
    {
        *this = FQuaternion{};
        return;
    }
    X = float(X / Length);
    Y = float(Y / Length);
    Z = float(Z / Length);
    W = float(W / Length);
}

FMatrix FQuaternion::ToRotationMatrix() const
{
    FQuaternion Q = *this;
    Q.Normalize();
    const float XX = Q.X * Q.X, YY = Q.Y * Q.Y, ZZ = Q.Z * Q.Z;
    const float XY = Q.X * Q.Y, XZ = Q.X * Q.Z, YZ = Q.Y * Q.Z;
    const float XW = Q.X * Q.W, YW = Q.Y * Q.W, ZW = Q.Z * Q.W;
    return FMatrix(
        1.0f - 2.0f * (YY + ZZ), 2.0f * (XY + ZW), 2.0f * (XZ - YW), 0.0f,
        2.0f * (XY - ZW), 1.0f - 2.0f * (XX + ZZ), 2.0f * (YZ + XW), 0.0f,
        2.0f * (XZ + YW), 2.0f * (YZ - XW), 1.0f - 2.0f * (XX + YY), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}

FQuaternion FQuaternion::GetUprightCameraRotation() const
{
    const FMatrix R = ToRotationMatrix();
    const FVector Forward = R.GetAxis(0);
    const float Horizontal = std::hypot(Forward.X, Forward.Y);
    // At the pole, recover heading from the previous right axis.
    const float Yaw = Horizontal > 1e-6f ? std::atan2(Forward.Y, Forward.X)
        : std::atan2(-R.M[1][0], R.M[1][1]);
    constexpr float Limit = 89.0f * PI / 180.0f;
    const float Pitch = std::clamp(std::atan2(-Forward.Z, Horizontal), -Limit, Limit);
    FQuaternion Result = FromAxisAngle(FVector(0, 0, 1), Yaw)
        * FromAxisAngle(FVector(0, 1, 0), Pitch);
    Result.Normalize();
    return Result;
}

FQuaternion FQuaternion::GetWithoutRoll(const FVector& WorldUp) const
{
    FQuaternion Orientation = *this;
    Orientation.Normalize();
    const FMatrix Rotation = Orientation.ToRotationMatrix();
    const FVector Forward = Rotation.GetAxis(0);
    const FVector Right = Rotation.GetAxis(1);
    const FVector UpReference = WorldUp.GetNormalized();
    FVector TargetRight = UpReference.Cross(Forward);
    // Avoid amplifying rounding errors when forward is parallel to world up.
    if (!std::isfinite(TargetRight.LengthSquared()) || TargetRight.LengthSquared() < 1e-8f)
        return Orientation;
    TargetRight.Normalize();

    // A world-space twist about forward changes only roll, not the viewing direction.
    const float SinAngle = Forward.Dot(Right.Cross(TargetRight));
    const float CosAngle = Right.Dot(TargetRight);
    const float Angle = std::atan2(SinAngle, CosAngle);
    FQuaternion Result = FromAxisAngle(Forward, Angle) * Orientation;
    Result.Normalize();
    return Result;
}
