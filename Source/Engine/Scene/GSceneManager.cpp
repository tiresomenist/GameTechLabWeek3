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
    NextScene = nullptr;
    NextSceneFile.clear();
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

	if (!CurrentScene)
	{
		InternalLoadScene();
	}
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
            const FString Type = Object.at("Type").get<FString>();
            if (!IsAllowedSceneType(Type) || !FClassRegistry::FindClassType(FName(Type))) return false;
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
		NextScene = nullptr;
		NextSceneFile = "";
		return;
	}

	TArray<FArchive> ObjectInfoList;
	uint32 NextUUID = 0;

	// 현재 Scene을 제거하기 전에 파일 전체를 파싱하고 검증합니다.
	try
	{
		if (!NextSceneFile.empty())
		{
			const FString FileName = GetScenePath(NextSceneFile);

			const FString FileText = File::ReadText(FileName);
			const nlohmann::json FileJSON = nlohmann::json::parse(FileText);

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
		UE_LOG("[SceneManger] 저장된 {} 씬 로드 실패: {}", NextSceneFile, Error.what());
		NextScene = nullptr;
		NextSceneFile = "";
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
		NextScene = nullptr;
		NextSceneFile = "";
		return;
	}

	CurrentScene->Deserialize(ObjectInfoList);
	CurrentScene->BeginPlay();


	NextScene = nullptr;
	NextSceneFile = "";
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
