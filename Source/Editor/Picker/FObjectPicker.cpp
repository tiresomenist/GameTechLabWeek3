#include "pch.h"
#include "FObjectPicker.h"
#include "Core/Math/FVector.h"
#include "Core/Math/Matrix.h"
#include "Engine/Input/GInputManager.h"
#include "Engine/Renderer/FVertexSimple.h"
#include "Engine/Scene/UScene.h"
#include "Engine/Log.h"
#include "Engine/Component/UCameraComponent.h"
#include "Engine/Component/Primitive/UPrimitiveComponent.h"
#include "Editor/FEditor.h"
#include "Engine/Actor/AActor.h"
#include "Engine/Component/Light/USpotLightComponent.h"

FObjectPicker::FObjectPicker(FEditor* InEditor)
	: Editor{ InEditor }
{
}

bool FObjectPicker::MakeWorldRay(FRay& OutRay) {
	UCameraComponent* Camera = Editor->GetEditorCamera();

	auto& Input = *GInputManager::GetInstance();
	float NDCX = Input.GetLeftCursorX();
	float NDCY = Input.GetLeftCursorY();

	FVector4 Near(NDCX, NDCY, 0.0f, 1.0f);
	FVector4 Far(NDCX, NDCY, 1.0f, 1.0f);
	FMatrix Projection = Camera->GetProjectionMatrix();
	FMatrix View = Camera->GetViewMatrix();

	FMatrix VPInverse;
	if (!(View * Projection).TryInverse(VPInverse)) {
		return false;
	}

	FVector4 NearWorld = Near * VPInverse;	//World에서의 Ray 시작점
	FVector4 FarWorld = Far * VPInverse; //World에서의 Ray 끝점

	if (std::fabs(NearWorld.W) < 1.0e-6f || std::fabs(FarWorld.W) < 1.0e-6f) return false;

	NearWorld = FVector4(FVector(NearWorld) / NearWorld.W, 1.0f);
	FarWorld = FVector4(FVector(FarWorld) / FarWorld.W, 1.0f);

	FVector4 Direction = FVector4((FVector(FarWorld) - FVector(NearWorld)).GetNormalized(), 0.0f);	//Ray 방향
	
	OutRay.Origin = FVector(NearWorld);
	OutRay.Direction = FVector(Direction);
	return true;
}

bool FObjectPicker::RayTriangleIntersect(const FRay& Ray,FVector A, FVector B, FVector C ,float& OutDistance ) {
	const FVector Edge1 = B - A;
	const FVector Edge2 = C - A;

	const FVector P = Ray.Direction.Cross(Edge2);
	const float Determinant = Edge1.Dot(P);

	if (std::fabs(Determinant) < 1.0e-6f)
		return false; // 평행 또는 퇴화 삼각형

	const float InverseDeterminant = 1.0f / Determinant;

	const FVector T = Ray.Origin - A;
	const float U = T.Dot(P) * InverseDeterminant;
	if (U < 0.0f || U > 1.0f)
		return false;

	const FVector Q = T.Cross(Edge1);
	const float V = Ray.Direction.Dot(Q) * InverseDeterminant;
	if (V < 0.0f || U + V > 1.0f)
		return false;

	OutDistance = Edge2.Dot(Q) * InverseDeterminant;
	return OutDistance > 1.0e-6f;
}

bool FObjectPicker::RayAABBIntersect(const FRay& Ray,const FVector& BoundsMin,const FVector& BoundsMax,float MaxDistance,float& OutDistance)
{
	float Enter = 0.0f;
	float Exit = MaxDistance;

	auto TestAxis = [&](float Origin, float Direction, float Min, float Max) -> bool
		{
			// 이 축으로 움직이지 않으면 시작점이 범위 안에 있어야 함.
			if (Direction == 0.0f)
				return Origin >= Min && Origin <= Max;

			float T0 = (Min - Origin) / Direction;
			float T1 = (Max - Origin) / Direction;

			if (T0 > T1)
				std::swap(T0, T1);

			Enter = (std::max)(Enter, T0);
			Exit = (std::min)(Exit, T1);

			// 같을 때도 허용해야 두께가 0인 Plane·Triangle을 검사할 수 있음.
			return Enter <= Exit;
		};

	if (!TestAxis(Ray.Origin.X, Ray.Direction.X, BoundsMin.X, BoundsMax.X))
		return false;

	if (!TestAxis(Ray.Origin.Y, Ray.Direction.Y, BoundsMin.Y, BoundsMax.Y))
		return false;

	if (!TestAxis(Ray.Origin.Z, Ray.Direction.Z, BoundsMin.Z, BoundsMax.Z))
		return false;

	OutDistance = Enter;
	return true;
}

USceneComponent* FObjectPicker::Pick()
{
	if (!Editor) { return nullptr; }

	UScene* Scene = Editor->GetCurrentScene();
	const UCameraComponent* Camera = Editor->GetEditorCamera();

	if (!Scene || !Camera) { return nullptr; }

	// 숨겨진 프리미티브와 아이콘을 선택하지 않도록 처리함
	if (!Editor->IsShowingPrimitives()) { return nullptr; }

	FRay Ray;

	if (!MakeWorldRay(Ray)) { return nullptr; }

	// 프리미티브와 광원을 같은 선택 결과로 취급함
	USceneComponent* SelectedObject = nullptr;
	float ClosestDistance = 100000.0f;
	// 19번 최적화해야됨
	Scene->ForEachPrimitive(
		[&](UPrimitiveComponent* Primitive)
		{
			FVector BoundsMin;
			FVector BoundsMax;
			if (!Primitive->GetLocalBounds(BoundsMin, BoundsMax)) return;

			FMatrix InverseWorld;
			const FMatrix& RenderWorldMatrix = Primitive->GetRenderWorldMatrix(Editor->GetEditorCamera());
			if (!RenderWorldMatrix.TryInverse(InverseWorld)) {
				return;
			}

			FRay LocalRay;
			LocalRay.Origin = FVector(FVector4(Ray.Origin, 1.0f) * InverseWorld);
			LocalRay.Direction = FVector(FVector4(Ray.Direction, 0.0f) * InverseWorld);

			float AABBDistance = 0.0f;
			if (!RayAABBIntersect(LocalRay, BoundsMin, BoundsMax, ClosestDistance, AABBDistance)) return;

			if (Primitive->IsAABBOnlyPickable())
			{
				ClosestDistance = AABBDistance;
				SelectedObject = Primitive;
				return;
			}

			FMeshResource* Mesh = Primitive->GetMeshResource();
			if (!Mesh) return;

			const size_t Count = Mesh->GetIndices().Num();
            const size_t VertexCount = Mesh->GetPositions().Num();
            if (Count != Mesh->GetIndexCount() || Count % 3 != 0) return;
            for (size_t Index = 0; Index < Count; Index += 3)
			{
				const uint32 I0 = Mesh->GetIndices()[Index];
				const uint32 I1 = Mesh->GetIndices()[Index + 1];
				const uint32 I2 = Mesh->GetIndices()[Index + 2];
                if (I0 >= VertexCount || I1 >= VertexCount || I2 >= VertexCount) continue;

				const FVector& A = Mesh->GetPositions()[I0];
				const FVector& B = Mesh->GetPositions()[I1];
				const FVector& C = Mesh->GetPositions()[I2];

				float T;
				
				if (RayTriangleIntersect(LocalRay, A, B, C, T))
				{
					if (T < ClosestDistance)
					{
						ClosestDistance = T;
						SelectedObject = Primitive;
					}
				}
			}
		}
	);
	//SpotLight용 임시코드. continue 조건 바꿈으로써 차후에 확장가능
	Scene->ForEachActor(
		[&](AActor* Actor)
		{
			for (UActorComponent* Component : Actor->GetComponents())
			{
				if (!Component->IsA(USpotLightComponent::GetClass()))
				{
					continue;
				}

				auto* SpotLight = static_cast<USpotLightComponent*>(Component);

				// 실제 렌더링과 동일한 행렬 및 로컬 범위를 조회함
				const FPrimitiveRenderData IconData = SpotLight->BuildIconRenderData(Camera, false);

				if (!IconData.VertexBuffer || !IconData.IndexBuffer || !IconData.Material || !IconData.WorldMatrix || IconData.IndexCount == 0)
				{
					continue;
				}

				FMatrix InverseWorld;

				// 크기가 0인 경우 등 역변환이 불가능한 아이콘을 제외함
				if (!IconData.WorldMatrix->TryInverse(InverseWorld))
				{
					continue;
				}

				// 월드 광선을 아이콘의 로컬 공간으로 변환함
				FRay LocalRay;

				LocalRay.Origin = FVector(FVector4(Ray.Origin, 1.0f) * InverseWorld);

				// 거리 비교 기준을 유지하기 위해 정규화하지 않음
				LocalRay.Direction = FVector(FVector4(Ray.Direction, 0.0f) * InverseWorld);

				float HitDistance = 0.0f;

				// 로컬 XY 평면의 아이콘 사각형과 교차 검사함
				if (!RayAABBIntersect(LocalRay,IconData.Min,IconData.Max,ClosestDistance,HitDistance))
				{
					continue;
				}

				// 기존 메시와 아이콘 중 광선 시작점에 가까운 대상을 선택함
				if (HitDistance > EPSILON &&HitDistance < ClosestDistance)
				{
					ClosestDistance = HitDistance;
					SelectedObject = SpotLight;
				}
			}
		}
	);
	return SelectedObject;
}
