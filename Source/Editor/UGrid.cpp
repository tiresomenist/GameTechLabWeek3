#include "pch.h"
#include "UGrid.h"
#include "Engine/Object/UObject.h"
#include "Engine/Object/FClassType.h"
#include "Engine/Renderer/FVertexSimple.h"
#include "Engine/Resource/GResourceManager.h"

void UGrid::Initialize(FEditor* InEditor)
{
	Editor = InEditor;
	MeshResource = GResourceManager::GetInstance()->GetPrimitive("Grid");
}

TArray<FPrimitiveRenderData> UGrid::GetRenderData()
{
	return TArray<FPrimitiveRenderData>();
}

FLineDrawRequest UGrid::BuildLineDrawRequest(const FGrid& Grid, const FVector& CameraPosition) const
{
    FLineDrawRequest Request;

    if (!std::isfinite(Grid.Interval) || Grid.Interval < FGrid::MinInterval || !std::isfinite(Grid.Extent) ||
        Grid.Extent <= 0.0f || !std::isfinite(CameraPosition.X) || !std::isfinite(CameraPosition.Y))
    {
        return Request;
    }

    const double Interval = Grid.Interval;
    const double Half = Grid.Extent;

    const double CenterX = std::floor(CameraPosition.X / Interval) * Interval;
    const double CenterY = std::floor(CameraPosition.Y / Interval) * Interval;

    const double FirstX = std::ceil((CenterX - Half) / Interval);
    const double FirstY = std::ceil((CenterY - Half) / Interval);

    const double CountX = std::floor((CenterX + Half) / Interval) - FirstX + 1.0;
    const double CountY = std::floor((CenterY + Half) / Interval) - FirstY + 1.0;

    const double MaxVertices = static_cast<double>((std::numeric_limits<UINT>::max)()) / sizeof(FVertexSimple);

    if (!std::isfinite(CountX) || !std::isfinite(CountY) || CountX < 0.0 || CountY < 0.0 ||
        4.0 * (CountX + CountY) > MaxVertices)
    {
        return Request;
    }

    const auto GetAlpha = [Half](double Distance)
        {
            const double Start = Half * 0.7;

            const double T = std::clamp((Distance - Start) / (Half - Start), 0.0, 1.0);

            return static_cast<float>(1.0 - T * T * (3.0 - 2.0 * T));
        };

    // 그리드가 자신의 선 연결 관계를 생성함
    const auto AddSegment = [&Request](double AX, double AY, float AlphaA, double BX, double BY, float AlphaB)
        {
            const uint32 Base =
                static_cast<uint32>(Request.Vertices.Num());

            Request.Vertices.Add({static_cast<float>(AX),static_cast<float>(AY),0.0f,1.0f, 1.0f, 1.0f, AlphaA});

            Request.Vertices.Add({static_cast<float>(BX),static_cast<float>(BY),0.0f,1.0f, 1.0f, 1.0f, AlphaB});

            Request.Indices.Add(Base);
            Request.Indices.Add(Base + 1);
        };

    const double AxisTolerance = Interval * 0.01;

    for (uint32 I = 0; I < static_cast<uint32>(CountX); ++I)
    {
        const double X = (FirstX + I) * Interval;

        if (std::abs(X) <= AxisTolerance)
        {
            continue;
        }

        const double Offset = X - CenterX;
        const float EndAlpha = GetAlpha(std::hypot(Offset, Half));
        const float CenterAlpha = GetAlpha(std::abs(Offset));

        AddSegment(X, CenterY - Half, EndAlpha, X, CenterY, CenterAlpha);

        AddSegment(X, CenterY, CenterAlpha, X, CenterY + Half, EndAlpha);
    }

    for (uint32 I = 0; I < static_cast<uint32>(CountY); ++I)
    {
        const double Y = (FirstY + I) * Interval;

        if (std::abs(Y) <= AxisTolerance)
        {
            continue;
        }

        const double Offset = Y - CenterY;
        const float EndAlpha = GetAlpha(std::hypot(Offset, Half));
        const float CenterAlpha = GetAlpha(std::abs(Offset));

        AddSegment(CenterX - Half, Y, EndAlpha,CenterX, Y, CenterAlpha);

        AddSegment(CenterX, Y, CenterAlpha, CenterX + Half, Y, EndAlpha);
    }

    return Request;
}
