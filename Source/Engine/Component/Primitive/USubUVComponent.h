#pragma once

#include "UPrimitiveComponent.h"

class USubUVComponent : public UPrimitiveComponent
{
	UCLASS(USubUVComponent, "SubUV", UPrimitiveComponent)

public:
	virtual void Initialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual FPrimitiveRenderData CreateRenderData(bool bSelected = false) const override;
	virtual FMeshResource* GetMeshResource() const override;
	virtual const FMatrix& GetRenderWorldMatrix(const UCameraComponent* Camera) const override;

private:
	static constexpr uint32 Columns = 6;
	static constexpr uint32 Rows = 6;
	static constexpr float FramesPerSecond = 20.0f;

	float AnimationTime = 0.0f;
	uint32 FrameIndex = 0;
	mutable FMatrix BillboardWorldMatrix;
};
