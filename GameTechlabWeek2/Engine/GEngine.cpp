#include "GEngine.h"
#include "Windows.h"

#include "Engine/Object/FObjectFactory.h"
#include "Engine/Object/GObjectStatics.h"
#include "Engine/Object/UObject.h"
#include "Engine/Core.h"
#include "Engine/Log.h"

#include "Engine/Editor/FEditor.h"
#include "Engine/GSceneManager.h"
#include "Engine/FConsole.h"

#include "GDevice.h"
#include "GResourceManager.h"

#include <chrono>
#include <stdexcept>

#include "Editor/Window/UConsoleWindow.h"
#include "Editor/Window/UEditorWindow.h"
#include "Editor/Window/UPropertyWindow.h"
#include "Editor/Window/USceneWindow.h"

float GEngine::GetTime()
{
	static auto Start = std::chrono::steady_clock::now();
	auto Now = std::chrono::steady_clock::now();

	return std::chrono::duration<float>(Now - Start).count();
}

GEngine* GEngine::GetInstance()
{
    static GEngine Instance{};
    return &Instance;
}

// 엔진을 초기 상태로 초기화합니다.
void GEngine::Initialize(HWND InHwnd)
{
    try
    {
        // 콘솔 초기화
        Console = new FConsole();
        Console->Initialize();

        // Device 초기화
        // DirectX 백버퍼 크기를 실제 윈도우 클라이언트 크기에 맞춥니다.
        // Main() { CreateWindwoExW(... 1024,1024 ...) }  -> 제목 표시줄과 테두리를 포함한 전체 창 크기
        // GetClientRect() -> 제목 표시줄과 테두리를 제외한 클라이언트 영역
        // 이를 사용함으로 프로그램 사용 초기 Imgui출력 위치가 이상한 문제가 해결됩니다.
        RECT ClientRect{};
        if (!GetClientRect(InHwnd, &ClientRect)) throw std::runtime_error("GetClientRect failed");
        const uint32 ClientWidth = static_cast<uint32>(ClientRect.right - ClientRect.left);
        const uint32 ClientHeight = static_cast<uint32>(ClientRect.bottom - ClientRect.top);
        GDevice& Device = *GDevice::GetInstance();
        Device.Initialize(InHwnd, ClientWidth, ClientHeight);

        // 리소스 매니저 초기화
        GResourceManager& ResourceManager = *GResourceManager::GetInstance();
        ResourceManager.Initialize(&Device);

        // 렌더러 초기화
        Renderer.Create(InHwnd, &Device);

        // 씬 매니저 초기화
        GSceneManager* SceneManager = GSceneManager::GetInstance();
        SceneManager->Initialize();

        // 에디터 초기화
        Editor = new FEditor();
        Editor->Initialize();

        StartTime = GetTime();
        LastTickTime = GetTime();
    }
    catch (...)
    {
        Destroy();
        throw;
    }
}

// 엔진의 메인 게임 루프를 실행합니다.
void GEngine::Tick()
{
	float DeltaTime = GetTime() - LastTickTime;
	LastTickTime = GetTime();

	if (DeltaTime > 0.1f)
	{
		UE_LOG("[경고] 프레임 업데이트 시간이 100ms를 초과했습니다. 걸린 시간: {:.1f} ms", DeltaTime * 1000);
	}

	// 게임 로직을 수행합니다.
	GSceneManager* SceneManager = GSceneManager::GetInstance();
	SceneManager->Tick(DeltaTime);

	Editor->Tick(DeltaTime);

	// 게임 화면을 렌더링합니다.
	UScene* CurrentScene = SceneManager->GetScene();
	Renderer.Render(DeltaTime, Editor, CurrentScene);
}

// 엔진의 자원을 정리합니다.
void GEngine::Destroy()
{
	// 에디터 정리
	if (Editor) Editor->Release();
	delete Editor;
	Editor = nullptr;

	// 씬 매니저 정리
	GSceneManager* SceneManager = GSceneManager::GetInstance();
	SceneManager->Release();

	// GObjectStatics 정리
	GObjectStatics::Release();

	//렌더러 해제
	Renderer.Shutdown();

	// 리소스 매니저 정리
	GResourceManager::GetInstance()->Shutdown();

	//디바이스 해제
	GDevice::GetInstance()->Release();

	// 콘솔 정리
	delete Console;
	Console = nullptr;
}
