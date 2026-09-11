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

#include <charconv>
#include <filesystem>
#include <limits>
#include <stdexcept>

namespace
{
	constexpr FStringView SceneDirectory = "Scenes";

	FString GetScenePath(FStringView SceneName)
	{
		return (std::filesystem::path(SceneDirectory) / (FString{ SceneName } + ".json")).generic_string();
	}
}

GSceneManager* GSceneManager::GetInstance()
{
	static GSceneManager* SceneManager = new GSceneManager();
	return SceneManager;
}

void GSceneManager::Initialize()
{
	LoadScene(UScene::GetClass(), "");
}

void GSceneManager::Release()
{
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
	if (!Root.is_object())
	{
		// 주어진 JSON이 객체가 아님
		return false;
	}

	if (Root.at("Version").get<int32>() != 1)
	{
		// 버전이 다름
		return false;
	}

	const uint32 NextUUID = Root.at("NextUUID").get<uint32>();
	const nlohmann::json& Primitives = Root.at("Primitives");

	if (!Primitives.is_object())
	{
		// Primitives가 존재하지 않거나 객체가 아님
		return false;
    }

	for (const auto& Item : Primitives.items())
	{
		const uint32 UUID = std::stoi(Item.key());
		const nlohmann::json& Primitive = Item.value();

		if (!Primitive.is_object())
		{
			// Primitives 객체의 value가 객체가 아님
			return false;
		}

		const nlohmann::json& Type = Primitive.at("Type");
		if (!Type.is_string())
		{
			// 객체에 Type가 없음
			return false;
		}

		const FString TypeName = Type.get<FString>();
		if (FClassRegistry::FindClassType(TypeName) == nullptr)
		{
			// 주어진 객체의 Type이 프로그램에 존재하지 않음
			return false;
		}
	}

	return true;
}

void GSceneManager::InternalLoadScene()
{
	// UScene의 자식인지 체크
	if (!NextScene->IsA(UScene::GetClass())) { return; }

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
	UObject* RawPtr = FObjectFactory::ConstructEngineObject(NextScene);
	CurrentScene = static_cast<UScene*>(RawPtr);

	CurrentScene->Deserialize(ObjectInfoList);
	CurrentScene->BeginPlay();


	NextScene = nullptr;
	NextSceneFile = "";
}


void GSceneManager::SaveScene(FStringView SerializedName)
{
	if (CurrentScene == nullptr) { return; }
	if (SerializedName == "") { return; }

	TArray<FArchive> ObjectInfoList;
	CurrentScene->Serialize(ObjectInfoList);
	
	nlohmann::json ObjectArray = nlohmann::json::object();

	for (auto& Item : ObjectInfoList)
	{
		FString UUID = std::to_string(Item.GetUInt32("UUID"));
		ObjectArray[UUID] = Item.GetJSON();
	}

	uint32 NextUUID = GObjectStatics::GetNextUUID(EObjectDomain::EOT_Scene);

	nlohmann::json FileJSON;
	FileJSON["Version"] = 1;
	FileJSON["NextUUID"] = NextUUID;
	FileJSON["Primitives"] = ObjectArray;

	const FString FileName = GetScenePath(SerializedName);

	try
	{
		std::filesystem::create_directories(SceneDirectory);
		FString FileText = FileJSON.dump();
		File::WriteText(FileName, FileText);
	}
	catch (const std::exception& Error)
	{
		UE_LOG("[SceneManger] {} 씬 저장 실패: {}", FileName, Error.what());
	}
}
