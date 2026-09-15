#pragma once

#include "Core/Container/FString.h"

#include <filesystem>

struct FSceneType;
class UScene;

// Singleton
class GSceneManager
{
public:
	static GSceneManager* GetInstance();

	void Initialize();
	void Release();

	void Tick(float DeltaTime);

	void LoadScene(FSceneType* SceneType, FStringView SerializedName = "");
	// 파일 대화상자 등에서 얻은 전체 경로로 씬을 로드
	void LoadSceneFromPath(FSceneType* SceneType, const std::filesystem::path& ScenePath);
	void SaveScene(FStringView SerializedName);

	UScene* GetScene() { return CurrentScene; };

private:

	void InternalLoadScene();
	void ClearNextScene();

	UScene* CurrentScene = nullptr;
	FSceneType* NextScene = nullptr;
	FString NextSceneFile = "";
	std::filesystem::path NextScenePath;

	// 싱글톤
	GSceneManager() = default;
	~GSceneManager() = default;
	GSceneManager(const GSceneManager&) = delete;
	GSceneManager& operator=(const GSceneManager&) = delete;
};

