#pragma once

#include "Container/FString.h"

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
	void SaveScene(FStringView SerializedName);

	UScene* GetScene() { return CurrentScene; };

private:

	void InternalLoadScene();

	UScene* CurrentScene = nullptr;
	FSceneType* NextScene = nullptr;
	FString NextSceneFile = "";

	// 싱글톤
	GSceneManager() = default;
	~GSceneManager() = default;
	GSceneManager(const GSceneManager&) = delete;
	GSceneManager& operator=(const GSceneManager&) = delete;
};

