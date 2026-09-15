#include "pch.h"
#include "UObject.h"
#include "Engine/Object/GObjectStatics.h"
#include "Engine/Memory/GAllocator.h"
#include "Engine/Object/FArchive.h"
#include "Engine/Log.h"

FClassType* UObject::GetClass()
{
	static auto CreateObject = [](const FObjectCreateInfo& Info)
		{
			return new UObject(Info);
		};

	static FClassType Type
	{
		.Name = "Object",
		.ClassConstructor = CreateObject,
	};

    return &Type;
}

UObject::UObject(const FObjectCreateInfo& Info)
	: UUID{ Info.UUID }
	, InternalIndex{ Info.InternalIndex }
	, ClassType{ Info.ClassType }
	, Domain{ Info.Domain }
	, Name{ Info.ClassType->Name,Info.NameNumber }
{
}

bool UObject::IsA(FClassType* InClassType) const
{
	const FClassType* CurrentType = ClassType;

	// 포인터 노드를 순회하며 타입을 검색합니다.
	while (CurrentType != nullptr)
	{
		if (CurrentType == InClassType)
		{
			return true;
		}

		CurrentType = CurrentType->ParentClassType;
	}

	return false;
}

void UObject::Initialize()
{
}

void* UObject::operator new(size_t Size)
{
    void* Ptr = GAllocator::Allocate(Size);
    if (!Ptr) throw std::bad_alloc();
    return Ptr;
}

void* UObject::operator new(size_t Size, std::align_val_t Alignment)
{
    void* Ptr = GAllocator::Allocate(
		Size,
		static_cast<size_t>(Alignment)
	);
    if (!Ptr) throw std::bad_alloc();
    return Ptr;
}

void UObject::operator delete(void* Ptr)
{
	GAllocator::Free(Ptr);
}

void UObject::operator delete(void* Ptr, std::align_val_t)
{
    GAllocator::Free(Ptr);
}

UObject::~UObject()
{
	GObjectStatics::Unregister(InternalIndex, this);
}

void UObject::Serialize(FArchive& Archive)
{
	// FClassType의 Serialize 이름 지정
	Archive.SetString("Type", ClassType->Name);
	Archive.SetString("Name", Name.ToString());
}

void UObject::Deserialize(FArchive& Archive)
{
	if (Archive.Contains("Name"))
	{
		Name = FName(Archive.GetString("Name"));
	}
}
