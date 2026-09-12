#include "pch.h"
#include "UWorldGridGizmo.h"
#include "Engine/Renderer/FVertexSimple.h"
#include "Engine/Renderer/Line/FLineDrawRequest.h"
#include <cmath>
#include <limits>
#include <stdexcept>

void UWorldGridGizmo::AppendLineRequests(const FVector& CameraPosition,
    const FMatrix&, TArray<FLineDrawRequest>& OutRequests) const
{
    if (!std::isfinite(GridSpace) || GridSpace <= 0.0f ||
        !std::isfinite(GridExtent) || GridExtent <= 0.0f ||
        !std::isfinite(CameraPosition.X) || !std::isfinite(CameraPosition.Y)) return;

    // Keep a fixed world-space rectangle; only line density changes with spacing.
    const double MinX = double(CameraPosition.X) - GridExtent;
    const double MaxX = double(CameraPosition.X) + GridExtent;
    const double MinY = double(CameraPosition.Y) - GridExtent;
    const double MaxY = double(CameraPosition.Y) + GridExtent;
    const double FirstX = std::ceil(MinX / GridSpace);
    const double LastX = std::floor(MaxX / GridSpace);
    const double FirstY = std::ceil(MinY / GridSpace);
    const double LastY = std::floor(MaxY / GridSpace);
    const double ColumnCount = (std::max)(LastX - FirstX + 1.0, 0.0);
    const double RowCount = (std::max)(LastY - FirstY + 1.0, 0.0);
    const double PointCount = (ColumnCount + RowCount) * 2.0;
    if (!std::isfinite(PointCount) || PointCount > (std::numeric_limits<int>::max)() ||
        PointCount > (std::numeric_limits<UINT>::max)() / sizeof(FVertexSimple))
        throw std::length_error("Grid exceeds line buffer size limits");
    if (PointCount == 0) return;

    FLineDrawRequest Request;
    Request.Points.SetNum(static_cast<size_t>(PointCount));
    Request.Indices.SetNum(static_cast<size_t>(PointCount));
    Request.R = Request.G = Request.B = 0.3f;
    uint32 Base = 0;
    const auto AddSegment = [&](const FVector& Start, const FVector& End)
    {
        if (Start.X == End.X && Start.Y == End.Y) return;
        Request.Points[Base] = Start;
        Request.Points[Base + 1] = End;
        Request.Indices[Base] = Base;
        Request.Indices[Base + 1] = Base + 1;
        Base += 2;
    };

    for (uint32 Row = 0; Row < static_cast<uint32>(RowCount); ++Row)
    {
        const double CellY = FirstY + Row;
        const float Y = static_cast<float>(CellY * GridSpace);
        // Positive world axes have their own colored requests.
        const double EndX = CellY == 0.0 ? (std::min)(MaxX, 0.0) : MaxX;
        if (MinX < EndX)
            AddSegment(FVector(float(MinX), Y, 0), FVector(float(EndX), Y, 0));
    }
    for (uint32 Column = 0; Column < static_cast<uint32>(ColumnCount); ++Column)
    {
        const double CellX = FirstX + Column;
        const float X = static_cast<float>(CellX * GridSpace);
        const double EndY = CellX == 0.0 ? (std::min)(MaxY, 0.0) : MaxY;
        if (MinY < EndY)
            AddSegment(FVector(X, float(MinY), 0), FVector(X, float(EndY), 0));
    }
    Request.Points.SetNum(Base);
    Request.Indices.SetNum(Base);
    if (Base != 0) OutRequests.Add(Request);
}

void UWorldGridGizmo::SetGridExtent(float InExtent)
{
    if (std::isfinite(InExtent) && InExtent > 0.0f) GridExtent = InExtent;
}

void UWorldGridGizmo::SetGridSpace(float InSpace)
{
    if (std::isfinite(InSpace) && InSpace > 0.0f) GridSpace = InSpace;
}

float UWorldGridGizmo::GetGridExtent() const
{
    return GridExtent;
}

float UWorldGridGizmo::GetGridSpace() const
{
    return GridSpace;
}
