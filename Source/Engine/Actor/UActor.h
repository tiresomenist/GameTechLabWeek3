#pragma once

#include "Core/Container/TArray.h"
#include "Engine/Component/UActorComponent.h"

class USceneComponent;
struct FClassType;

// Scene에 배치되는 게임 오브젝트 단위입니다. Component의 생성과 파괴를 소유합니다.
class UActor : public UObject
{
    UCLASS(UActor, "Actor", UObject)

public:
    UActorComponent* CreateComponent(FClassType* Type, uint32 UUID = -1, FName InName = FName());
    bool RemoveComponent(UActorComponent* Component, bool bDestroy = true);

    UActorComponent* FindComponentByName(const FName& InName) const;

    void SetRootComponent(USceneComponent* Component);
    USceneComponent* GetRootComponent() const { return RootComponent; }

    const TArray<UActorComponent*>& GetComponents() const { return Components; }

    virtual void BeginPlay();
    virtual void Tick(float DeltaTime);
    virtual void EndPlay();

    ~UActor() override;

private:
    void ReleaseComponents();

    TArray<UActorComponent*> Components;
    USceneComponent* RootComponent = nullptr;
    FName ActorName;
    bool bHasBegunPlay = false;
};
