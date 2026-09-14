#include <pch.h>
#include "FGeometryGenerator.h"


void FGeometryGenerator::CreatePlane(
    float Width,
    float Height,
    uint32 SubdivisionsX,
    uint32 SubdivisionsY,
    TArray<FVertexTest>& OutVertices,
    TArray<uint32>& OutIndices
)
{
    OutVertices.Empty();
    OutIndices.Empty();

    const uint32 SegX = (SubdivisionsX < 1) ? 1 : SubdivisionsX;
    const uint32 SegY = (SubdivisionsY < 1) ? 1 : SubdivisionsY;

    const uint32 VertexCountX = SegX + 1;
    const uint32 VertexCountY = SegY + 1;

    float HalfWidth = 0.5f * Width;
    float HalfHeight = 0.5f * Height;

    float DeltaX = Width / static_cast<float>(SegX);
    float DeltaY = Height / static_cast<float>(SegY);

    for (uint32 i = 0; i < VertexCountY; ++i)
    {
        float y = -HalfHeight + static_cast<float>(i) * DeltaY;
        float v = 1.0f - (static_cast<float>(i) / static_cast<float>(SegY));

        for (uint32 j = 0; j < VertexCountX; ++j)
        {
            float x = -HalfWidth + static_cast<float>(j) * DeltaX;
            float u = static_cast<float>(j) / static_cast<float>(SegX);

            FVertexTest Vertex;
            Vertex.x = x;
            Vertex.y = y;
            Vertex.z = 0.0f;

            Vertex.nx = 0.0f;
            Vertex.ny = 0.0f;
            Vertex.nz = 1.0f;

            Vertex.u = u;
            Vertex.v = v;

            OutVertices.Add(Vertex);
        }
    }

    for (uint32 i = 0; i < SegY;++i)
    {
        for (uint32 j = 0;j < SegX;++j)
        {
            uint32 BottomLeft = i * VertexCountX + j;
            uint32 BottomRight = BottomLeft + 1;
            uint32 TopLeft = (i + 1) * VertexCountX + j;
            uint32 TopRight = TopLeft + 1;

            OutIndices.Add(BottomLeft);
            OutIndices.Add(BottomRight);
            OutIndices.Add(TopRight);

            OutIndices.Add(BottomLeft);
            OutIndices.Add(TopRight);
            OutIndices.Add(TopLeft);
        }
    }
}

void FGeometryGenerator::CreateCube(
    float Width,
    float Height,
    float Depth,
    TArray<FVertexTest>& OutVertices,
    TArray<uint32>& OutIndices
)
{

}
void FGeometryGenerator::CreateSphere(
    float Radius,
    uint32 SliceCount,
    uint32 StackCount,
    TArray<FVertexTest>& OutVertices,
    TArray<uint32>& OutIndices
)
{

}