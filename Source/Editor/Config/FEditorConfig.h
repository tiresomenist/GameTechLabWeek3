#pragma once
#include "Core/Core.h"
#include "Core/Util/File.h"
#include "Engine/Renderer/FRenderer.h"
#include "sstream"
#include <filesystem>

namespace
{
	constexpr FStringView ConfigDirectory = "Source/Editor/Config";
	constexpr FStringView DefaultConfigFileName = "editor.ini";

	FString GetEditorConfigPath(FStringView ConfigFileName = DefaultConfigFileName)
	{
		namespace fs = std::filesystem;

		std::error_code Ec;
		fs::create_directories(fs::path(FString{ ConfigDirectory }), Ec);

		const fs::path FullPath = fs::path(FString{ ConfigDirectory }) / FString{ ConfigFileName };

		return FullPath.generic_string();
	}
}


struct FEditorConfig
{
	float CameraMoveSpeed = 20.0f;
	float GridInterval = 5.0f;
	bool bShowPrimitives = true;
	bool bShowUUIDLabels = true;
	bool bShowBoundingBoxes = true;
	EViewModeIndex ViewMode = EViewModeIndex::VMI_Lit;

	static FEditorConfig Load(FString FileName)
	{
		FEditorConfig Config;
		FString FileText;
		try
		{
			FileText = File::ReadText(GetEditorConfigPath(FileName));
		}
		catch (...)
		{
			return Config;
		}
		if (FileText.empty())
		{
			return Config;
		}

		size_t LineStart = 0;
		const size_t TextLength = FileText.length();

		while (LineStart < TextLength)
		{
			size_t LineEnd = FileText.find('\n', LineStart);
			if (LineEnd == FString::npos)
			{
				LineEnd = TextLength;
			}

			FString Line = FileText.substr(LineStart, LineEnd - LineStart);
			LineStart = LineEnd + 1;

			if (!Line.empty() && Line[Line.length() - 1] == '\r')
			{
				Line = Line.substr(0, Line.length() - 1);
			}

			if (Line.empty() || Line[0] == ';' || Line[0] == '#' || Line[0] == '[')
			{
				continue;
			}
			size_t DelimiterPos = Line.find('=');
			if (DelimiterPos == FString::npos)
			{
				continue;
			}

			FString Key = Line.substr(0, DelimiterPos);
			FString Value = Line.substr(DelimiterPos + 1);

			auto Trim = [](FString& S) {
				size_t First = S.find_first_not_of(" \t\r\n");
				if (First == FString::npos) { S.clear(); return; }
				size_t Last = S.find_last_not_of(" \t\r\n");
				S = S.substr(First, Last - First + 1);
				};
			Trim(Key);
			Trim(Value);

			const char* RawValue = Value.c_str();

			if (Key == "CameraMoveSpeed")
			{
				Config.CameraMoveSpeed = static_cast<float>(std::atof(RawValue));
			}
			else if (Key == "GridInterval")
			{
				Config.GridInterval = static_cast<float>(std::atof(RawValue));
			}
			else if (Key == "ShowPrimitives")
			{
				Config.bShowPrimitives = (Value == "true" || Value == "1");
			}
			else if (Key == "ShowUUIDLabels")
			{
				Config.bShowUUIDLabels = (Value == "true" || Value == "1");
			}
			else if (Key == "ShowBoundingBoxes")
			{
				Config.bShowBoundingBoxes = (Value == "true" || Value == "1");
			}
			else if (Key == "ViewMode")
			{
				int ModeIndex = std::atoi(RawValue);
				Config.ViewMode = static_cast<EViewModeIndex>(ModeIndex);
			}
		}

		return Config;
	}

	void Save(FString FileName = "editor.ini") const
	{
		std::stringstream Stream;
		Stream << "[Editor]\n";
		Stream << "CameraMoveSpeed=" << CameraMoveSpeed << "\n";
		Stream << "GridInterval=" << GridInterval << "\n";
		Stream << "ShowPrimitives=" << (bShowPrimitives ? "true" : "false") << "\n";
		Stream << "ShowUUIDLabels=" << (bShowUUIDLabels ? "true" : "false") << "\n";
		Stream << "ShowBoundingBoxes=" << (bShowBoundingBoxes ? "true" : "false") << "\n";
		Stream << "ViewMode=" << static_cast<int>(ViewMode) << "\n";

		try
		{
			const FString ResolvedPath = GetEditorConfigPath(FileName);
			File::WriteText(ResolvedPath, Stream.str().c_str());
		}
		catch (...)
		{
			// 파일 쓰기 실패 시 예외 (필요 시 UE_LOG 추가)
		}
	}
};