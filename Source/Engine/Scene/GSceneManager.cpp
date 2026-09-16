#include "pch.h"
#include "GSceneManager.h"
#include "Core/Container/FString.h"
#include "Engine/Object/FArchive.h"
#include "Engine/Object/FClassRegistry.h"
#include "Engine/Object/GObjectStatics.h"
#include "Engine/Scene/UScene.h"
#include "Core/Util/File.h"
#include "Engine/Log.h"
#include "nlohmann/json.hpp"

#include "Engine/Scene/SceneValidation.h"
#include <array>
#include <memory>
#include <unordered_set>
#include <vector>
#include <charconv>
#include <filesystem>
#include <limits>
#include <optional>
#include <stdexcept>

namespace
{
	constexpr FStringView SceneDirectory = "Scenes";

	FString GetScenePath(FStringView SceneName)
    {
        FString Name{SceneName};
        if (Name.empty() || Name.back() == '.' || Name.back() == ' ' ||
            Name.find_first_of("\\/:*?\"<>|") != FString::npos)
            throw std::runtime_error("Invalid scene name");
        for (unsigned char C : Name)
            if (C < 32) throw std::runtime_error("Invalid scene name");
        FString Base = Name.substr(0, Name.find('.'));
        for (char& C : Base) if (C >= 'a' && C <= 'z') C -= ('a' - 'A');
        const bool Numbered = Base.size() == 4 &&
            (Base.starts_with("COM") || Base.starts_with("LPT")) && Base[3] >= '1' && Base[3] <= '9';
        if (Base == "CON" || Base == "PRN" || Base == "AUX" || Base == "NUL" || Numbered)
            throw std::runtime_error("Reserved scene name");
        return (std::filesystem::path(SceneDirectory) / (Name + ".json")).generic_string();
    }
}

GSceneManager* GSceneManager::GetInstance()
{
    static GSceneManager Instance{};
    return &Instance;
}

void GSceneManager::Initialize()
{
	LoadScene(UScene::GetStaticSceneType(), "");
}

void GSceneManager::Release()
{
    ClearNextScene();
	if (CurrentScene)
	{
		CurrentScene->EndPlay();

		delete CurrentScene;
		CurrentScene = nullptr;
	}
}

void GSceneManager::Tick(float DeltaTime)
{
	if (CurrentScene)
	{
		CurrentScene->Tick(DeltaTime);
	}

	if (NextScene)
	{
		InternalLoadScene();
	}
}

void GSceneManager::LoadScene(FSceneType* SceneType, FStringView SerializedName)
{
	NextScene = SceneType;
	NextSceneFile = SerializedName;
	NextScenePath.clear();

	if (!CurrentScene)
	{
		InternalLoadScene();
	}
}

void GSceneManager::LoadSceneFromPath(FSceneType* SceneType, const std::filesystem::path& ScenePath)
{
	NextScene = SceneType;
	NextSceneFile.clear();
	NextScenePath = ScenePath;

	if (!CurrentScene)
	{
		InternalLoadScene();
	}
}

void GSceneManager::ClearNextScene()
{
	NextScene = nullptr;
	NextSceneFile.clear();
	NextScenePath.clear();
}

bool ValidateSceneJSON(const nlohmann::json& Root)
{
    try
    {
        if (!Root.is_object() || !Root.at("Version").is_number_integer() || Root.at("Version") != 1)
            return false;
        const auto& Next = Root.at("NextUUID");
        if (!Next.is_number_integer()) return false;
        const uint32 NextUUID = ParseSceneUUID(Next.dump(), true);
        const auto& Primitives = Root.at("Primitives");
        if (!Primitives.is_object()) return false;
        for (const auto& Item : Primitives.items())
        {
            if (ParseSceneUUID(Item.key()) >= NextUUID) return false;
            const auto& Object = Item.value();
            if (!Object.is_object() || !Object.at("Type").is_string()) return false;
			const FName TypeName(Object.at("Type").get<FString>());

			const FResolvedSceneType Resolved = ResolveSceneType(TypeName);
			if (!Resolved.IsValid()){return false;}
        }
        return true;
    }
    catch (const nlohmann::json::exception&) { return false; }
    catch (const std::runtime_error&) { return false; }
}

void GSceneManager::InternalLoadScene()
{
	if (NextScene == nullptr || NextScene->SceneConstructor == nullptr)
	{
		ClearNextScene();
		return;
	}

	TArray<FArchive> ObjectInfoList;
	uint32 NextUUID = 0;

	// 현재 Scene을 제거하기 전에 파일 전체를 파싱하고 검증합니다.
	try
	{
		// 경로 지정 로드가 우선, 없으면 이름 기반 로드, 둘 다 없으면 빈 씬
		std::optional<FString> FileText;
		if (!NextScenePath.empty())
		{
			FileText = File::ReadTextFromPath(NextScenePath);
		}
		else if (!NextSceneFile.empty())
		{
			FileText = File::ReadText(GetScenePath(NextSceneFile));
		}

		if (FileText)
		{
			const nlohmann::json FileJSON = nlohmann::json::parse(*FileText);

			if (!ValidateSceneJSON(FileJSON))
			{
				throw std::runtime_error("JSON 형식이 올바르지 않습니다.");
			}

			NextUUID = FileJSON.at("NextUUID").get<uint32>();

			const nlohmann::json& List = FileJSON.at("Primitives");
			for (const auto& Item : List.items())
			{
				const uint32 UUID = std::stoi(Item.key());
				FArchive Archive{ Item.value() };
				Archive.SetUInt32("UUID", UUID);
				ObjectInfoList.Add(Archive);
			}
		}
	}
	catch (const std::exception& Error)
	{
		const FString SceneLabel = NextScenePath.empty() ? NextSceneFile : NextScenePath.filename().string();
		UE_LOG("[SceneManger] 저장된 {} 씬 로드 실패: {}", SceneLabel, Error.what());
		ClearNextScene();
		return;
	}

	// 검증을 통과한 뒤 기존 Scene을 교체합니다.

	//2.[P1]씬 로드의 예외 경계가 너무 좁음
	if (CurrentScene)
	{
		CurrentScene->EndPlay();
		delete CurrentScene;
		CurrentScene = nullptr;
	}

	GObjectStatics::SetNextUUID(EObjectDomain::EOT_Scene, NextUUID);
	CurrentScene = NextScene->SceneConstructor();
	if (CurrentScene == nullptr)
	{
		UE_LOG("[SceneManger] {} Scene 생성 실패", NextScene->Name);
		ClearNextScene();
		return;
	}

	CurrentScene->Deserialize(ObjectInfoList);
	CurrentScene->BeginPlay();

	ClearNextScene();
}


void GSceneManager::SaveScene(FStringView SerializedName)
{
    if (!CurrentScene || SerializedName.empty()) return;
    try
    {
        const FString FileName = GetScenePath(SerializedName);
        TArray<FArchive> ObjectInfoList;
        CurrentScene->Serialize(ObjectInfoList);
        auto Objects = nlohmann::json::object();
        for (auto& Item : ObjectInfoList)
        {
            const FString UUID = std::to_string(Item.GetUInt32("UUID"));
            if (Objects.contains(UUID)) throw std::runtime_error("Duplicate UUID while saving");
            Objects[UUID] = Item.GetJSON();
        }
        nlohmann::json Root;
        Root["Version"] = 1;
        Root["NextUUID"] = GObjectStatics::GetNextUUID(EObjectDomain::EOT_Scene);
        Root["Primitives"] = std::move(Objects);
        if (!ValidateSceneJSON(Root)) throw std::runtime_error("Invalid scene data while saving");
        std::filesystem::create_directories(SceneDirectory);
        File::WriteText(FileName, Root.dump());
    }
    catch (const std::exception& Error)
    {
        UE_LOG("[SceneManager] Save {} failed: {}", SerializedName, Error.what());
    }
}
