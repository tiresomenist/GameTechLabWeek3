#include "pch.h"
#include "Matrix.h"
#include "FQuaternion.h"

const FMatrix FMatrix::Identity = FMatrix(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

FMatrix FMatrix::MakeTranslationMatrix(const FVector& Location)
{
	return FMatrix(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		Location.X, Location.Y, Location.Z, 1.0f);
}

FMatrix FMatrix::MakeScaleMatrix(const FVector& Scale)
{
	return FMatrix(
		Scale.X, 0.0f, 0.0f, 0.0f,
		0.0f, Scale.Y, 0.0f, 0.0f,
		0.0f, 0.0f, Scale.Z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
}

FMatrix FMatrix::MakeRotationXMatrix(float Radian)
{
    return FQuaternion::FromAxisAngle(FVector(1, 0, 0), Radian).ToRotationMatrix();
}

FMatrix FMatrix::MakeRotationYMatrix(float Radian)
{
    return FQuaternion::FromAxisAngle(FVector(0, 1, 0), Radian).ToRotationMatrix();
}

FMatrix FMatrix::MakeRotationZMatrix(float Radian)
{
    return FQuaternion::FromAxisAngle(FVector(0, 0, 1), Radian).ToRotationMatrix();
}

FMatrix FMatrix::MakeRotationMatrix(const FVector& EulerRadians)
{
    return FQuaternion::FromEuler(EulerRadians).ToRotationMatrix();
}

FMatrix FMatrix::MakeModelMatrix(
    const FVector& Location, const FQuaternion& Rotation, const FVector& Scale)
{
    return MakeScaleMatrix(Scale) * Rotation.ToRotationMatrix() * MakeTranslationMatrix(Location);
}

FMatrix FMatrix::MakeNormalMatrix(const FMatrix& Input)
{
	return Input.NormalMatrix();
}

FMatrix FMatrix::operator*(const FMatrix& Rhs) const
{
	return FMatrix(M[0][0] * Rhs.M[0][0] + M[0][1] * Rhs.M[1][0] + M[0][2] * Rhs.M[2][0] + M[0][3] * Rhs.M[3][0], M[0][0] * Rhs.M[0][1] + M[0][1] * Rhs.M[1][1] + M[0][2] * Rhs.M[2][1] + M[0][3] * Rhs.M[3][1], M[0][0] * Rhs.M[0][2] + M[0][1] * Rhs.M[1][2] + M[0][2] * Rhs.M[2][2] + M[0][3] * Rhs.M[3][2], M[0][0] * Rhs.M[0][3] + M[0][1] * Rhs.M[1][3] + M[0][2] * Rhs.M[2][3] + M[0][3] * Rhs.M[3][3],
				   M[1][0] * Rhs.M[0][0] + M[1][1] * Rhs.M[1][0] + M[1][2] * Rhs.M[2][0] + M[1][3] * Rhs.M[3][0], M[1][0] * Rhs.M[0][1] + M[1][1] * Rhs.M[1][1] + M[1][2] * Rhs.M[2][1] + M[1][3] * Rhs.M[3][1], M[1][0] * Rhs.M[0][2] + M[1][1] * Rhs.M[1][2] + M[1][2] * Rhs.M[2][2] + M[1][3] * Rhs.M[3][2], M[1][0] * Rhs.M[0][3] + M[1][1] * Rhs.M[1][3] + M[1][2] * Rhs.M[2][3] + M[1][3] * Rhs.M[3][3],
		           M[2][0] * Rhs.M[0][0] + M[2][1] * Rhs.M[1][0] + M[2][2] * Rhs.M[2][0] + M[2][3] * Rhs.M[3][0], M[2][0] * Rhs.M[0][1] + M[2][1] * Rhs.M[1][1] + M[2][2] * Rhs.M[2][1] + M[2][3] * Rhs.M[3][1], M[2][0] * Rhs.M[0][2] + M[2][1] * Rhs.M[1][2] + M[2][2] * Rhs.M[2][2] + M[2][3] * Rhs.M[3][2], M[2][0] * Rhs.M[0][3] + M[2][1] * Rhs.M[1][3] + M[2][2] * Rhs.M[2][3] + M[2][3] * Rhs.M[3][3],
		           M[3][0] * Rhs.M[0][0] + M[3][1] * Rhs.M[1][0] + M[3][2] * Rhs.M[2][0] + M[3][3] * Rhs.M[3][0], M[3][0] * Rhs.M[0][1] + M[3][1] * Rhs.M[1][1] + M[3][2] * Rhs.M[2][1] + M[3][3] * Rhs.M[3][1], M[3][0] * Rhs.M[0][2] + M[3][1] * Rhs.M[1][2] + M[3][2] * Rhs.M[2][2] + M[3][3] * Rhs.M[3][2], M[3][0] * Rhs.M[0][3] + M[3][1] * Rhs.M[1][3] + M[3][2] * Rhs.M[2][3] + M[3][3] * Rhs.M[3][3]);
}

FVector4 FMatrix::TransformVector4(const FVector4& V) const
{
	return FVector4(
		V.X * M[0][0] +
		V.Y * M[1][0] +
		V.Z * M[2][0] +
		V.W * M[3][0],

		V.X * M[0][1] +
		V.Y * M[1][1] +
		V.Z * M[2][1] +
		V.W * M[3][1],

		V.X * M[0][2] +
		V.Y * M[1][2] +
		V.Z * M[2][2] +
		V.W * M[3][2],

		V.X * M[0][3] +
		V.Y * M[1][3] +
		V.Z * M[2][3] +
		V.W * M[3][3]
	);
}

FVector FMatrix::TransformPosition(const FVector& V) const
{
	FVector4 V4(V.X, V.Y, V.Z, 1.0f);
	FVector4 Result = TransformVector4(V4);
	return FVector(Result.X, Result.Y, Result.Z);
}

FMatrix FMatrix::Transpose() const
{
	return FMatrix(M[0][0], M[1][0], M[2][0], M[3][0],
				   M[0][1], M[1][1], M[2][1], M[3][1],
				   M[0][2], M[1][2], M[2][2], M[3][2],
				   M[0][3], M[1][3], M[2][3], M[3][3]);
}

bool FMatrix::TryInverse(FMatrix& OutInverse) const
{
    constexpr int Size = 4;
	
	// [원본 행렬 | 단위행렬] 형태의 첨가행렬
    float Augmented[Size][Size * 2] = {};

    for (int Row = 0; Row < Size; ++Row) {
        for (int Column = 0; Column < Size; Column++) {
            Augmented[Row][Column] = M[Row][Column];
            Augmented[Row][Column + Size] = (Row == Column) ? 1.0f : 0.0f;
        }
    }

    for (int PivotColumn = 0;PivotColumn < Size;++PivotColumn) {
        //부분피벗팅
        int PivotRow = PivotColumn;
        float MaxValue = std::fabs(Augmented[PivotRow][PivotColumn]);

		for (int Row = PivotColumn + 1; Row < Size;Row++) {
            const float Value =std::fabs(Augmented[Row][PivotColumn]);
            if (Value > MaxValue){
                MaxValue = Value;
                PivotRow = Row;
            }
        }

        // 피벗이 거의 0일때 역행렬 존재X
        if (MaxValue <= 1.0e-8f) {
            return false;
        }

        // 현재 행과 피벗 행 교환
        if (PivotRow != PivotColumn) {
			for (int Column = 0; Column < Size * 2;Column++) {
				std::swap(Augmented[PivotRow][Column], Augmented[PivotColumn][Column]);
            }
        }

        // 피벗을 1로 만든다.
        const float Pivot = Augmented[PivotColumn][PivotColumn];

        for (int Column = 0;Column < Size * 2;Column++) {
            Augmented[PivotColumn][Column] /= Pivot;
        }

        // 피벗 행을 제외한 나머지 행의 현재 열을 0으로 만든다.
        for (int Row = 0; Row < Size; Row++) {
            if (Row == PivotColumn){
                continue;
            }
            const float Factor = Augmented[Row][PivotColumn];

            for (int Column = 0; Column < Size * 2;Column++) {
				Augmented[Row][Column] -= Factor * Augmented[PivotColumn][Column];
            }
        }
    }

    // 붙였던 4x4단위행렬부분을 역행렬로 취함
    for (int Row = 0; Row < Size; ++Row)
    {
        for (int Column = 0; Column < Size; ++Column)
        {
            OutInverse.M[Row][Column] =
                Augmented[Row][Column + Size];
        }
    }

    return true;
}

FMatrix FMatrix::Inverse() const
{
	FMatrix Result{};

	const bool bSuccess = TryInverse(Result);

	// 역행렬이 없을때 알림
	assert(bSuccess && "Matrix is singular.");

	return Result;
}

float FMatrix::Determinant() const
{
	//일반적인 라플라스 전개를 통한 행렬식 계산
	// 상단 소행렬식
	const float C0 = M[0][0] * M[1][1] - M[0][1] * M[1][0];
	const float C1 = M[0][0] * M[1][2] - M[0][2] * M[1][0];
	const float C2 = M[0][0] * M[1][3] - M[0][3] * M[1][0];
	const float C3 = M[0][1] * M[1][2] - M[0][2] * M[1][1];
	const float C4 = M[0][1] * M[1][3] - M[0][3] * M[1][1];
	const float C5 = M[0][2] * M[1][3] - M[0][3] * M[1][2];

	// 하단 소행렬식
	const float S0 = M[2][0] * M[3][1] - M[2][1] * M[3][0];
	const float S1 = M[2][0] * M[3][2] - M[2][2] * M[3][0];
	const float S2 = M[2][0] * M[3][3] - M[2][3] * M[3][0];
	const float S3 = M[2][1] * M[3][2] - M[2][2] * M[3][1];
	const float S4 = M[2][1] * M[3][3] - M[2][3] * M[3][1];
	const float S5 = M[2][2] * M[3][3] - M[2][3] * M[3][2];

	

	// 2x2 소행렬식들의 여인수 전개
	return (C0 * S5 - C1 * S4 + C2 * S3 + C3 * S2 - C4 * S1 + C5 * S0);
}

FVector FMatrix::GetAxis(int32 AxisIndex) const
{
	assert(AxisIndex >= 0 && AxisIndex < 3);
	return FVector(M[AxisIndex][0], M[AxisIndex][1], M[AxisIndex][2]);
}

FVector FMatrix::GetOrigin() const
{
	return FVector(M[3][0], M[3][1], M[3][2]);
}

FMatrix FMatrix::NormalMatrix() const
{
	FMatrix Result = *this;
	Result.M[3][0] = 0.0f;
	Result.M[3][1] = 0.0f;
	Result.M[3][2] = 0.0f;
	Result.M[3][3] = 1.0f;
	return Result.Inverse().Transpose();
}

bool FMatrix::IsOrthogonal(float Epsilon) const
{
	FVector4 Temp1 = FVector4(M[0][0], M[0][1], M[0][2], M[0][3]);
	FVector4 Temp2 = FVector4(M[1][0], M[1][1], M[1][2], M[1][3]);
	FVector4 Temp3 = FVector4(M[2][0], M[2][1], M[2][2], M[2][3]);
	FVector4 Temp4 = FVector4(M[3][0], M[3][1], M[3][2], M[3][3]);
	
	return std::fabs(Temp1.Dot(Temp2)) <= Epsilon && std::fabs(Temp1.Dot(Temp3)) <= Epsilon && std::fabs(Temp1.Dot(Temp4)) <= Epsilon && std::fabs(Temp3.Dot(Temp2)) <= Epsilon && std::fabs(Temp4.Dot(Temp2)) <= Epsilon && std::fabs(Temp3.Dot(Temp4)) <= Epsilon;
}

bool FMatrix::IsOrthonormal(float Epsilon) const
{
	FVector4 Temp1 = FVector4(M[0][0], M[0][1], M[0][2], M[0][3]);
	FVector4 Temp2 = FVector4(M[1][0], M[1][1], M[1][2], M[1][3]);
	FVector4 Temp3 = FVector4(M[2][0], M[2][1], M[2][2], M[2][3]);
	FVector4 Temp4 = FVector4(M[3][0], M[3][1], M[3][2], M[3][3]);
	return IsOrthogonal(Epsilon)&&std::fabs(Temp1.Length()-1.0f)<=Epsilon&& fabs(Temp2.Length() - 1.0f) <= Epsilon&& fabs(Temp3.Length() - 1.0f) <= Epsilon&& fabs(Temp4.Length() - 1.0f) <= Epsilon;
}

FVector4 operator*(const FVector4& V, const FMatrix& Matrix)
{
	return Matrix.TransformVector4(V);
}
