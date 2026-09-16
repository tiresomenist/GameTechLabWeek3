#pragma once

#include "Core/Container/TArray.h"
#include "Engine/Component/UActorComponent.h"

class USceneComponent;
struct FClassType;

// Scene에 배치되는 게임 오브젝트 단위입니다. Component의 생성과 파괴를 소유합니다.
class AActor : public UObject
{
   UCLASS(AActor, "Actor", UObject)

public:
    UActorComponent* CreateComponent(FClassType* Type, uint32 UUID = -1);
    bool RemoveComponent(UActorComponent* Component, bool bDestroy = true);

    void SetRootComponent(USceneComponent* Component);
    USceneComponent* GetRootComponent() const { return RootComponent; }

    const TArray<UActorComponent*>& GetComponents() const { return Components; }

    // 삭제 수명을 공유하는 Actor 계층입니다. Transform 계층은 각 Actor 내부 Component가 담당합니다.
    bool SetParentActor(AActor* NewParent);
    AActor* GetParentActor() const { return ParentActor; }
    const TArray<AActor*>& GetChildActors() const { return ChildActors; }

    virtual void BeginPlay();
    virtual void Tick(float DeltaTime);
    virtual void EndPlay();

    ~AActor() override;

private:
    void ReleaseComponents();
    void DetachChildren();

    TArray<UActorComponent*> Components;
    USceneComponent* RootComponent = nullptr;
    AActor* ParentActor = nullptr;
    TArray<AActor*> ChildActors;
    bool bHasBegunPlay = false;
};
