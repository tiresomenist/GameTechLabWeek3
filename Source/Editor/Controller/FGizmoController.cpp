#include "pch.h"
#include "Editor/Controller/FGizmoController.h"
#include "Editor/FEditor.h"
#include "Engine/Renderer/GDevice.h"
#include "Engine/Component/UCameraComponent.h"
#include <cmath>
#include "Engine/Input/GInputManager.h"
#include "Engine/Component/Primitive/UPrimitiveComponent.h"
#include "Editor/Gizmo/UObjectAxisGizmo.h"


namespace
{
	bool WorldToPixel(const FVector& Position,const FMatrix& ViewProjection,const D3D11_VIEWPORT& Viewport, FVector& OutPixel)
	{
		const FVector4 Clip = FVector4(Position, 1.0f) * ViewProjection;

		if (!std::isfinite(Clip.W) || Clip.W <= 1.0e-6f)
			return false;

		const float NDCX = Clip.X / Clip.W;
		const float NDCY = Clip.Y / Clip.W;

		OutPixel = FVector(
			Viewport.TopLeftX + (NDCX + 1.0f) * 0.5f * Viewport.Width,
			Viewport.TopLeftY + (1.0f - NDCY) * 0.5f * Viewport.Height,	0.0f
		);

		return std::isfinite(OutPixel.X)&& std::isfinite(OutPixel.Y);
	}
}

FGizmoController::FGizmoController(FEditor* InEditor)
    : Editor(InEditor)
{
    WorldDirections.SetNum(3);
    ScreenDirections.SetNum(3);
    UnitsPerPixel.SetNum(3);
    if (Editor != nullptr)
    {
        ObjectAxisGizmo =
            dynamic_cast<UObjectAxisGizmo*>(Editor->GetObjectAxisGizmo());
    }
}

FGizmoController::~FGizmoController()
{
}



void FGizmoController::CalculateAxis()
{
    for (int32 Axis = 0; Axis < 3; ++Axis)
    {
        WorldDirections[Axis] = FVector(0, 0, 0);
        ScreenDirections[Axis] = FVector(0, 0, 0);
        UnitsPerPixel[Axis] = 0.0f;
    }

    if (!Editor || !SelectedObject || !ObjectAxisGizmo) //뭔가 잘못되었으면
        return;

    UCameraComponent* Camera = Editor->GetEditorCamera();
    if (!Camera) return;    //카메라를 못받아왔으면

    const auto& Viewport = GDevice::GetInstance()->GetViewport();
    if (Viewport.Width <= 0.0f || Viewport.Height <= 0.0f) return; //창 크기가 0보다 작으면

    Camera->SetAspectRatio(Viewport.Width / Viewport.Height);

    FMatrix ViewProjection = Camera->GetViewMatrix() * Camera->GetProjectionMatrix();

    // 현재 프로젝트는 UpdateTransform()에서 축별 행렬을 갱신
    if (!ObjectAxisGizmo->UpdateTransform()) return;

    TArray<FMatrix> AxisMatrices = {
        ObjectAxisGizmo->GetXAxisWorldMatirx(),
        ObjectAxisGizmo->GetYAxisWorldMatirx(),
        ObjectAxisGizmo->GetZAxisWorldMatirx()
    };


    for (int32 Axis = 0; Axis < 3; ++Axis)
    {
        FVector WorldOrigin(FVector4(0, 0, 0, 1) * AxisMatrices[Axis]); //월드기준 화살표 원점

        FVector WorldEnd(FVector4(0, 0, 1, 1) * AxisMatrices[Axis]);    //월드기준 화살표 끝점

        FVector WorldDelta = WorldEnd - WorldOrigin;
        float WorldLength = WorldDelta.Length();

        if (!std::isfinite(WorldLength) || WorldLength <= 1.0e-6f)
            continue;

        WorldDirections[Axis] = WorldDelta / WorldLength;

        FVector ScreenOrigin;
        FVector ScreenEnd;

        //월드좌표계->화면좌표계
        if (!WorldToPixel(WorldOrigin, ViewProjection, Viewport, ScreenOrigin) ||
            !WorldToPixel(WorldEnd, ViewProjection, Viewport, ScreenEnd))
        {
            continue;
        }

        //화면기준 기즈모 방향
        const FVector ScreenDelta = ScreenEnd - ScreenOrigin;
        const float PixelLength = ScreenDelta.Length();

        // 축이 너무 짧아서 땡기기 힘들때
        if (!std::isfinite(PixelLength) || PixelLength < 2.0f)
            continue;

        ScreenDirections[Axis] = ScreenDelta / PixelLength;
        WorldDirections[Axis] = WorldDelta / WorldLength;
        UnitsPerPixel[Axis] = WorldLength / PixelLength;
    }
}

void FGizmoController::SetSelectedObject(USceneComponent* InObject)
{
	EndDrag();
	SelectedObject = InObject;
}

//에디터에서 드래그 입력 있고, 축 선택 성공시.
bool FGizmoController::BeginDrag(int32 Axis)
{
	EndDrag();
	if (Editor) ObjectAxisGizmo = dynamic_cast<UObjectAxisGizmo*>(Editor->GetObjectAxisGizmo());
	if (!SelectedObject || !ObjectAxisGizmo || Axis < 0 || Axis > 2) return false;

	CalculateAxis();
	if (Mode != EGizmoMode::Rotate && ScreenDirections[Axis].LengthSquared() <= 1.0e-6f) return false;
	if (WorldDirections[Axis].LengthSquared() <= 1.0e-6f) return false;

	ActiveAxis = Axis;
	StartObjectLocation = SelectedObject->GetRelativeLocation();
    StartObjectRotation = SelectedObject->GetRelativeRotation();
    StartObjectScale = SelectedObject->GetRelativeScale3D();
	DragPixels = FVector(0, 0, 0);
	DragScreenDirection = ScreenDirections[Axis];
	DragWorldDirection = WorldDirections[Axis];
	DragUnitsPerPixel = UnitsPerPixel[Axis];
    //회전 모드일때 추가 계산
	if (Mode == EGizmoMode::Rotate)
	{
        //객체의 월드 위치
		RotationPivot = SelectedObject->GetWorldMatrix().GetOrigin();
		//누적회전각
        AccumulatedAngle = 0.0;
		auto& Input = *GInputManager::GetInstance();
		
        bHasRotationDirection = GetRotationDirection(Input.GetLeftCursorX(), Input.GetLeftCursorY(), PreviousRotationDirection);
		if (!bHasRotationDirection) { EndDrag(); return false; }
	}
	bDragging = true;
	return true;
}

void FGizmoController::EndDrag()
{
	bDragging = false;
	ActiveAxis = -1;
	bHasRotationDirection = false;
}

void FGizmoController::ChangeMod()
{
    EndDrag();
    if (!ObjectAxisGizmo) return;
    switch (Mode)
    {
    case EGizmoMode::Translate:
        Mode = EGizmoMode::Rotate;
        break;

    case EGizmoMode::Rotate:
        Mode = EGizmoMode::Scale;
        break;

    case EGizmoMode::Scale:
        Mode = EGizmoMode::Translate;
        break;
    }
    ObjectAxisGizmo->SetMode(Mode);
}

void FGizmoController::Tick()
{
	auto& Input = *GInputManager::GetInstance();
	int32 DeltaX = 0, DeltaY = 0;
	Input.ConsumeLeftDragDelta(DeltaX, DeltaY);

	if (!bDragging) return;
	if (!SelectedObject || !ObjectAxisGizmo)
	{
		EndDrag();
		return;
	}

	DragPixels.X += float(DeltaX);
	DragPixels.Y += float(DeltaY);

    const float Distance = DragPixels.Dot(DragScreenDirection) * DragUnitsPerPixel;
    //축방향으로 움직인 값. 내적으로 구함
    const float AlongPixels = DragPixels.Dot(DragScreenDirection);
    constexpr float PixelsPerScaleUnit = 100.0f;

    //스케일 변화량
    const float Factor = (std::max)(0.01f, 1.0f + AlongPixels / PixelsPerScaleUnit);

    FVector NewScale = StartObjectScale;

    switch (Mode)
    {
    case EGizmoMode::Translate:
        SelectedObject->SetRelativeLocation(StartObjectLocation + DragWorldDirection * Distance);
        break;

    case EGizmoMode::Rotate:
    {
        const auto& Viewport = GDevice::GetInstance()->GetViewport();
        FVector Direction;
        if (Viewport.Width <= 0.0f || Viewport.Height <= 0.0f) break;
        const float X = 2.0f * (Input.GetLeftCursorPixelX() - Viewport.TopLeftX) / Viewport.Width - 1.0f;
        const float Y = 1.0f - 2.0f * (Input.GetLeftCursorPixelY() - Viewport.TopLeftY) / Viewport.Height;
        if (!GetRotationDirection(X, Y, Direction))
        {
            bHasRotationDirection = false;
            break;
        }
        if (bHasRotationDirection)
        {
            const float SinAngle = DragWorldDirection.Dot(PreviousRotationDirection.Cross(Direction));
            const float CosAngle = PreviousRotationDirection.Dot(Direction);
            AccumulatedAngle += std::atan2(SinAngle, CosAngle);
            const auto DeltaRotation = FQuaternion::FromAxisAngle(DragWorldDirection, float(std::remainder(AccumulatedAngle, 2.0 * PI)));
            SelectedObject->SetRelativeRotation(DeltaRotation * StartObjectRotation);
        }
        PreviousRotationDirection = Direction;
        bHasRotationDirection = true;
        break;
    }
    case EGizmoMode::Scale:
        switch (ActiveAxis)
        {
        case 0: NewScale.X *= Factor; break;
        case 1: NewScale.Y *= Factor; break;
        case 2: NewScale.Z *= Factor; break;
        }
        SelectedObject->SetRelativeScale3D(NewScale);
        break;
    }

	if (!Input.GetKey(GInputManager::EI_LMOUSE)) EndDrag();
}

bool FGizmoController::GetRotationDirection(float NDCX, float NDCY, FVector& OutDirection) const
{
    // 뭔가 잘못되었으면 리턴
    if (!Editor || !Editor->GetEditorCamera()) return false;    
    const auto& Viewport = GDevice::GetInstance()->GetViewport();
    if (Viewport.Width <= 0.0f || Viewport.Height <= 0.0f) return false;
    auto* Camera = Editor->GetEditorCamera();
    Camera->SetAspectRatio(Viewport.Width / Viewport.Height);

    //VP행렬의 역행렬
    FMatrix Inverse;
    if (!(Camera->GetViewMatrix() * Camera->GetProjectionMatrix()).TryInverse(Inverse)) return false;
    const FVector4 Near = FVector4(NDCX, NDCY, 0, 1) * Inverse;
    const FVector4 Far = FVector4(NDCX, NDCY, 1, 1) * Inverse;
    if (!std::isfinite(Near.W) || !std::isfinite(Far.W) || std::fabs(Near.W) < 1.0e-6f || std::fabs(Far.W) < 1.0e-6f) return false;
    
    // 클릭->월드 레이로 변환
    const FVector Origin = FVector(Near) / Near.W;
    const FVector Direction = (FVector(Far) / Far.W - Origin).GetNormalized();
    
    const float Denominator = Direction.Dot(DragWorldDirection);
    if (!std::isfinite(Denominator) || std::fabs(Denominator) < 1.0e-4f) return false;

    const float T = (RotationPivot - Origin).Dot(DragWorldDirection) / Denominator;
    if (!std::isfinite(T) || T < 0.0f) return false;

    const FVector Radial = Origin + Direction * T - RotationPivot;
    const float LengthSquared = Radial.LengthSquared();

    if (!std::isfinite(LengthSquared) || LengthSquared < 1.0e-8f) return false;

    OutDirection = Radial / std::sqrt(LengthSquared);
    return true;
}


