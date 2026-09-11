#pragma once
#include "Core/Container/TArray.h"
#include "Editor/FEditor.h"
#include "Engine/Renderer/FPrimitiveRenderData.h"
#include "Engine/Object/UObject.h"
#include "Engine/Resource/GResourceManager.h"
#include "Editor/Gizmo/EGizmoMode.h"

struct FGizmoHandle
{
	int32 Axis = -1;       // 어느 축이 선택됐는가?
	FMeshResource* Mesh = nullptr;   // 무엇을 그리는가?
	FMatrix WorldMatrix = FMatrix::Identity;  // 어디에 어떻게 놓는가?
	int32 Topology = -1; //어떤식으로 그려지는가? 0:LINELIST 1:TRIANGLELIST
	bool IsSeleted = false;
};

class UGizmo : public UObject
{

    UCLASS(UGizmo, "Gizmo", UObject)

protected:
	EGizmoMode Mode = EGizmoMode::Translate;
	FEditor* Editor = nullptr;
	TArray<FGizmoHandle> Handles;
public:

	void Initialize(FEditor* InEditor);
	const TArray<FGizmoHandle>& GetHandles() const { return Handles; }

	// 렌더러에게 전달할 렌더 정보
	virtual TArray<FPrimitiveRenderData> GetRenderData();
	TArray<FMeshResource*> GetMeshResources() const {
	TArray< FMeshResource*> GizmoArray;
	for (auto& handle : GetHandles()) {
		GizmoArray.Add(handle.Mesh);
	}
		return GizmoArray;
	}

};
