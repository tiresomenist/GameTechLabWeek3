#pragma once

#include "Core/Math/FVector.h"
#include "Core/Container/TArray.h"
#include "Engine/Renderer/FVertexSimple.h"


class FGeometryGenerator
{
public:
    static void CreatePlane(
        float Width,
        float Height,
        uint32 SubdivisionsX,
        uint32 SubdivisionsY,
        TArray<FVertexTexture>& OutVertices,
        TArray<uint32>& OutIndices
    );

    static void CreateCube(
        float Width,
        float Height,
        float Depth,
        TArray<FVertexTexture>& OutVertices,
        TArray<uint32>& OutIndices
    );

    static void CreateSphere(
        float Radius,
        uint32 SliceCount,
        uint32 StackCount,
        TArray<FVertexTexture>& OutVertices,
        TArray<uint32>& OutIndices
    );
};