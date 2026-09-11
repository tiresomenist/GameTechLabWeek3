#pragma once
#include "FVector.h"
#include <cmath>

inline bool ApplyScaleEdit(const FVector& Before, int Axis, float Value, bool Locked, FVector& Out)
{
    if (Axis < 0 || Axis > 2 || !std::isfinite(Value)) return false;
    const float Previous = Axis == 0 ? Before.X : Axis == 1 ? Before.Y : Before.Z;
    FVector Candidate = Before;
    // A zero axis has no usable ratio: change only the edited axis.
    if (Locked && std::fabs(Previous) > 1.0e-6f)
    {
        const float Ratio = Value / Previous;
        if (!std::isfinite(Ratio)) return false;
        Candidate = Before * Ratio;
    }
    if (Axis == 0) Candidate.X = Value;
    if (Axis == 1) Candidate.Y = Value;
    if (Axis == 2) Candidate.Z = Value;
    if (!std::isfinite(Candidate.X) || !std::isfinite(Candidate.Y) || !std::isfinite(Candidate.Z)) return false;
    Out = Candidate;
    return true;
}
