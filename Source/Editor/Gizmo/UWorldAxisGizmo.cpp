#include "pch.h"
#include "Core/Core.h"
#include "UWorldAxisGizmo.h"

TArray<FPrimitiveRenderData> UWorldAxisGizmo::GetRenderData()
{
	return TArray<FPrimitiveRenderData>();
}

FLineDrawRequest UWorldAxisGizmo::BuildLineDrawRequest(const FGrid& Grid, const FVector& CameraPosition) const
{
    FLineDrawRequest Request;

    if (!std::isfinite(Grid.Interval) || Grid.Interval < FGrid::MinInterval ||!std::isfinite(Grid.Extent) ||
        Grid.Extent <= 0.0f ||!std::isfinite(CameraPosition.X) ||!std::isfinite(CameraPosition.Y) ||!std::isfinite(CameraPosition.Z))
    {
        return Request;
    }

    const double Interval = Grid.Interval;
    const double Half = Grid.Extent;

    const double CenterX = std::floor(CameraPosition.X / Interval) * Interval;
    const double CenterY = std::floor(CameraPosition.Y / Interval) * Interval;

    const float MinX = static_cast<float>(CenterX - Half);
    const float MaxX = static_cast<float>(CenterX + Half);
    const float MinY = static_cast<float>(CenterY - Half);
    const float MaxY = static_cast<float>(CenterY + Half);

    const float MinZ = static_cast<float>(CameraPosition.Z - Half * 2.0);
    const float MaxZ = static_cast<float>(CameraPosition.Z + Half * 2.0);

    Request.Vertices = {
        { MinX, 0, 0, 1, 0, 0, 1 },
        { MaxX, 0, 0, 1, 0, 0, 1 },
        { 0, MinY, 0, 0, 1, 0, 1 },
        { 0, MaxY, 0, 0, 1, 0, 1 },
        { 0, 0, MinZ, 0, 0, 1, 1 },
        { 0, 0, MaxZ, 0, 0, 1, 1 }
    };

    Request.Indices = { 0, 1, 2, 3, 4, 5 };

    return Request;
}