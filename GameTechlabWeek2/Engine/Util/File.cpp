#include "pch.h"
#include "File.h"
#include <Windows.h>
#include <filesystem>
#include <system_error>

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <format>


void File::WriteText(FStringView Path, FStringView Text)
{
    namespace fs = std::filesystem;
    const fs::path Target = fs::absolute(fs::path(FString{Path}));
    wchar_t TempName[MAX_PATH]{};
    if (!GetTempFileNameW(Target.parent_path().c_str(), L"scn", 0, TempName))
        throw std::system_error(GetLastError(), std::system_category());
    const fs::path Temp{TempName};
    try
    {
        std::ofstream Out;
        Out.exceptions(std::ios::failbit | std::ios::badbit);
        Out.open(Temp, std::ios::binary | std::ios::trunc);
        Out << Text;
        Out.flush();
        Out.close();
        if (!MoveFileExW(Temp.c_str(), Target.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            throw std::system_error(GetLastError(), std::system_category());
    }
    catch (...)
    {
        std::error_code Ignored;
        fs::remove(Temp, Ignored);
        throw;
    }
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
