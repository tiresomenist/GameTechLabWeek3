#include "pch.h"
#include "FVector.h"


const FVector FVector::Zero = FVector(0.0f, 0.0f, 0.0f);
const FVector FVector::Forward = FVector(1.0f, 0.0f, 0.0f);
const FVector FVector::Right = FVector(0.0f, 1.0f, 0.0f);
const FVector FVector::Up = FVector(0.0f, 0.0f, 1.0f);

FVector FVector::operator+(const FVector& rhs) const
{
	return FVector(X + rhs.X, Y + rhs.Y, Z + rhs.Z);
}

FVector FVector::operator-(const FVector& rhs) const
{
	return FVector(X - rhs.X, Y - rhs.Y, Z - rhs.Z);
}

FVector FVector::operator*(float scalar) const
{
	return FVector(X * scalar, Y * scalar, Z * scalar);
}

FVector FVector::operator/(float scalar) const
{
	assert(std::fabs(scalar) > EPSILON);

	if (std::fabs(scalar) <= EPSILON)
	{

		return Zero;
	}

	const float Inverse = 1.0f / scalar;
	return *this * Inverse;
}

FVector& FVector::operator+=(const FVector& rhs)
{
	FVector result = *this + rhs;
	*this = result;
	return *this;
}

FVector& FVector::operator-=(const FVector& rhs)
{
	FVector result = *this - rhs;
	*this = result;
	return *this;
}

FVector& FVector::operator*=(float scalar)
{
	FVector result = *this * scalar;
	*this = result;
	return *this;
}

FVector& FVector::operator/=(float scalar)
{
	FVector result = *this / scalar;
	*this = result;
	return *this;
}

float FVector::Dot(const FVector& rhs)const
{
	return X * rhs.X + Y * rhs.Y + Z * rhs.Z;
}

FVector FVector::Cross(const FVector& rhs)const
{
	return FVector(
		Y * rhs.Z - Z * rhs.Y,
		Z * rhs.X - X * rhs.Z,
		X * rhs.Y - Y * rhs.X
	);
}

bool FVector::Equals(const FVector& other, float Epsilon) const
{
	return std::fabs(X - other.X) <= Epsilon && std::fabs(Y - other.Y) <= Epsilon && std::fabs(Z - other.Z) <= Epsilon;
}

float FVector::Length() const
{
	return sqrtf(LengthSquared());
}

float FVector::LengthSquared() const
{
	return X * X + Y * Y + Z * Z;
}

float FVector::Distance(const FVector& rhs) const
{
	FVector diff = *this - rhs;
	return diff.Length();
}

FVector FVector::GetNormalized() const
{
	float length = Length();
	if (length > EPSILON)
	{
		return *this / length;
	}
	return FVector();
}

void FVector::Normalize()
{
	float length = Length();
	if (length > EPSILON)
	{
		*this /= length;
	}
}

FVector::FVector(const FVector4& InVector4):X(InVector4.X), Y(InVector4.Y), Z(InVector4.Z)
{
}

float FVector4::Dot(const FVector4& Other)const
{
	return X * Other.X + Y * Other.Y + Z * Other.Z + W * Other.W;
}


float FVector4::LengthSquared()const
{
	return X * X + Y * Y + Z * Z + W * W;
}

float FVector4::Length()const
{
	return sqrtf(LengthSquared());
}

float FVector4::LengthSquared3()const
{
	return X * X + Y * Y + Z * Z;
}

float FVector4::Length3()const
{
	return sqrtf(LengthSquared3());
}

FVector4::FVector4(const FVector& InVector, float _w):X(InVector.X), Y(InVector.Y), Z(InVector.Z),W(_w)
{
}

FVector FVector4::getXYZ() const
{
	return FVector(X, Y, Z);
}
