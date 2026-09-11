#include "pch.h"
#include "GObjectStatics.h"
#include "Engine/Object/UObject.h"

#include <iostream>

void GObjectStatics::SetNextUUID(EObjectDomain Domain, uint32 UUID)
{
	NextUUID[static_cast<size_t>(Domain)] = UUID;
}

void GObjectStatics::AddObject(UObject* Object)
{
	//6. [P2] 삭제한 전역 객체 슬롯이 영구 누적
	ObjectArray.Add(Object);
}

void GObjectStatics::DestoryObject(uint32 InternalIndex)
{
	ObjectArray[InternalIndex] = nullptr;
}

uint32 GObjectStatics::GetNextIndex()
{
	return ObjectArray.Num();
}

void GObjectStatics::Release()
{
	for (auto Item : ObjectArray)
	{
		if (Item != nullptr)
		{
			delete Item;
		}
	}

	ObjectArray.Empty();
}
