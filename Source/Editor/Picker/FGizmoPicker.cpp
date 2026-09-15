#include "pch.h"
#include "Editor/Picker/FGizmoPicker.h"
#include "Engine/Input/GInputManager.h"
#include "Engine/Log.h"
#include "Engine/Renderer/GDevice.h"
#include "Engine/Scene/UScene.h"
#include "Editor/Gizmo/UObjectAxisGizmo.h"
#include "Editor/Picker/FObjectPicker.h"


namespace
{	//VP행렬->에디터의카메라참조 Viewport->GDevice 참조
	bool WorldToPixel(const FVector& Position, const FMatrix& ViewProjection, const D3D11_VIEWPORT& Viewport, FVector& OutPixel)
	{
		const FVector4 Clip = FVector4(Position, 1.0f) * ViewProjection;

		if (!std::isfinite(Clip.W) || Clip.W <= 1.0e-6f)
			return false;

		const float NDCX = Clip.X / Clip.W;
		const float NDCY = Clip.Y / Clip.W;

		OutPixel = FVector(
			Viewport.TopLeftX + (NDCX + 1.0f) * 0.5f * Viewport.Width,
			Viewport.TopLeftY + (1.0f - NDCY) * 0.5f * Viewport.Height, 0.0f
		);

		return std::isfinite(OutPixel.X) && std::isfinite(OutPixel.Y);
	}
}


FGizmoPicker::FGizmoPicker(FEditor* InEditor)
	: Editor{ InEditor }
{

}

FGizmoPicker::~FGizmoPicker()
{
}

bool FGizmoPicker::RayTriangleIntersect(const FRay& Ray, FVector A, FVector B, FVector C, float& OutDistance) {
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

bool FGizmoPicker::MakeWorldRay(FRay& OutRay) {

    auto& Input = *GInputManager::GetInstance();
    float NDCX = Input.GetLeftCursorX();
    float NDCY = Input.GetLeftCursorY();

    FVector4 Near(NDCX, NDCY, 0.0f, 1.0f);
    FVector4 Far(NDCX, NDCY, 1.0f, 1.0f);
    FMatrix Projection = Editor->GetEditorCamera()->GetProjectionMatrix();
    FMatrix View = Editor->GetEditorCamera()->GetViewMatrix();

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

int FGizmoPicker::Pick(UGizmo* InGizmos)
{
    auto* Gizmo = dynamic_cast<UObjectAxisGizmo*>(InGizmos);
    if (!Editor || !Editor->GetEditorCamera() || !Gizmo || !Gizmo->UpdateTransform()) return -1;    //뭔가 잘못되었으면

    const auto& Viewport = GDevice::GetInstance()->GetViewport();
    if (Viewport.Width <= 0 || Viewport.Height <= 0) return -1; //창 크기가 0보다 작으면

    //현재 종횡비를 갱신하고, VP행렬을 가져온다
    auto* Camera = Editor->GetEditorCamera();
    Camera->SetAspectRatio(Viewport.Width / Viewport.Height);
    const FMatrix VP = Camera->GetViewMatrix() * Camera->GetProjectionMatrix();
    
    auto& Input = *GInputManager::GetInstance();
    //NDC좌표에서 PIXEL좌표로
    const FVector Click(
        Viewport.TopLeftX + (Input.GetLeftCursorX() + 1) * 0.5f * Viewport.Width,
        Viewport.TopLeftY + (1 - Input.GetLeftCursorY()) * 0.5f * Viewport.Height, 0);
    
    // 클릭 허용 픽셀
    constexpr float PickRadiusPixels = 8.0f;
    float BestDistanceSquared = PickRadiusPixels * PickRadiusPixels;

    int32 SelectedAxis = -1;

    FRay Ray;
    if (!MakeWorldRay(Ray)) return -1;	//Ray 계산 실패

    float ClosestDistance = 100000.f;

    for (const FGizmoHandle& Handle : Gizmo->GetHandles())
    {
        const FMeshResource* Mesh = Handle.Mesh;
        if (!Mesh || Handle.Axis < 0 || Handle.Axis > 2) continue;

        const uint32 Count = (std::min)(Mesh->GetIndexCount(), uint32(Mesh->GetIndices().Num()));

        if (Handle.Topology == 0) {
            for (uint32 Index = 0; Index + 1 < Count; Index += 2)
            {
                const uint32 I0 = Mesh->GetIndices()[Index], I1 = Mesh->GetIndices()[Index + 1];
                if (I0 >= uint32(Mesh->GetPositions().Num()) || I1 >= uint32(Mesh->GetPositions().Num())) continue;
                const auto& V0 = Mesh->GetPositions()[I0];
                const auto& V1 = Mesh->GetPositions()[I1];

                //월드 좌표계에서 선분 정점 위치 계산
                FVector A(FVector4(V0, 1.0f) * Handle.WorldMatrix);
                FVector B(FVector4(V1, 1.0f) * Handle.WorldMatrix);

                // 클립좌표계로 변환
                const FVector4 CA = FVector4(A, 1) * VP;
                const FVector4 CB = FVector4(B, 1) * VP;

                //NearZ 평면과 비교해서 잘라내기
                if (!std::isfinite(CA.Z) || !std::isfinite(CB.Z) || (CA.Z < 0 && CB.Z < 0)) continue;

                if (CA.Z < 0) A = A + (B - A) * (CA.Z / (CA.Z - CB.Z));
                else if (CB.Z < 0) B = A + (B - A) * (CA.Z / (CA.Z - CB.Z));

                //정점을 픽셀로 변환
                FVector PA, PB;
                if (!WorldToPixel(A, VP, Viewport, PA) || !WorldToPixel(B, VP, Viewport, PB)) continue;

                //픽셀로 변환된 최종 선분
                const FVector Edge = PB - PA;
                const float LengthSquared = Edge.LengthSquared();
                //클릭 지점을 선분에 사영시켰을때의 비율
                const float T = LengthSquared > 1.0e-8f ? std::clamp((Click - PA).Dot(Edge) / LengthSquared, 0.0f, 1.0f) : 0.0f;
                const float DistanceSquared = (Click - (PA + Edge * T)).LengthSquared();
                if (DistanceSquared < BestDistanceSquared)
                {
                    BestDistanceSquared = DistanceSquared;
                    SelectedAxis = Handle.Axis;
                }
            }
        }
        else {
            const FMatrix* World = &Handle.WorldMatrix;
            for (uint32 Index = 0; Index + 2 < Count; Index += 3)
            {
                const uint32 I0 = Mesh->GetIndices()[Index];
                const uint32 I1 = Mesh->GetIndices()[Index + 1];
                const uint32 I2 = Mesh->GetIndices()[Index + 2];

                if (I0 >= uint32(Mesh->GetPositions().Num()) || I1 >= uint32(Mesh->GetPositions().Num())|| I2 >= uint32(Mesh->GetPositions().Num())) continue;

                FVector A = Mesh->GetPositions()[I0];
                FVector B = Mesh->GetPositions()[I1];
                FVector C = Mesh->GetPositions()[I2];


                // 현재 오브젝트의 WorldMatrix를 반영
                A = FVector(FVector4(A, 1.0f) * *World);
                B = FVector(FVector4(B, 1.0f) * *World);
                C = FVector(FVector4(C, 1.0f) * *World);

                float T;

                if (RayTriangleIntersect(Ray, A, B, C, T))
                {
                    if (T < ClosestDistance)
                    {
                        ClosestDistance = T;
                        //X,Y,Z축 중 어느걸 골랐는지 체크
                        SelectedAxis = Handle.Axis;
                    }
                }
            }
        }
        
    }
    return SelectedAxis;
}
