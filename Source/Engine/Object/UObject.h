#pragma once

#include "Engine/Object/FClassRegistry.h"
#include "Engine/Object/FClassType.h"
#include "Engine/Object/EObjectDomain.h"
#include "Core/Core.h"
#include "Core/Container/FString.h"

/// <summary>
/// UObject 클래스 정의를 생성합니다. UObject로 부터 상속받는 모든 클래스는
/// 이 매크로를 사용해야만 합니다.
/// </summary>
/// <param name="ClassName">클래스 이름</param>
/// <param name="SerializeName">JSON 직렬화에 사용될 문자열 (중복 금지!!)</param>
/// <param name="ParentName">부모 클래스 이름</param>
///////////////////////////////////////////////////////////////////////
#define UCLASS(ClassName, SerializeName, ParentName)                  \
private:                                                              \
	using Super = ParentName;                                         \
                                                                      \
protected:                                                            \
	ClassName(const FObjectCreateInfo& Info)						  \
		: Super{ Info }                        						  \
	{                                                                 \
	}                                                                 \
                                                                      \
public:                                                               \
	static FClassType* GetClass()                                     \
	{                                                                 \
		static auto CreateObject = [](const FObjectCreateInfo& Info)  \
			{                                                         \
				return new ClassName(Info); 						  \
			};                                                        \
                                                                      \
		static FClassType Type                                        \
		{                                                             \
			.Name = FName(SerializeName),                             \
			.ClassConstructor = CreateObject,                         \
			.ParentClassType = Super::GetClass(),                     \
		};                                                            \
                                                                      \
		return &Type;                                                 \
	}																  \
                                                                      \
private:                                                              \
                                                                      \
    static inline void* __INTERNAL__RegisteredClassType =			  \
		FClassRegistry::__INTERNAL__Add(GetClass());                  \
                                                                      \
///////////////////////////////////////////////////////////////////////

class FArchive;

/// <summary>
/// 객체를 만들때 전달할 구조체
/// </summary>
struct FObjectCreateInfo
{
	uint32 UUID;
	uint32 InternalIndex;
	FClassType* ClassType;
	EObjectDomain Domain;
};

/// <summary>
/// 엔진이 직접 생성 및 관리하는 모든 유형의 객체들을 정의합니다.
/// </summary>
class UObject
{
private:
	/// <summary>
	/// Scene을 JSON 직렬화/역직렬화 하는 과정에서도 Scene 내부의 UObject 사이의 참조 관계가 유지되도록 하기 위한 값입니다.
	/// </summary>
	uint32 UUID;

	/// <summary>
	/// 엔진 내부의 객체 배열에서 객체를 찾을 때 사용됩니다.
	/// </summary>
	uint32 InternalIndex;

	/// <summary>
	/// 이 객체가 엔진에서 어떤 도메인에 속하는지 나타냅니다.
	/// </summary>
	EObjectDomain Domain;

	FClassType* ClassType;

protected:

	UObject(const FObjectCreateInfo& Info);

public:

	UObject(const UObject&) = delete;
	UObject& operator=(const UObject&) = delete;
	static FClassType* GetClass();

	uint32 GetUUID() { return UUID; }
	uint32 GetInternalIndex() { return InternalIndex; }

	/// <summary>
	/// 이 객체의 타입 정보를 반환합니다.
	/// </summary>
	/// <returns>이 객체의 타입 정보</returns>
	FClassType* GetInstanceClass() const { return ClassType; }

	/// <summary>
	/// 이 객체가 주어진 타입의 자식인지 확인합니다.
	/// </summary>
	/// <param name="InClassType">자식 여부를 확인하려는 타입</param>
	/// <returns>자식 여부</returns>
	bool IsA(FClassType* InClassType) const;

	/// <summary>
	/// 객체의 초기화를 실행합니다.
	/// </summary>
	virtual void Initialize();
	
	/// <summary>
	/// 객체에 담긴 정보를 직렬화하여 Archive에 저장합니다.
	/// </summary>
	/// <param name="Archive"></param>
	virtual void Serialize(FArchive& Archive);
	
	/// <summary>
	/// Archive에 담긴 정보를 역직렬화하여 UObject에 저장합니다.
	/// </summary>
	/// <param name="Archive"></param>
	virtual void Deserialize(FArchive& Archive);

	void* operator new(size_t Size);
	void* operator new(size_t Size, std::align_val_t Alignment);
	void operator delete(void* Ptr);
    void operator delete(void* Ptr, std::align_val_t Alignment);

	virtual ~UObject();

};
