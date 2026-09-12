#pragma once

#include "UGizmo.h"

class UWorldGridGizmo : public UGizmo
{

    UCLASS(UWorldGridGizmo, "WorldGridGizmo", UGizmo)

public:
    void AppendLineRequests(const FVector& CameraPosition,
        const FMatrix& ViewProjection, TArray<FLineDrawRequest>& OutRequests) const override;

    void SetGridExtent(float InExtent);
    void SetGridSpace(float InSpace);
    float GetGridExtent() const;
    float GetGridSpace() const;

private:
    // World-space half-width, independent of the distance between grid lines.
    float GridExtent = 120.0f;
    float GridSpace = 1.0f;
};

