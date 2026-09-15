#pragma once

#include <Windows.h>
#include <filesystem>
#include <optional>

#include "Core/Container/FString.h"

enum class EFileDialogType
{
	Json,
	Image,
	All
};

namespace File
{
	void WriteText(FStringView Path, FStringView Text);
	FString ReadText(FStringView Path);
	// 한글 경로 대응용: wide 경로로 파일을 연다
	FString ReadTextFromPath(const std::filesystem::path& Path);

	// .json 파일 선택 대화상자. 취소/실패 시 std::nullopt
	// 반드시 메인(UI) 스레드에서 호출할 것 (STA 필요)
	std::optional<std::filesystem::path> OpenFileDialog(HWND Owner = nullptr, EFileDialogType Type = EFileDialogType::All, const std::filesystem::path& InitialDir = {});
};
