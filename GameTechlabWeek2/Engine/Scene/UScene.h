#pragma once
#include "Container/TArray.h"
#include "Engine/Object/UObject.h"
#include "Engine/Object/UActor.h"
#include "Engine/Object/FObjectFactory.h"
#include "Engine/Renderer/RenderUtil.h"
#include "../Object/Primitive/UPrimitiveComponent.h"

struct FPrimitiveRenderData;
class UCameraComponent;
class FRenderer;
class FArchive;

class UScene : public UObject
{

	UCLASS(UScene, "Scene", UObject)

public:
	virtual void BeginPlay();

    virtual void Tick(float DeltaTime);

	virtual void EndPlay();

	UCameraComponent* GetMainCamera() const { return MainCamera; }
	void SetMainCamera(UCameraComponent* InCamera) { MainCamera = InCamera; }

	void CreateMainCamera();

	void Serialize(TArray<FArchive>& ObjectInfoList);

	void Deserialize(TArray<FArchive>& ObjectInfoList);

	template <typename T>
	T SpawnActor(FClassType* Type, uint32 UUID = -1)
	{
		if (Type == nullptr || !Type->IsA(UActor::GetClass()))
		{
			return nullptr;
		}

		UActor* Actor = static_cast<UActor*>(FObjectFactory::ConstructSceneObject(Type, UUID));
		Actors.Add(Actor);
		return static_cast<T>(Actor);
	}

	void Destroy(UObject* Object);
	void DestroyActor(UActor* Actor);

	//외부에서 Primitive 접근 제공
	template <typename Func>
	void ForEachPrimitive(Func&& Function) const
	{
		for (UActor* Actor : Actors)
		{
			for (UActorComponent* Component : Actor->GetComponents())
			{
				if (Component->IsA(UPrimitiveComponent::GetClass()))
				{
					Function(static_cast<UPrimitiveComponent*>(Component));
				}
			}
		}
	}

	virtual ~UScene();

protected:

	/// <summary>
	/// Scene에 종속된 모든 Actor를 담는 멤버 변수. Component는 Actor가 소유합니다.
	/// </summary>
	TArray<UActor*> Actors {};
	
	/// <summary>
	/// Scene의 렌더링을 담당할 MainCamera를 담는 멤버 변수
	/// </summary>
	UCameraComponent* MainCamera = nullptr;

public:
	friend TArray<FPrimitiveRenderData> RenderUtil::GetRenderList(FEditor* Editor, UScene* Scene);
	friend TArray<FPrimitiveRenderData> RenderUtil::GetGizmoList(FEditor* Editor, UScene* Scene);
};
