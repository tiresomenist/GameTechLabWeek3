#include "UScene.h"
// TEMP(UI test): Gizmo implementation is currently excluded from the build.
// #include "Engine/Gizmo/UGizmo.h"

#include "Engine/Object/UCameraComponent.h"
#include "Engine/Object/Primitive/UPrimitiveComponent.h"
#include "Engine/Object/Primitive/USphereComponent.h"

#include "Engine/Object/FArchive.h"
#include "Engine/Object/FClassRegistry.h"
#include "Engine/Log.h"

// UScene의 BeginPlay, Tick, EndPlay는 모든 Scene에 대한 공통 로직이 필요하면 작성
// But 아직 그런 용도가 없음 언젠가 생기면 쓰는걸로...
void UScene::BeginPlay() {}
void UScene::Tick(float DeltaTime) {
    //for (auto actor : Objects) {
    //    actor.tick(delta);
    //}
}
void UScene::EndPlay() {}

void UScene::CreateMainCamera()
{
    MainCamera = SpawnObject<UCameraComponent*>(UCameraComponent::GetClass());
}

void UScene::Serialize(TArray<FArchive>& ObjectInfoList)
{
    for (auto Item : Objects)
    {
        FArchive Archive;
        Archive.SetUInt32("UUID", Item->GetUUID());

        Item->Serialize(Archive);
        ObjectInfoList.Add(Archive);
    }
}

void UScene::Deserialize(TArray<FArchive>& ObjectInfoList)
{
    for (auto& Item : ObjectInfoList)
    {
        FString TypeName = Item.GetString("Type");
        FClassType* Type = FClassRegistry::FindClassType(TypeName);

        uint32 UUID = Item.GetUInt32("UUID");
        UObject* Object = FObjectFactory::ConstructSceneObject(Type, UUID);
        Objects.Add(Object);
        
        Object->Deserialize(Item);
    }
}

void UScene::Destroy(UObject* Object)
{
    for (int i = 0; i < Objects.Num(); i++)
    {
        if (Objects[i] == Object)
        {
            delete Object;
            Objects.RemoveAt(i);
            break;
        }
    }
}

UScene::~UScene()
{
    for (auto Item : Objects)
    {
        delete Item;
    }

    Objects.Empty();
}
