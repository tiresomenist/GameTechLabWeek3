#include "pch.h"
#include "UObjectAxisGizmo.h"
#include "Engine/Object/USceneComponent.h"
#include "Engine/GResourceManager.h"

#include "Engine/GDevice.h"
#include "Engine/Object/UCameraComponent.h"
#include <cmath>

//UObjectAxisGizmo::UObjectAxisGizmo()

void UObjectAxisGizmo::Initialize()
{
	Handles = {
	{ 0, nullptr, FMatrix::MakeRotationYMatrix(PI / 2) },
	{ 1, nullptr, FMatrix::MakeRotationXMatrix(-PI / 2) },
	{ 2, nullptr, FMatrix::Identity }
	};
	Mode = EGizmoMode::Translate;
	SetMode(Mode);
}

bool UObjectAxisGizmo::UpdateTransform() {
	if (Editor == nullptr || Handles.Num() != 3) return false;

	auto* SelectedObject = Editor->GetSelectedSceneComponent();
	auto* Camera = Editor->GetEditorCamera();

	if (!SelectedObject || !Camera)	return false;

	const auto& Viewport = GDevice::GetInstance()->GetViewport();

	if (!std::isfinite(Viewport.Width) ||!std::isfinite(Viewport.Height) ||	Viewport.Width <= 0.0f ||Viewport.Height <= 0.0f)
	{
		return false;
	}

	const FVector WorldLocation =SelectedObject->GetWorldMatrix().GetOrigin();

	const FVector4 ViewPosition = FVector4(WorldLocation, 1.0f) * Camera->GetViewMatrix();

	const float ViewDepth = ViewPosition.X;

	if (!std::isfinite(ViewDepth) ||ViewDepth <= Camera->GetNearZ() ||ViewDepth >= Camera->GetFarZ())
	{
		return false;
	}

	float VisibleWorldHeight;

	if (Camera->GetIsPerspective())
	{
		const float FOV = Camera->GetFOV();

		if (!std::isfinite(FOV) || FOV <= 0.0f || FOV >= PI) return false;

		VisibleWorldHeight = 2.0f * ViewDepth * std::tan(FOV * 0.5f);
	}
	else
	{
		VisibleWorldHeight = Camera->GetOrthoHeight();
	}

	if (!std::isfinite(VisibleWorldHeight) ||VisibleWorldHeight <= 0.0f)
	{
		return false;
	}

	const float TargetPixels = Viewport.Height * GizmoScreenHeightRatio;

	const float WorldUnitsPerPixel = VisibleWorldHeight / Viewport.Height;

	if (HandleBaseLengths.Num() != 3) return false;
	TArray<FMatrix> ScaleMatrices;
	for (const float BaseLength : HandleBaseLengths)
	{
		if (!std::isfinite(BaseLength) || BaseLength <= 0.0f) return false;
		const float DisplayScale = WorldUnitsPerPixel * TargetPixels / BaseLength;
		if (!std::isfinite(DisplayScale) || DisplayScale <= 0.0f) return false;
		ScaleMatrices.Add(FMatrix::MakeScaleMatrix(GizmoScale * DisplayScale));
	}
	
	const FMatrix TranslationMatrix = FMatrix::MakeTranslationMatrix(WorldLocation);

	FMatrix RotationMatirx = FMatrix::Identity;
	if (Mode == EGizmoMode::Scale) {
		RotationMatirx = SelectedObject->GetRelativeRotation().ToRotationMatrix();
	}

	Handles[0].WorldMatrix = ScaleMatrices[0] * FMatrix::MakeRotationYMatrix(PI / 2) * RotationMatirx * TranslationMatrix;
	Handles[1].WorldMatrix = ScaleMatrices[1] * FMatrix::MakeRotationXMatrix(-PI / 2) * RotationMatirx * TranslationMatrix;
	Handles[2].WorldMatrix = ScaleMatrices[2] * RotationMatirx * TranslationMatrix;

	return true;
}

TArray<FPrimitiveRenderData> UObjectAxisGizmo::GetRenderData()
{
	switch (Mode)
	{
	case EGizmoMode::Translate:
		return GetTranslateRenderData();

	case EGizmoMode::Rotate:
		return GetRotateRenderData();

	case EGizmoMode::Scale:
		return GetScaleRenderData();
	}

	return {};
}

TArray<FPrimitiveRenderData> UObjectAxisGizmo::GetTranslateRenderData()
{
	TArray<FPrimitiveRenderData> Result;

	if (!UpdateTransform()) {
		return Result;
	}
	FMeshResource* Mesh;
	Mesh = Handles[0].Mesh;
	if (Mesh != nullptr)
	{
		FPrimitiveRenderData Data;
		Data.VertexBuffer = Mesh->VertexBuffer;
		Data.IndexBuffer = Mesh->IndexBuffer;
		Data.Stride = Mesh->Stride;
		Data.IndexCount = Mesh->IndexCount;
		Data.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		// UObjectAxisGizmo의 멤버 행렬
		Data.WorldMatrix = &Handles[0].WorldMatrix;
		Data.isSelected = (Editor->GetActiveGizmoAxis() == Handles[0].Axis);

		Result.Add(Data);
	}
	Mesh = Handles[1].Mesh;
	if (Mesh != nullptr)
	{
		FPrimitiveRenderData Data;
		Data.VertexBuffer = Mesh->VertexBuffer;
		Data.IndexBuffer = Mesh->IndexBuffer;
		Data.Stride = Mesh->Stride;
		Data.IndexCount = Mesh->IndexCount;
		Data.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		// UObjectAxisGizmo의 멤버 행렬
		Data.WorldMatrix = &Handles[1].WorldMatrix;
		Data.isSelected = (Editor->GetActiveGizmoAxis() == Handles[1].Axis);

		Result.Add(Data);
	}
	Mesh = Handles[2].Mesh;
	if (Mesh != nullptr)
	{
		FPrimitiveRenderData Data;
		Data.VertexBuffer = Mesh->VertexBuffer;
		Data.IndexBuffer = Mesh->IndexBuffer;
		Data.Stride = Mesh->Stride;
		Data.IndexCount = Mesh->IndexCount;
		Data.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		// UObjectAxisGizmo의 멤버 행렬
		Data.WorldMatrix = &Handles[2].WorldMatrix;
		Data.isSelected = (Editor->GetActiveGizmoAxis() == Handles[2].Axis);

		Result.Add(Data);
	}
	return Result;
}

TArray<FPrimitiveRenderData> UObjectAxisGizmo::GetRotateRenderData()
{
	TArray<FPrimitiveRenderData> Result;

	if (!UpdateTransform()) {
		return Result;
	}
	FMeshResource* Mesh;
	Mesh = Handles[0].Mesh;
	if (Mesh != nullptr)
	{
		FPrimitiveRenderData Data;
		Data.VertexBuffer = Mesh->VertexBuffer;
		Data.IndexBuffer = Mesh->IndexBuffer;
		Data.Stride = Mesh->Stride;
		Data.IndexCount = Mesh->IndexCount;
		Data.Topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		// UObjectAxisGizmo의 멤버 행렬
		Data.WorldMatrix = &Handles[0].WorldMatrix;
		Data.isSelected = (Editor->GetActiveGizmoAxis() == Handles[0].Axis);

		Result.Add(Data);
	}
	Mesh = Handles[1].Mesh;
	if (Mesh != nullptr)
	{
		FPrimitiveRenderData Data;
		Data.VertexBuffer = Mesh->VertexBuffer;
		Data.IndexBuffer = Mesh->IndexBuffer;
		Data.Stride = Mesh->Stride;
		Data.IndexCount = Mesh->IndexCount;
		Data.Topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		// UObjectAxisGizmo의 멤버 행렬
		Data.WorldMatrix = &Handles[1].WorldMatrix;
		Data.isSelected = (Editor->GetActiveGizmoAxis() == Handles[1].Axis);

		Result.Add(Data);
	}
	Mesh = Handles[2].Mesh;
	if (Mesh != nullptr)
	{
		FPrimitiveRenderData Data;
		Data.VertexBuffer = Mesh->VertexBuffer;
		Data.IndexBuffer = Mesh->IndexBuffer;
		Data.Stride = Mesh->Stride;
		Data.IndexCount = Mesh->IndexCount;
		Data.Topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

		// UObjectAxisGizmo의 멤버 행렬
		Data.WorldMatrix = &Handles[2].WorldMatrix;
		Data.isSelected = (Editor->GetActiveGizmoAxis() == Handles[2].Axis);

		Result.Add(Data);
	}
	return Result;

}

TArray<FPrimitiveRenderData> UObjectAxisGizmo::GetScaleRenderData()
{
	TArray<FPrimitiveRenderData> Result;

	if (!UpdateTransform()) {
		return Result;
	}
	FMeshResource* Mesh;
	Mesh = Handles[0].Mesh;
	if (Mesh != nullptr)
	{
		FPrimitiveRenderData Data;
		Data.VertexBuffer = Mesh->VertexBuffer;
		Data.IndexBuffer = Mesh->IndexBuffer;
		Data.Stride = Mesh->Stride;
		Data.IndexCount = Mesh->IndexCount;
		Data.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		// UObjectAxisGizmo의 멤버 행렬
		Data.WorldMatrix = &Handles[0].WorldMatrix;
		Data.isSelected = (Editor->GetActiveGizmoAxis() == Handles[0].Axis);

		Result.Add(Data);
	}
	Mesh = Handles[1].Mesh;
	if (Mesh != nullptr)
	{
		FPrimitiveRenderData Data;
		Data.VertexBuffer = Mesh->VertexBuffer;
		Data.IndexBuffer = Mesh->IndexBuffer;
		Data.Stride = Mesh->Stride;
		Data.IndexCount = Mesh->IndexCount;
		Data.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		// UObjectAxisGizmo의 멤버 행렬
		Data.WorldMatrix = &Handles[1].WorldMatrix;
		Data.isSelected = (Editor->GetActiveGizmoAxis() == Handles[1].Axis);

		Result.Add(Data);
	}
	Mesh = Handles[2].Mesh;
	if (Mesh != nullptr)
	{
		FPrimitiveRenderData Data;
		Data.VertexBuffer = Mesh->VertexBuffer;
		Data.IndexBuffer = Mesh->IndexBuffer;
		Data.Stride = Mesh->Stride;
		Data.IndexCount = Mesh->IndexCount;
		Data.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		// UObjectAxisGizmo의 멤버 행렬
		Data.WorldMatrix = &Handles[2].WorldMatrix;
		Data.isSelected = (Editor->GetActiveGizmoAxis() == Handles[2].Axis);

		Result.Add(Data);
	}
	return Result;

}

FMatrix UObjectAxisGizmo::GetXAxisWorldMatirx() const
{
	return Handles[0].WorldMatrix;
}

FMatrix UObjectAxisGizmo::GetYAxisWorldMatirx() const
{
	return Handles[1].WorldMatrix;
}

FMatrix UObjectAxisGizmo::GetZAxisWorldMatirx() const
{
	return Handles[2].WorldMatrix;
}

void UObjectAxisGizmo::SetMode(EGizmoMode InMode)
{
	Mode = InMode;
	TArray<FString> Names;
	switch (Mode)
	{
	case EGizmoMode::Translate: Names = { "ArrowRed", "ArrowGreen", "ArrowBlue" }; break;
	case EGizmoMode::Rotate: Names = { "RotateRed", "RotateGreen", "RotateBlue" }; break;
	case EGizmoMode::Scale: Names = { "ScaleRed", "ScaleGreen", "ScaleBlue" }; break;
	}
	if (Names.Num() != 3 || Handles.Num() != 3) return;

	HandleBaseLengths = { 0.0f, 0.0f, 0.0f };
	for (int32 Axis = 0; Axis < 3; ++Axis) {

		Handles[Axis].Mesh = GResourceManager::GetInstance()->GetPrimitive(Names[Axis]);
		Handles[Axis].Topology = Mode == EGizmoMode::Rotate ? 0 : 1;

		const auto* Mesh = Handles[Axis].Mesh;
		if (!Mesh) continue;
		// 이동/스케일은 원점부터 Z축 끝까지, 회전은 XY 평면의 반지름.
		// 메시 정점 순회는 모드 변경 때만 수행한다.
		for (const auto& Vertex : Mesh->vertexs)
		{
			const float Length = Mode == EGizmoMode::Rotate
				? std::hypot(Vertex.x * GizmoScale.X, Vertex.y * GizmoScale.Y)
				: std::fabs(Vertex.z * GizmoScale.Z);
			if (!std::isfinite(Length))
			{
				HandleBaseLengths[Axis] = 0.0f;
				break;
			}
			if (Length > HandleBaseLengths[Axis]) HandleBaseLengths[Axis] = Length;
		}
	}
		
		
}
