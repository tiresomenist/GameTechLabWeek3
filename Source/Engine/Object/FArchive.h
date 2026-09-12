#pragma once

#include "Core/Container/FString.h"
#include "Core/Container/TArray.h"
#include "Core/Core.h"
#include <type_traits>
#include <array>
#include <cmath>
#include <limits>

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
	bool Contains(const FString& Key) const { return Object.contains(Key); }

    std::array<float, 3> GetVector3OrDefault(const FString& Key, float Default) const
    {
        std::array<float, 3> Result{Default, Default, Default};
        const auto It = Object.find(Key);
        if (It == Object.end() || !It->is_array() || It->size() != 3) return Result;
        const double Limit = (std::numeric_limits<float>::max)();
        for (size_t I = 0; I < 3; ++I)
        {
            const auto& Value = (*It)[I];
            if (!Value.is_number()) continue;
            const double Number = Value.get<double>();
            if (std::isfinite(Number) && Number >= -Limit && Number <= Limit)
                Result[I] = static_cast<float>(Number);
        }
        return Result;
    }

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
