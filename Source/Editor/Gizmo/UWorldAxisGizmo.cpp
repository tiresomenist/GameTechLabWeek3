#include "pch.h"
#include "UWorldAxisGizmo.h"
#include "Engine/Renderer/Line/FLineDrawRequest.h"
#include <cmath>
#include <limits>

namespace
{
    // Positive world ray P(t)=Axis*t, t>=0. Clip against all six D3D frustum planes.
    bool GetVisibleAxisRange(const FMatrix& ViewProjection, const FVector& Axis,
        float& OutStart, float& OutEnd)
    {
        const FVector4 Origin = ViewProjection.TransformVector4(FVector4(0, 0, 0, 1));
        const FVector4 Direction = ViewProjection.TransformVector4(FVector4(Axis, 0));
        double Start = 0.0;
        double End = (std::numeric_limits<double>::infinity)();
        const auto ClipPlane = [&](double Constant, double Slope)
        {
            // Keep Constant + Slope*t >= 0.
            if (!std::isfinite(Constant) || !std::isfinite(Slope)) return false;
            if (Slope == 0.0) return Constant >= 0.0;
            const double Intersection = -Constant / Slope;
            if (Slope > 0.0) Start = (std::max)(Start, Intersection);
            else End = (std::min)(End, Intersection);
            return Start <= End;
        };
        // Clip coordinates satisfy -w<=x,y<=w and 0<=z<=w.
        if (!ClipPlane(double(Origin.W) + Origin.X, double(Direction.W) + Direction.X) ||
            !ClipPlane(double(Origin.W) - Origin.X, double(Direction.W) - Direction.X) ||
            !ClipPlane(double(Origin.W) + Origin.Y, double(Direction.W) + Direction.Y) ||
            !ClipPlane(double(Origin.W) - Origin.Y, double(Direction.W) - Direction.Y) ||
            !ClipPlane(Origin.Z, Direction.Z) ||
            !ClipPlane(double(Origin.W) - Origin.Z, double(Direction.W) - Direction.Z) ||
            !std::isfinite(End) || End > (std::numeric_limits<float>::max)() || Start >= End)
            return false;
        OutStart = static_cast<float>(Start);
        OutEnd = static_cast<float>(End);
        return OutStart < OutEnd;
    }
}

TArray<FPrimitiveRenderData> UWorldAxisGizmo::GetRenderData()
{
    return TArray<FPrimitiveRenderData>();
}

void UWorldAxisGizmo::AppendLineRequests(const FVector&,
    const FMatrix& ViewProjection, TArray<FLineDrawRequest>& OutRequests) const
{
    const FVector Axes[] = { FVector(1, 0, 0), FVector(0, 1, 0), FVector(0, 0, 1) };
    for (const FVector& Axis : Axes)
    {
        float Start = 0, End = 0;
        if (GetVisibleAxisRange(ViewProjection, Axis, Start, End))
        {
            OutRequests.Add({
                { Axis * Start, Axis * End }, { 0, 1 },
                Axis.X, Axis.Y, Axis.Z, 1.0f
            });
        }
    }
}
