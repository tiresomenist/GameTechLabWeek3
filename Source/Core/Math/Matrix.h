#pragma once

#include <cmath>
#include <cassert>
#include <algorithm>
#include "Core/Core.h"
#include "Core/Core.h"
#include "FVector.h"
//벡터, 행렬 구조체 선언


struct FQuaternion;

struct FMatrix
{
	//==================
	//문서에는 직교행렬, 정규 직교행렬 등을 구현하라고 하는데..
	//==================
	//M[행][열]
	float M[4][4];

	// 단위행렬
	static const FMatrix Identity;

	static FMatrix MakeTranslationMatrix(const FVector& Location);
	static FMatrix MakeScaleMatrix(const FVector& Scale);

    // Angle-based compatibility helpers convert through FQuaternion.
	static FMatrix MakeRotationXMatrix(float Radian);
	static FMatrix MakeRotationYMatrix(float Radian);
	static FMatrix MakeRotationZMatrix(float Radian);

	static FMatrix MakeRotationMatrix(const FVector& Rotation);
	static FMatrix MakeModelMatrix(
		const FVector& Location,
		const FQuaternion& Rotation,
		const FVector& Scale);

	//모델 행렬을 넣어서 법선 행렬을 만드는 함수
	static FMatrix MakeNormalMatrix(const FMatrix& model);

	//행렬곱
	FMatrix operator*(const FMatrix& Rhs) const;
	//벡터*행렬
	FVector4 TransformVector4(const FVector4& V) const;
	// 3D 위치(Position) 변환 (W=1.0)
	FVector TransformPosition(const FVector& V) const;

	//전치행렬,역행렬,행렬식
	FMatrix Transpose() const;
	//역행렬 시도.
	bool TryInverse(FMatrix& OutInverse) const;
	FMatrix Inverse() const;
	float Determinant() const;

	// 축 위치 추출
	FVector GetAxis(int32 AxisIndex) const; // 0: X축, 1: Y축, 2: Z축
	FVector GetOrigin() const;
	// 법선 행렬
	FMatrix NormalMatrix() const;

	//직교행렬인가? 정규직교행렬인가?
	bool IsOrthogonal(float Epsilon = EPSILON) const;
	bool IsOrthonormal(float Epsilon = EPSILON) const;

	FMatrix(float m00 = 1.0f, float m01 = 0.0f, float m02 = 0.0f, float m03 = 0.0f,
		float m10 = 0.0f, float m11 = 1.0f, float m12 = 0.0f, float m13 = 0.0f,
		float m20 = 0.0f, float m21 = 0.0f, float m22 = 1.0f, float m23 = 0.0f,
		float m30 = 0.0f, float m31 = 0.0f, float m32 = 0.0f, float m33 = 1.0f)
	{
		M[0][0] = m00; M[0][1] = m01; M[0][2] = m02; M[0][3] = m03;
		M[1][0] = m10; M[1][1] = m11; M[1][2] = m12; M[1][3] = m13;
		M[2][0] = m20; M[2][1] = m21; M[2][2] = m22; M[2][3] = m23;
		M[3][0] = m30; M[3][1] = m31; M[3][2] = m32; M[3][3] = m33;
	}


};

FVector4 operator*(
	//실제 연산은 FMatrix::TransformVector4에서 수행
	const FVector4& V,
	const FMatrix& Matrix);
