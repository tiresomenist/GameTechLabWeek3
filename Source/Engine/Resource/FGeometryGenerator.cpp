#include <pch.h>
#include "FGeometryGenerator.h"

void FGeometryGenerator::CreatePlane(
	float Width,
	float Height,
	uint32 SubdivisionsX,
	uint32 SubdivisionsY,
	TArray<FVertexTexture>& OutVertices,
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

			FVertexTexture Vertex;
			Vertex.x = x;
			Vertex.y = y;
			Vertex.z = 0.0f;

			Vertex.r = 1.0f;
			Vertex.g = 1.0f;
			Vertex.b = 1.0f;
			Vertex.a = 1.0f;

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
	TArray<FVertexTexture>& OutVertices,
	TArray<uint32>& OutIndices
)
{
	OutVertices.Empty();
	OutIndices.Empty();

	float HalfX = 0.5f * Width;
	float HalfY = 0.5f * Height;
	float HalfZ = 0.5f * Depth;

	auto AddFace = [&](FVector p0, FVector p1, FVector p2, FVector p3)
		{
			uint32 Base = OutVertices.Num();

			OutVertices.Add({ p0.X, p0.Y, p0.Z, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f });
			OutVertices.Add({ p1.X, p1.Y, p1.Z, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f });
			OutVertices.Add({ p2.X, p2.Y, p2.Z, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f });
			OutVertices.Add({ p3.X, p3.Y, p3.Z, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f });

			OutIndices.Add(Base + 0); OutIndices.Add(Base + 1); OutIndices.Add(Base + 2);
			OutIndices.Add(Base + 0); OutIndices.Add(Base + 2); OutIndices.Add(Base + 3);
		};


	AddFace({ -HalfX, -HalfY, -HalfZ }, { -HalfX, +HalfY, -HalfZ }, { +HalfX, +HalfY, -HalfZ }, { +HalfX, -HalfY, -HalfZ });
	AddFace({ +HalfX, -HalfY, +HalfZ }, { +HalfX, +HalfY, +HalfZ }, { -HalfX, +HalfY, +HalfZ }, { -HalfX, -HalfY, +HalfZ });
	AddFace({ -HalfX, +HalfY, -HalfZ }, { -HalfX, +HalfY, +HalfZ }, { +HalfX, +HalfY, +HalfZ }, { +HalfX, +HalfY, -HalfZ });
	AddFace({ -HalfX, -HalfY, +HalfZ }, { -HalfX, -HalfY, -HalfZ }, { +HalfX, -HalfY, -HalfZ }, { +HalfX, -HalfY, +HalfZ });
	AddFace({ -HalfX, -HalfY, +HalfZ }, { -HalfX, +HalfY, +HalfZ }, { -HalfX, +HalfY, -HalfZ }, { -HalfX, -HalfY, -HalfZ });
	AddFace({ +HalfX, -HalfY, -HalfZ }, { +HalfX, +HalfY, -HalfZ }, { +HalfX, +HalfY, +HalfZ }, { +HalfX, -HalfY, +HalfZ });
}

void FGeometryGenerator::CreateSphere(
	float Radius,
	uint32 SliceCount,
	uint32 StackCount,
	TArray<FVertexTexture>& OutVertices,
	TArray<uint32>& OutIndices
)
{
	SliceCount = SliceCount < 3 ? 3 : SliceCount;
	StackCount = StackCount < 3 ? 3 : StackCount;

	OutVertices.Empty();
	OutIndices.Empty();

	const uint32 RingVertexCount = SliceCount + 1;

	for (uint32 i = 0; i <= StackCount; ++i)
	{
		float Phi = i * (PI / StackCount);
		float Z = Radius * cos(Phi);
		float R = Radius * sin(Phi);
		float V = i / static_cast<float>(StackCount);

		for (uint32 j = 0; j <= SliceCount; ++j)
		{
			float theta = j * (PI * 2 / SliceCount);
			float X = R * cos(theta);
			float Y = R * sin(theta);
			float U = 1- j / static_cast<float>(SliceCount);

			FVertexTexture Vertex;
			Vertex.x = X; Vertex.y = Y; Vertex.z = Z;
			Vertex.r = 1.0f; Vertex.g = 1.0f; Vertex.b = 1.0f; Vertex.a = 1.0f;
			Vertex.u = U; Vertex.v = V;

			OutVertices.Add(Vertex);
		}
	}

	for (uint32 i = 0; i < StackCount; ++i)
	{
		uint32 RowA = i * RingVertexCount;
		uint32 RowB = (i + 1) * RingVertexCount;

		for (uint32 j = 0; j < SliceCount; ++j)
		{
			uint32 A = RowA + j;
			uint32 B = RowA + j + 1;
			uint32 C = RowB + j;
			uint32 D = RowB + j + 1;

			OutIndices.Add(A); OutIndices.Add(B); OutIndices.Add(C);
			OutIndices.Add(B); OutIndices.Add(D); OutIndices.Add(C);
		}
	}
}