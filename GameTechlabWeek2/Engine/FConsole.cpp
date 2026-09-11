#include "FConsole.h"
#include "windows.h"

const TDeque<FString>& FConsole::Get() const
{
	return MessageList;
}

void FConsole::Initialize()
{
}

void FConsole::Append(FStringView Message)
{
	FString Item = FString(Message);
	MessageList.PushLast(Item); 

	if (MessageList.Num() > static_cast<size_t>(MaxMessages))
	{
		MessageList.PopFirst();
	}
}

void FConsole::Clear()
{
	MessageList.Reset();
}

void FConsole::SetMaxMessages(int32 Num)
{
	MaxMessages = (Num < 0) ? 0 : Num;

	while (MessageList.Num() > static_cast<size_t>(MaxMessages))
	{
		MessageList.PopFirst();
	}
}
