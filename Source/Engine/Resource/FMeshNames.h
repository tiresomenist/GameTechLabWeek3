#pragma once

#include "Core/Name/FName.h"

struct FMeshNames
{
    const FName Sphere{ "Sphere" };
    const FName Cube{ "Cube" };
    const FName Plane{ "Plane" };
    const FName Triangle{ "Triangle" };
    const FName Pepe{ "Pepe" };
    const FName Octopus{ "Octopus" };

    const FName Flame{ "Flame" };
    const FName SpotLightIcon{ "SpotLightIcon" };
    const FName Grid{ "Grid" };

    const FName ArrowRed{ "ArrowRed" };
    const FName ArrowGreen{ "ArrowGreen" };
    const FName ArrowBlue{ "ArrowBlue" };

    const FName MoveRed{ "MoveRed" };
    const FName MoveGreen{ "MoveGreen" };
    const FName MoveBlue{ "MoveBlue" };

    const FName RotateRed{ "RotateRed" };
    const FName RotateGreen{ "RotateGreen" };
    const FName RotateBlue{ "RotateBlue" };

    const FName ScaleRed{ "ScaleRed" };
    const FName ScaleGreen{ "ScaleGreen" };
    const FName ScaleBlue{ "ScaleBlue" };
};

inline const FMeshNames& GetMeshNames()
{
    static const FMeshNames Names;
    return Names;
}