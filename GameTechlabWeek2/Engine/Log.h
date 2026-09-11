
#include "Engine/GEngine.h"
#include "Engine/FConsole.h"
#include "Container/FString.h"

#include <format>
#include <utility>

template <typename... Args>
void UE_LOG(std::format_string<Args...> Format, Args&&... Arguments)
{
	GEngine& Engine = *GEngine::GetInstance();
	FConsole* Console = Engine.GetConsole();
	if (!Console) return;

	FString Message = std::format(Format, std::forward<Args>(Arguments)...);
	Console->Append(Message);
}
