#include "pch.h"
#include "File.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <format>


void File::WriteText(FStringView Path, FStringView Text)
{
	FString PathString{ Path };

	std::ofstream Out{ PathString };

	if (!Out.is_open())
	{
		throw std::runtime_error(std::format("파일을 작성할 수 없습니다: {}", Path));
	}

	Out << Text;

	Out.close();
}

FString File::ReadText(FStringView Path)
{
	FString PathString{ Path };

	std::ifstream In{ PathString };
	std::stringstream StringStream;

	if (!In.is_open())
	{
		throw std::runtime_error(std::format("파일을 불러올 수 없습니다: {}", Path));
	}

	FString Text;

	StringStream << In.rdbuf();

	Text = StringStream.str();

	In.close();

	return Text;
}
