#pragma once

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
        TArray<FVertexTest>& OutVertices,
        TArray<uint32>& OutIndices
    );

    static void CreateCube(
        float Width,
        float Height,
        float Depth,
        TArray<FVertexTest>& OutVertices,
        TArray<uint32>& OutIndices
    );

    static void CreateSphere(
        float Radius,
        uint32 SliceCount,
        uint32 StackCount,
        TArray<FVertexTest>& OutVertices,
        TArray<uint32>& OutIndices
    );
};