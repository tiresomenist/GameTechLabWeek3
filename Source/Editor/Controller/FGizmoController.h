#pragma once
#include "Core/Core.h"
#include "Core/Math/FVector.h"
#include "Core/Container/TArray.h"
#include "Editor/Gizmo/EGizmoMode.h"
#include "Core/Math/FQuaternion.h"

class FEditor;
class USceneComponent;
class UObjectAxisGizmo;

class FGizmoController
{
public:
	FGizmoController(FEditor* InEditor);
	~FGizmoController();
	void CalculateAxis();

	void SetSelectedObject(USceneComponent* InObject);
	void Tick();

	bool BeginDrag(int32 Axis);
	void EndDrag();
	bool IsDragging() const { return bDragging; }
	int32 GetActiveAxis() const { return bDragging ? ActiveAxis : -1; }
	void ChangeMod();
private:
	bool GetRotationDirection(float NDCX, float NDCY, FVector& OutDirection) const;
	FVector RotationPivot;
	FVector PreviousRotationDirection;
	double AccumulatedAngle = 0.0;
	bool bHasRotationDirection = false;
	USceneComponent* SelectedObject = nullptr;
	
	UObjectAxisGizmo* ObjectAxisGizmo = nullptr;
	
	FEditor* Editor = nullptr;
	bool bDragging = false;
	int32 ActiveAxis = -1;

	FVector StartObjectLocation;
	FVector StartObjectScale;
	FQuaternion StartObjectRotation;

	FVector DragPixels = FVector(0, 0, 0);

	// CalculateAxis()에서 축별로 계산
	TArray<FVector> WorldDirections;	//오브젝트 움직일 월드방향
	TArray<FVector> ScreenDirections; // 마우스 변위와 내적할 화면 방향
	TArray<float> UnitsPerPixel = {};

	EGizmoMode Mode = EGizmoMode::Translate; //0-Location,1-Rotation, 2-Scale
	

	// 드래그 시작 시 선택 축의 값을 고정.
	FVector DragWorldDirection;
	FVector DragScreenDirection;
	float DragUnitsPerPixel = 0.0f;
};
