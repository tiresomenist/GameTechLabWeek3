#pragma once

#include "Container/FString.h"
#include "Container/TArray.h"
#include "Engine/Core.h"
#include <type_traits>

#include "nlohmann/json.hpp"

// TODO: 언젠가는 이 코드가 JSON에 강하게 커플링 되어있는 문제를 해결해야할지도

class FArchive
{
private:
	nlohmann::json Object;

public:
	FArchive();
	explicit FArchive(const nlohmann::json& InObject);

	nlohmann::json GetJSON() const { return Object; }

	int32 GetInt32(const FString& Key);
	void SetInt32(const FString& Key, int32 Value);

	float GetFloat(const FString& Key);
	void SetFloat(const FString& Key, float Value);

	uint32 GetUInt32(const FString& Key);
	void SetUInt32(const FString& Key, uint32 Value);

	double GetDouble(const FString& Key);
	void SetDouble(const FString& Key, double Value);

	bool GetBool(const FString& Key);
	void SetBool(const FString& Key, bool Value);
	
	FString GetString(const FString& Key);
	void SetString(const FString& Key, const FString& Value);
	
	// GetArray는 필요하면 더 추가
	//1.[P1]씬 좌표 배열 길이 미검사
	template <typename T>
	TArray<T> GetArray(const FString& Key)
	{
		TArray<T> Array;

		for (const auto& Item : Object.at(Key))
		{
			T Value = Item.get<T>();
			Array.Add(Value);
		}

		return Array;
	}

	template <typename T>
	void SetArray(const FString& Key, const TArray<T>& Value)
	{
		Object[Key] = nlohmann::json::array();

		for (int i = 0; i < Value.Num(); ++i)
		{
			Object[Key].push_back(Value[i]);
		}
	}
};
