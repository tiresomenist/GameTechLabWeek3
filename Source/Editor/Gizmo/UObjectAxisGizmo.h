#pragma once

#include "UGizmo.h"



class UObjectAxisGizmo : public UGizmo
{
	UCLASS(UObjectAxisGizmo, "ObjectAxisGizmo", UGizmo)

public:
	virtual void Initialize()override;
	bool UpdateTransform();
	UObjectAxisGizmo();
	TArray<FPrimitiveRenderData> GetRenderData() override;
	TArray<FPrimitiveRenderData> GetTranslateRenderData();
	TArray<FPrimitiveRenderData> GetRotateRenderData();
	TArray<FPrimitiveRenderData> GetScaleRenderData();
	FMatrix GetXAxisWorldMatirx()const;
	FMatrix GetYAxisWorldMatirx()const;
	FMatrix GetZAxisWorldMatirx()const;
	//void Set

	const TArray<FGizmoHandle>& GetHandles() const { return Handles; }
	void SetMode(EGizmoMode InMode);
private:
	// 모드 변경 시 메시에서 구한 축별 기준 길이(GizmoScale 반영).
	TArray<float> HandleBaseLengths;
	// 이동·스케일: 축 길이, 회전: 링 반지름
	float GizmoScreenHeightRatio = 0.15f;
	// X,Y,Z 핸들
	FVector GizmoScale = FVector(0.2f,0.2f,0.2f);
};

