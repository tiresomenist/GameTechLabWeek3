#include "pch.h"
#include "UScene.h"
#include "SceneValidation.h"
#include <memory>
#include <stdexcept>
// TEMP(UI test): Gizmo implementation is currently excluded from the build.
// #include "Engine/Gizmo/UGizmo.h"

#include "Engine/Component/UCameraComponent.h"
#include "Engine/Actor/UActor.h"
#include "Engine/Component/UActorComponent.h"
#include "Engine/Component/USceneComponent.h"
#include "Engine/Component/UWidgetComponent.h"
#include "Engine/Component/Primitive/UPrimitiveComponent.h"
#include "Engine/Component/Primitive/USphereComponent.h"

#include "Engine/Object/FArchive.h"
#include "Engine/Object/FClassRegistry.h"
#include "Engine/Log.h"

FSceneType* UScene::GetStaticSceneType()
{
    static FSceneType Type
    {
        .Name = "Scene",
        .SceneConstructor = []() -> UScene* { return new UScene(); },
    };

    return &Type;
}

// UScene의 BeginPlay, Tick, EndPlay는 모든 Scene에 대한 공통 로직이 필요하면 작성
// But 아직 그런 용도가 없음 언젠가 생기면 쓰는걸로...
void UScene::BeginPlay()
{
    for (UActor* Actor : Actors)
    {
        Actor->BeginPlay();
    }
}

void UScene::Tick(float DeltaTime)
{
    for (UActor* Actor : Actors)
    {
        Actor->Tick(DeltaTime);
    }
}

void UScene::EndPlay()
{
    for (UActor* Actor : Actors)
    {
        Actor->EndPlay();
    }
}

void UScene::CreateMainCamera()
{
    UActor* CameraActor = SpawnActor<UActor*>(UActor::GetClass());
    MainCamera = static_cast<UCameraComponent*>(CameraActor->CreateComponent(UCameraComponent::GetClass()));
}

void UScene::Serialize(TArray<FArchive>& ObjectInfoList)
{
    for (UActor* Actor : Actors)
    {
        for (UActorComponent* Component : Actor->GetComponents())
        {
            // UUID 표시는 런타임에 자동 추가하는 보조 컴포넌트이므로 씬 파일에는 저장하지 않는다.
            if (Component->IsA(UWidgetComponent::GetClass())) continue;

            FArchive Archive;
            Archive.SetUInt32("UUID", Component->GetUUID());
            Archive.SetUInt32("ActorUUID", Actor->GetUUID());

            Component->Serialize(Archive);
            ObjectInfoList.Add(Archive);
        }
    }
}

void UScene::Deserialize(TArray<FArchive>& ObjectInfoList)
{
    for (auto& Item : ObjectInfoList)
    {
        FString TypeName = Item.GetString("Type");
        if (!IsAllowedSceneType(TypeName)) throw std::runtime_error("Unsupported scene type");
        // 이전 버전에서 저장된 UUID 위젯은 로드 후 EnsureUUIDWidgets가 다시 생성한다.
        if (TypeName == "WidgetComponent") continue;

        FClassType* Type = FClassRegistry::FindClassType(FName(TypeName));

        uint32 UUID = Item.GetUInt32("UUID");
        UActor* Actor = nullptr;
        if (Item.GetJSON().contains("ActorUUID"))
        {
            const uint32 ActorUUID = Item.GetUInt32("ActorUUID");
            for (UActor* ExistingActor : Actors)
            {
                if (ExistingActor->GetUUID() == ActorUUID)
                {
                    Actor = ExistingActor;
                    break;
                }
            }

            if (Actor == nullptr)
            {
                Actor = SpawnActor<UActor*>(UActor::GetClass(), ActorUUID);
            }
        }
        else
        {
            // 기존 Component-직접-소유 JSON과의 호환: Component 하나당 Actor 하나를 만듭니다.
            Actor = SpawnActor<UActor*>(UActor::GetClass());
        }

        UActorComponent* Component = Actor->CreateComponent(Type, UUID);
        if (Component == nullptr)
        {
            UE_LOG("[UScene] {} 타입은 ActorComponent가 아니므로 로드하지 않습니다.", TypeName);
            continue;
        }

        Component->Deserialize(Item);
        if (MainCamera == nullptr && Component->IsA(UCameraComponent::GetClass()))
		{
			MainCamera = static_cast<UCameraComponent*>(Component);
		}
	}

	EnsureUUIDWidgets();
}

void UScene::EnsureUUIDWidgets()
{
	for (UActor* Actor : Actors)
	{
		USceneComponent* Root = Actor->GetRootComponent();
		if (!Root || !Root->IsA(UPrimitiveComponent::GetClass())) continue;

		bool bHasWidget = false;
		for (UActorComponent* Component : Actor->GetComponents())
		{
			if (Component->IsA(UWidgetComponent::GetClass()))
			{
				bHasWidget = true;
				break;
			}
		}

		if (!bHasWidget)
		{
			Actor->CreateComponent(UWidgetComponent::GetClass());
		}
	}
}

void UScene::Destroy(UObject* Object)
{
    if (Object == nullptr)
    {
        return;
    }

    if (Object->IsA(UActor::GetClass()))
    {
        DestroyActor(static_cast<UActor*>(Object));
        return;
    }

    if (Object->IsA(UActorComponent::GetClass()))
    {
        UActorComponent* Component = static_cast<UActorComponent*>(Object);
        DestroyActor(Component->GetOwner());
    }
}

void UScene::DestroyActor(UActor* Actor)
{
    if (Actor == nullptr)
    {
        return;
    }

    for (int32 Index = 0; Index < Actors.Num(); ++Index)
    {
        if (Actors[Index] == Actor)
        {
            if (MainCamera != nullptr && MainCamera->GetOwner() == Actor)
            {
                MainCamera = nullptr;
            }

            Actor->EndPlay();
            delete Actor;
            Actors.RemoveAt(Index);
            break;
        }
    }
}

UScene::~UScene()
{
    for (UActor* Actor : Actors)
    {
        delete Actor;
    }

    Actors.Empty();
}
