#include "pch.h"
#include "AActor.h"

#include "Engine/Object/FClassType.h"
#include "Engine/Object/FObjectFactory.h"
#include "Engine/Component/USceneComponent.h"

UActorComponent* AActor::CreateComponent(FClassType* Type, uint32 UUID)
{
    if (Type == nullptr || !Type->IsA(UActorComponent::GetClass()))
    {
        return nullptr;
    }

    UActorComponent* Component = static_cast<UActorComponent*>(
        FObjectFactory::ConstructSceneObject(Type, UUID));

    Component->SetOwner(this);
    Components.Add(Component);

    if (RootComponent == nullptr && Component->IsA(USceneComponent::GetClass()) && static_cast<USceneComponent*>(Component)->CanBeRootComponent())
    {
        RootComponent = static_cast<USceneComponent*>(Component);
    }

    Component->OnRegister();
    if (bHasBegunPlay)
    {
        Component->BeginPlay();
    }

    return Component;
}

bool AActor::RemoveComponent(UActorComponent* Component, bool bDestroy)
{
    if (Component == nullptr || Component->GetOwner() != this)
    {
        return false;
    }

    if (bHasBegunPlay)
    {
        Component->EndPlay();
    }
    Component->OnUnregister();

    for (int32 Index = 0; Index < Components.Num(); ++Index)
    {
        if (Components[Index] == Component)
        {
            Components.RemoveAt(Index);
            break;
        }
    }

    if (RootComponent == Component)
    {
        RootComponent = nullptr;

        // Root를 제거해도 Actor에 남아 있는 SceneComponent가 있으면 새 Root로 승격한다.
        for (UActorComponent* RemainingComponent : Components)
        {
            if (RemainingComponent->IsA(USceneComponent::GetClass()))
            {
                USceneComponent* Candidate = static_cast<USceneComponent*>(RemainingComponent);
                if (Candidate->CanBeRootComponent())
                {
                    RootComponent = Candidate;
                    break;
                }
            }
        }
    }

    Component->SetOwner(nullptr);
    if (bDestroy)
    {
        delete Component;
    }

    return true;
}

void AActor::SetRootComponent(USceneComponent* Component)
{
    if (Component == nullptr || Component->GetOwner() != this)
    {
        return;
    }

    RootComponent = Component;
}

void AActor::BeginPlay()
{
    if (bHasBegunPlay)
    {
        return;
    }

    bHasBegunPlay = true;
    for (UActorComponent* Component : Components)
    {
        Component->BeginPlay();
    }
}

void AActor::Tick(float DeltaTime)
{
    for (UActorComponent* Component : Components)
    {
        Component->Tick(DeltaTime);
    }
}

void AActor::EndPlay()
{
    if (!bHasBegunPlay)
    {
        return;
    }

    for (UActorComponent* Component : Components)
    {
        Component->EndPlay();
    }
    bHasBegunPlay = false;
}

void AActor::ReleaseComponents()
{
    for (UActorComponent* Component : Components)
    {
        Component->OnUnregister();
        Component->SetOwner(nullptr);
        delete Component;
    }

    Components.Empty();
    RootComponent = nullptr;
}

AActor::~AActor()
{
    EndPlay();
    ReleaseComponents();
}
