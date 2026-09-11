#include "GSceneManager.h"
#include "Container/FString.h"
#include "Engine/Object/FArchive.h"
#include "Engine/Object/FClassRegistry.h"
#include "Engine/Object/FObjectFactory.h"
#include "Engine/Object/GObjectStatics.h"
#include "Engine/Scene/UScene.h"
#include "Engine/Util/File.h"
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
	LoadScene(UScene::GetClass(), "");
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

void GSceneManager::LoadScene(FClassType* SceneType, FStringView SerializedName)
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
            if (!IsAllowedSceneType(Type) || !FClassRegistry::FindClassType(Type)) return false;
        }
        return true;
    }
    catch (const nlohmann::json::exception&) { return false; }
    catch (const std::runtime_error&) { return false; }
}

void GSceneManager::InternalLoadScene()
{
    FClassType* RequestedType = NextScene;
    FString RequestedFile = std::move(NextSceneFile);
    NextScene = nullptr;
    NextSceneFile.clear();
    if (!RequestedType || !RequestedType->IsA(UScene::GetClass())) return;

    constexpr size_t DomainCount = static_cast<size_t>(EObjectDomain::MAX_ITEMS);
    std::array<uint32, DomainCount> SavedUUIDs{};
    for (size_t I = 0; I < DomainCount; ++I)
        SavedUUIDs[I] = GObjectStatics::GetNextUUID(static_cast<EObjectDomain>(I));
    std::unique_ptr<UScene> Candidate;
    try
    {
        TArray<FArchive> ObjectInfoList;
        uint32 NextUUID = 0;
        if (!RequestedFile.empty())
        {
            const FString Text = File::ReadText(GetScenePath(RequestedFile));
            // Reject duplicate JSON keys instead of silently keeping the last value.
            std::vector<std::unordered_set<FString>> Keys;
            const auto Callback = [&Keys](int, nlohmann::json::parse_event_t Event, nlohmann::json& Parsed)
            {
                using EventType = nlohmann::json::parse_event_t;
                if (Event == EventType::object_start) Keys.emplace_back();
                else if (Event == EventType::object_end) Keys.pop_back();
                else if (Event == EventType::key && !Keys.back().insert(Parsed.get<FString>()).second)
                    throw std::runtime_error("Duplicate JSON key");
                return true;
            };
            const auto Root = nlohmann::json::parse(Text, Callback);
            if (!ValidateSceneJSON(Root)) throw std::runtime_error("Invalid scene JSON");
            NextUUID = ParseSceneUUID(Root.at("NextUUID").dump(), true);
            for (const auto& Item : Root.at("Primitives").items())
            {
                FArchive Archive{Item.value()};
                Archive.SetUInt32("UUID", ParseSceneUUID(Item.key()));
                ObjectInfoList.Add(Archive);
            }
        }
        GObjectStatics::SetNextUUID(EObjectDomain::EOT_Scene, NextUUID);
        Candidate.reset(static_cast<UScene*>(FObjectFactory::ConstructEngineObject(RequestedType)));
        Candidate->Deserialize(ObjectInfoList);
        Candidate->BeginPlay();
    }
    catch (const std::exception& Error)
    {
        Candidate.reset();
        for (size_t I = 0; I < DomainCount; ++I)
            GObjectStatics::SetNextUUID(static_cast<EObjectDomain>(I), SavedUUIDs[I]);
        UE_LOG("[SceneManager] Load {} failed: {}", RequestedFile, Error.what());
        if (!CurrentScene) throw; // Startup cannot continue without a scene.
        return;
    }
    if (CurrentScene)
    {
        CurrentScene->EndPlay();
        delete CurrentScene;
    }
    CurrentScene = Candidate.release();
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
