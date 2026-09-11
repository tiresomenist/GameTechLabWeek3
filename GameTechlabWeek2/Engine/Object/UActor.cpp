#include "pch.h"
#include "UActor.h"

#include "Engine/Object/FClassType.h"
#include "Engine/Object/FObjectFactory.h"
#include "Engine/Object/USceneComponent.h"

UActorComponent* UActor::CreateComponent(FClassType* Type, uint32 UUID)
{
    if (Type == nullptr || !Type->IsA(UActorComponent::GetClass()))
    {
        return nullptr;
    }

    UActorComponent* Component = static_cast<UActorComponent*>(
        FObjectFactory::ConstructSceneObject(Type, UUID));

    Component->SetOwner(this);
    Components.Add(Component);

    if (RootComponent == nullptr && Component->IsA(USceneComponent::GetClass()))
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

bool UActor::RemoveComponent(UActorComponent* Component, bool bDestroy)
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
    }

    Component->SetOwner(nullptr);
    if (bDestroy)
    {
        delete Component;
    }

    return true;
}

void UActor::SetRootComponent(USceneComponent* Component)
{
    if (Component == nullptr || Component->GetOwner() != this)
    {
        return;
    }

    RootComponent = Component;
}

void UActor::BeginPlay()
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

void UActor::Tick(float DeltaTime)
{
    for (UActorComponent* Component : Components)
    {
        Component->Tick(DeltaTime);
    }
}

void UActor::EndPlay()
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

void UActor::ReleaseComponents()
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

UActor::~UActor()
{
    EndPlay();
    ReleaseComponents();
}
