#pragma once

#include "Core/Container/FString.h"

namespace File
{
	void WriteText(FStringView Path, FStringView Text);
	FString ReadText(FStringView Path);
};