#include "pch.h"
#include "UScene.h"
#include "SceneValidation.h"
#include <memory>
#include <stdexcept>
// TEMP(UI test): Gizmo implementation is currently excluded from the build.
// #include "Engine/Gizmo/UGizmo.h"

#include "Engine/Object/UCameraComponent.h"
#include "Engine/Object/UActor.h"
#include "Engine/Object/UActorComponent.h"
#include "Engine/Object/USceneComponent.h"
#include "Engine/Object/Primitive/UPrimitiveComponent.h"
#include "Engine/Object/Primitive/USphereComponent.h"

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
        FClassType* Type = FClassRegistry::FindClassType(TypeName);

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
