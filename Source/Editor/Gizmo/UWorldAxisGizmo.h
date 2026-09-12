#pragma once

#include "UGizmo.h"
#include "Core/Container/TArray.h"
#include "Engine/Renderer/FPrimitiveRenderData.h"

class UWorldAxisGizmo : public UGizmo
{

	UCLASS(UWorldAxisGizmo, "WorldAxisGizmo", UGizmo)

public:

	virtual TArray<FPrimitiveRenderData> GetRenderData() override;
    void AppendLineRequests(const FVector& CameraPosition,
        const FMatrix& ViewProjection, TArray<FLineDrawRequest>& OutRequests) const override;

};

