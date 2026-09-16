#pragma once

#include "UGizmo.h"
#include "Core/Container/TArray.h"
#include "Engine/Renderer/FPrimitiveRenderData.h"

class UWorldAxisGizmo : public UGizmo
{

	UCLASS(UWorldAxisGizmo, "WorldAxisGizmo", UGizmo)

public:

	virtual TArray<FPrimitiveRenderData> GetRenderData() override;
	FLineDrawRequest BuildLineDrawRequest(const FGrid& Grid,const FVector& CameraPosition) const override;
};

