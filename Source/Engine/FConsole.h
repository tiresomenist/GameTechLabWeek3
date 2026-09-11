#pragma once

#include "Core/Container/TDeque.h"
#include "Core/Container/FString.h"

#include "Core/Core.h"

class FConsole
{
private:
	TDeque<FString> MessageList;
	int32 MaxMessages = 100;

public:
	const TDeque<FString>& Get() const;

	void Initialize();

	void Append(FStringView Message);
	void Clear();

	void SetMaxMessages(int32 Num);

};
