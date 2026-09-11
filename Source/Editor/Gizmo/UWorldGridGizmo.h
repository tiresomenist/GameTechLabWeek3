#pragma once

#include "UGizmo.h"

class UWorldGridGizmo : public UGizmo
{

    UCLASS(UWorldGridGizmo, "WorldGridGizmo", UGizmo)

private:
    // Grid 중심으로부터 양쪽에 몇 칸까지 만들 것인지
    int HalfGridCount = 20;

    // Grid 한 칸 간격
    float GridSpacing = 1.0f;

    // 몇 칸마다 Major Grid를 표시할 것인지
    int MajorGridInterval = 5;
};

