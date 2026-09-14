#pragma once
#include <memory>
#include "Core/Container/TArray.h"
#include "Engine/Actor/UActor.h"
#include "Engine/Object/FObjectFactory.h"
#include "Engine/Renderer/RenderUtil.h"
#include "Engine/Scene/FSceneType.h"
#include "Engine/Component/Primitive/UPrimitiveComponent.h"
#include "Engine/Component/UWidgetComponent.h"

struct FPrimitiveRenderData;
class UCameraComponent;
class FRenderer;
class FArchive;

// Scene은 Actor를 소유하는 컨테이너입니다. UUID/RTTI가 필요한 UObject가 아닙니다.
class UScene
{
public:
	UScene() = default;

	static FSceneType* GetStaticSceneType();
	virtual FSceneType* GetSceneType() const { return GetStaticSceneType(); }

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

	template <typename Func>
	void ForEachWidget(Func&& Function) const
	{
		for (UActor* Actor : Actors)
		{
			for (UActorComponent* Component : Actor->GetComponents())
			{
				if (Component->IsA(UWidgetComponent::GetClass()))
				{
					Function(static_cast<UWidgetComponent*>(Component));
				}
			}
		}
	}

	virtual ~UScene();

protected:
	void EnsureUUIDWidgets();

	/// <summary>
	/// Scene에 종속된 모든 Actor를 담는 멤버 변수. Component는 Actor가 소유합니다.
	/// </summary>
	TArray<UActor*> Actors {};
	
	/// <summary>
	/// Scene의 렌더링을 담당할 MainCamera를 담는 멤버 변수
	/// </summary>
	UCameraComponent* MainCamera = nullptr;

public:
	friend TArray<FPrimitiveRenderData> RenderUtil::GetRenderList(FEditor* Editor, UScene* Scene, bool bShowPrimitives);
	friend TArray<FPrimitiveRenderData> RenderUtil::GetGizmoList(FEditor* Editor, UScene* Scene);
};
