# 프로젝트 메모리·안정성 점검 — 2026-09-10

현재 작업 트리 기준이다. 기존 미커밋 소스 및 씬 파일은 수정하지 않았다. 아래 24개 항목은 코드로 확인한 결함과 특정 실패 조건에서 드러나는 위험을 함께 기록하며, 각 항목에 적용 조건을 명시했다. 모든 실행 상태에서 결함이 없음을 보증하는 보고서는 아니다.

**범위와 검증**

- Engine·Container 104개 파일, main, 벡터·행렬·쿼터니언, 모델 19개, 셰이더 2개, 빌드 설정과 소유권/호출 경로를 점검했다.
- Debug/Win32 빌드 및 MSVC 코드 분석 완료. 최초 PATH 중복과 샌드박스 FileTracker 제한을 해결한 후 빌드 성공.
- 별도 콘솔 검증 프로그램은 이번에 빌드한 실제 프로젝트 오브젝트 파일에 링크했다. 엔진 GUI를 실행하거나 기존 씬을 변경하지 않았다.
- 모델 19개의 정점 수와 인덱스를 검사했고, 현재 데이터에서 범위 밖 인덱스는 0개였다.
- ImGui/nlohmann은 컴파일러 분석과 프로젝트 연동 경로를 확인했다. 외부 라이브러리 전체 구현의 수동 전수 감사나 취약점 데이터베이스 조사는 수행하지 않았다.
- 실행 중 GPU live-object 수집, ASan 전체 앱 실행, 실제 메모리 부족/디바이스 제거/디스크 부족 주입은 미수행. 관련 항목은 코드 경로 근거다.
- 우선순위: P1 = 메모리 손상·크래시·데이터 유실 방지를 우선 수정, P2 = 누수·실패 복구·기능 안정성, P3 = 제한된 API 조건/진단 개선.

**1. [P1] 씬 좌표 배열 길이 미검사 — 범위 밖 읽기**

위치: [Engine/Object/USceneComponent.cpp:102](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Object/USceneComponent.cpp:102), [Engine/Object/FArchive.h:43](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Object/FArchive.h:43)

Location/Rotation/Scale의 원소 수를 확인하지 않고 [0], [1], [2]를 읽는다. 예를 들어 Location=[]인 Cube는 ValidateSceneJSON을 통과한다(검증 프로그램 확인). 이후 로드하면 vector의 범위 밖 접근으로 Debug assertion 또는 정의되지 않은 동작이 발생한다. 배열 여부, 정확히 3개인지, 각 원소가 유한한 숫자인지 먼저 검증해야 한다.

**2. [P1] 씬 로드의 예외 경계가 너무 좁음 — 기존 작업 유실**

위치: [Engine/GSceneManager.cpp:168](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GSceneManager.cpp:168), [Engine/Scene/UScene.cpp:36](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Scene/UScene.cpp:36)

검증은 Type 정도만 확인한다. 기존 CurrentScene을 삭제한 뒤 실행하는 ConstructEngineObject/Deserialize/BeginPlay는 try 밖이다. Location 키 누락이나 Camera 필수 필드 누락은 사전 검증을 통과하고, 기존 씬 삭제 후 예외로 프로그램을 종료시킬 수 있다. 임시 씬의 생성·역직렬화를 모두 성공시킨 뒤 교체하고, 실패 시 임시 객체와 UUID 상태를 롤백해야 한다. 단순 catch 추가만으로 이미 삭제된 씬은 복구되지 않는다.

**3. [P2] GDevice 전체 해제가 호출되지 않음 — COM/GPU 리소스 잔존**

위치: [Engine/GEngine.cpp:99](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GEngine.cpp:99), [Engine/GDevice.cpp:17](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GDevice.cpp:17), [Engine/FRenderer.cpp:80](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/FRenderer.cpp:80)

종료 경로에 GDevice::Release() 호출이 없다. SwapChain, DeviceContext, FrameBuffer/RTV, DepthStencilBuffer/DSV의 소유 참조가 남는다. 렌더러는 GetDevice()로 AddRef 없이 받은 포인터를 직접 Release하므로 소유권도 불일치한다. 단순히 GDevice::Release를 추가하면 동일 디바이스 소유 참조를 중복 반환할 수 있으므로 함께 고쳐야 한다. 컨텍스트 바인딩을 ClearState로 정리하고, 렌더러와 리소스 매니저를 정리한 뒤 디바이스 소유자가 최종 해제하도록 통일한다.

**4. [P2] DepthStencilState 3개 해제 누락**

위치: [Engine/FRenderer.cpp:46](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/FRenderer.cpp:46), [Engine/FRenderer.cpp:333](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/FRenderer.cpp:333), [Engine/FRenderer.cpp:354](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/FRenderer.cpp:354)

Shutdown에서 ReleaseDepthStencilStates를 호출하지 않는다. 해당 함수를 호출하더라도 HighlightDepthStencilState는 함수 안에 해제 코드가 없다. Default/Gizmo/Highlight 세 상태 모두 해제해야 한다. 현재 정상 종료에서도 재현 조건이 성립하는 정적 결함이다.

**5. [P2] 셰이더 성공 시 경고 blob 누수**

위치: [Engine/FRenderer.cpp:131](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/FRenderer.cpp:131)

errorBlob은 컴파일 실패 분기에서만 Release한다. 컴파일 성공과 함께 경고 blob이 반환되는 경우 참조를 잃는다. MainShader의 float4→float3 대입처럼 경고가 생길 수 있는 코드도 있다. 실제 경고 반환 여부는 런타임 셰이더 컴파일 검증이 필요하다. 결과와 무관하게 blob을 해제하거나 ComPtr로 관리해야 한다.

**6. [P2] 삭제한 전역 객체 슬롯이 영구 누적**

위치: [Engine/Object/GObjectStatics.cpp:11](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Object/GObjectStatics.cpp:11)

Destroy는 nullptr만 기록하고 새 객체는 항상 배열 끝에 추가된다. Spawn/Delete 또는 New/Load 반복으로 실제 UObject 수가 줄어도 전역 배열 크기는 계속 커진다. 검증 프로그램에서 동일 Add/Destroy 슬롯 연산 100,000회 후 빈 슬롯 100,000개가 남았다. 이는 현재 Win32에서 슬롯 데이터만 약 400KB이며 capacity 여유분은 별도다. UObject 힙 통계에는 보이지 않는다. 빈 슬롯 재사용이 필요하며, 단순 erase는 다른 객체 InternalIndex를 깨뜨리므로 금지해야 한다.

**7. [P3] 힙 singleton 4개가 해제되지 않음**

위치: [Engine/GEngine.cpp:34](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GEngine.cpp:34), [Engine/GDevice.cpp:6](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GDevice.cpp:6), [Engine/GSceneManager.cpp:29](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GSceneManager.cpp:29), [Engine/InputManager/GInputManager.cpp:5](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/InputManager/GInputManager.cpp:5)

new로 생성한 singleton 본체를 delete하지 않는다. 프로세스 생존 기간 유지 방식이며 프레임마다 증가하는 누수와는 다르지만, 명시적 종료 정리 및 CRT leak report 기준에서는 남는 할당이다. GResourceManager는 함수 정적 객체여서 이 항목에 해당하지 않는다. 함수 정적 객체 또는 명시적 singleton 수명 관리로 통일하되 종료 순서를 보장해야 한다.

**8. [P1, API 극단 입력] allocator 크기 덧셈 오버플로**

위치: [Engine/GAllocator.cpp:16](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GAllocator.cpp:16)

header + alignment padding + Size가 size_t 범위를 넘는지 검사하지 않는다. 실제 Win32 검증에서 Allocate(SIZE_MAX,16)가 예외 없이 성공하고 통계에 26바이트만 잡혔다. 검증에서는 반환된 payload에 쓰지 않고 안전하게 Free했다. 호출자가 요청 크기를 믿고 쓰면 힙 손상이다. 현재 UObject의 sizeof 요청은 작아 일반 UI에서 바로 발생하지 않지만 allocator 자체의 확정 결함이다. 덧셈 전 SIZE_MAX 기준으로 각 항을 검사해야 한다.

**9. [P2, 생성 실패/확장 조건] 객체 팩토리의 등록 순서와 예외 안전성**

위치: [Engine/Object/FObjectFactory.cpp:23](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Object/FObjectFactory.cpp:23), [Engine/Object/UObject.cpp:71](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Object/UObject.cpp:71)

ClassConstructor 반환 후 Initialize가 예외를 던지면 객체가 전역 배열에 등록되기 전이라 회수되지 않는다. AddObject의 할당 실패도 같은 문제다. 또한 파생 생성자가 예외를 던져 UObject 소멸자가 실행되면 아직 등록되지 않은 InternalIndex를 배열에 기록할 수 있다. Initialize 안에서 다른 UObject를 만드는 확장에서는 외부/내부 객체가 같은 next index를 예약하므로 등록 정보가 어긋난다. 생성 전에 슬롯을 예약하고, 등록 여부를 관리하며, 실패 시 객체와 슬롯을 함께 정리해야 한다. 현재 Initialize 구현에서 중첩 생성이 발생하는 경로는 확인하지 않았다.

**10. [P1, 그래픽 초기화 실패] 실패 결과를 무시하고 렌더링 계속**

위치: [Engine/GDevice.cpp:10](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GDevice.cpp:10), [Engine/FRenderer.cpp:24](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/FRenderer.cpp:24), [main.cpp:80](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/main.cpp:80)

CreateDeviceAndSwapChain 실패 후 CreateFrameBuffer에서 SwapChain을 역참조한다. 셰이더/버퍼/렌더 상태 및 ImGui 초기화 결과도 상위로 전달되지 않는다. RegisterClass/CreateWindow 결과도 미검사다. 그래픽 장치 사용 불가, debug layer 미설치, 리소스 파일 누락, 메모리 부족 등에서 초기화를 중단하고 부분 생성 리소스를 정리하는 경로가 필요하다.

**11. [P2, 리사이즈/디바이스 실패] null 렌더 타깃 상태로 다음 프레임 진행**

위치: [Engine/GDevice.cpp:40](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GDevice.cpp:40), [Engine/FRenderer.cpp:192](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/FRenderer.cpp:192), [Engine/GDevice.cpp:308](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GDevice.cpp:308)

기존 RTV/DSV를 먼저 해제한 뒤 ResizeBuffers가 실패하면 그대로 return한다. 재생성 실패도 복구하지 않으며 다음 프레임은 null RTV/DSV로 Clear를 호출한다. Present 결과를 무시해 device removed/reset을 감지하지도 않는다. 렌더 가능 플래그와 오류 전달, 재생성 성공 전 프레임 중단이 필요하다. DSV 생성 실패 시 먼저 만든 depth texture도 즉시 정리해야 한다.

**12. [P2, 할당/GPU 생성 실패] 메시 생성의 소유권 및 캐시 원자성 부족**

위치: [Engine/GResourceManager.h:64](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GResourceManager.h:64), [Engine/GResourceManager.cpp:68](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GResourceManager.cpp:68)

new 후 CPU 배열 복사 또는 캐시 삽입에서 예외가 발생하면 raw Mesh와 이미 생성한 COM 버퍼를 회수하지 못한다. VertexBuffer/IndexBuffer가 nullptr이어도 캐시에 저장하여 같은 이름 요청은 계속 불완전한 메시를 돌려준다. CPU 객체와 COM 리소스에 RAII를 적용하고, 두 버퍼 생성까지 모두 성공한 경우에만 캐시에 등록해야 한다.

**13. [P1, 비정상 씬 입력] UUID 충돌로 저장 시 객체 유실**

위치: [Engine/GSceneManager.cpp:87](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GSceneManager.cpp:87), [Engine/GSceneManager.cpp:153](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GSceneManager.cpp:153), [Engine/GSceneManager.cpp:202](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GSceneManager.cpp:202)

NextUUID가 기존 최대 UUID보다 큰지, UINT32 예약값/범위 및 중복 여부를 확인하지 않는다. UUID 0이 있는 파일에 NextUUID=0이면 다음 Spawn도 UUID 0을 받아, 저장할 때 JSON의 동일 키를 덮어쓴다. std::stoi는 '1junk'를 1로 받아들이고 uint32의 정상 범위인 2147483648은 거부한다(둘 다 실행 확인). 음수와 '1'/'01' 충돌도 차단해야 한다. 전체 문자열을 소비하는 from_chars(uint32)와 UUID 유일성/다음 값 검증을 사용한다.

**14. [P1, 저장 중 I/O 실패] 기존 파일을 먼저 잘라내고 쓰기 실패 미검사**

위치: [Engine/Util/File.cpp:14](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Util/File.cpp:14), [Engine/GSceneManager.cpp:214](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GSceneManager.cpp:214)

ofstream 기본 출력 모드는 기존 파일을 truncate한다. 이후 디스크 부족/쓰기·close 실패가 발생해도 스트림 상태를 확인하지 않아 catch에 도달하지 않을 수 있다. 기존 정상 씬이 빈 파일 또는 불완전한 파일로 바뀔 수 있다. 같은 디렉터리의 임시 파일에 쓰고 쓰기/flush/close 성공을 확인한 뒤 교체해야 한다.

**15. [P2, 사용자 씬 이름] Scenes 디렉터리 밖 읽기/덮어쓰기 허용**

위치: [Engine/GSceneManager.cpp:21](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GSceneManager.cpp:21)

씬 이름에 ../ 또는 절대 경로를 넣으면 Scenes 아래로 제한되지 않는다. SaveScene은 해당 사용자 권한 범위의 다른 .json 파일을 덮어쓸 수 있다. 네트워크 원격 취약점으로 주장하는 항목은 아니며 로컬 입력의 경로 경계 문제다. 파일명만 허용하거나 정규화한 최종 경로가 Scenes 내부인지 확인해야 한다.

**16. [P2] Scale Lock에서 0 나눗셈과 비율 왜곡**

위치: [Engine/Editor/Window/UPropertyWindow.cpp:130](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Editor/Window/UPropertyWindow.cpp:130)

Y/Z는 PrevScale이 0이어도 나눗셈한다. Scale Lock을 끈 채 Y 또는 Z를 0으로 만든 뒤 잠금을 켜고 해당 축을 수정하면 다른 축에 Inf/NaN이 전파될 수 있다. X는 이전 값을 0.0001~200으로만 보정하여 음수/200 초과 배율에서 실제 이전 값과 다른 비율을 적용한다. 세 축 공통으로 0 근방 처리 정책과 유한성 검사를 적용하고 실제 이전 값으로 계산해야 한다.

**17. [P2] ImGui 키보드 캡처 시 KEYUP 누락 — 이동키 고착**

위치: [Engine/InputManager/WndProc.cpp:28](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/InputManager/WndProc.cpp:28)

DisableKeyboard이면 KEYDOWN뿐 아니라 KEYUP까지 버린다. W를 누른 채 UI 입력으로 전환해 W를 놓으면 엔진의 W=true가 남을 수 있다. 이후 UI 캡처가 끝나면 카메라가 계속 움직인다. 이미 눌린 키의 release는 캡처와 관계없이 반영하고, 포커스 해제 때 bSpacePressPending도 초기화해야 한다.

**18. [P2, Map 실패] 실패 확인 전에 미초기화 pData 읽기**

위치: [Engine/FRenderer.cpp:475](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/FRenderer.cpp:475), [Engine/FRenderer.cpp:501](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/FRenderer.cpp:501)

D3D11_MAPPED_SUBRESOURCE를 초기화하지 않고 Map 성공 여부 확인 전에 pData를 읽는다. 성공 분기 내부에 동일 선언이 다시 있어 바깥 선언은 불필요하다. 현재 실패 포인터로 쓰는 동작은 없으므로 'Map 실패 즉시 임의 주소 쓰기'로 과장하면 안 된다. 구조체를 {} 초기화하고 SUCCEEDED 안에서만 pData를 접근한다.

**19. [P2] 씬 파일에서 에디터/엔진 클래스도 생성 가능**

위치: [Engine/GSceneManager.cpp:113](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GSceneManager.cpp:113), [Engine/Scene/UScene.cpp:40](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Scene/UScene.cpp:40)

등록 클래스이기만 하면 통과한다. 검증 프로그램에서 Type=SceneWindow도 허용됐다. 에디터 의존 객체를 Editor 없이 씬 객체로 생성하게 되어 도메인·초기화 계약이 깨진다. 현재 씬 렌더러가 모든 객체를 무조건 에디터로 호출하지는 않으므로 즉시 크래시로 단정하지 않는다. 저장 가능한 씬 타입을 명시적으로 제한해야 한다.

**20. [P2] TArray 메시 생성의 BoundsMin/BoundsMax 반대**

위치: [Engine/GResourceManager.cpp:86](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GResourceManager.cpp:86)

Min에 max, Max에 min을 적용한다. 현재 Move/Scale/Rotate 메시가 이 오버로드를 사용하여 잘못된 bounds를 갖는다. 일반 Cube/Sphere 등 C 배열 오버로드는 올바르게 계산한다. 기즈모 picker는 이 AABB를 사용하지 않아 현재 모든 피킹이 망가지는 것은 아니다. 해당 리소스를 AABB 검사에 사용하면 특히 Ray.Direction==0인 축에서 잘못 제외된다. 두 연산을 바로잡고 공통 구현으로 통합해야 한다.

**21. [P2, 신규 메시/외부 수정] C 배열 CreateMesh와 객체 picker의 인덱스 계약 미보호**

위치: [Engine/GResourceManager.h:68](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GResourceManager.h:68), [Engine/Editor/ObjectPicker/FObjectPicker.cpp:152](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Editor/ObjectPicker/FObjectPicker.cpp:152)

TArray CreateMesh는 인덱스를 검증하지만 C 배열 버전은 하지 않는다. FObjectPicker는 IndexCount 및 indexes의 값을 바로 배열 접근에 사용한다. 새 모델의 잘못된 인덱스 또는 공개 Mesh 필드 변경으로 CPU 범위 밖 읽기가 가능하다. 현재 19개 모델 데이터는 모두 범위 내임을 확인했다. 생성 시 검증을 공통화하고 리소스 필드를 외부에서 불변으로 유지해야 한다.

**22. [P2, 비정상 수치] NaN/Inf를 성공한 역행렬로 반환**

위치: [Matrix.cpp:112](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Matrix.cpp:112), [Engine/Object/USceneComponent.cpp:7](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Object/USceneComponent.cpp:7), [Engine/Object/UCameraComponent.cpp:127](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Object/UCameraComponent.cpp:127)

TryInverse는 MaxValue <= epsilon만 검사해 NaN 비교가 false이면 계속 진행한다. 실행 검증에서 NaN 행렬에 true를 반환했고 결과도 비유한 값이었다. Location/Scale 및 카메라 역직렬화도 유한성/범위를 충분히 검사하지 않아 renderer·picker로 잘못된 값이 전파된다. 입력과 계산 결과의 isfinite 검사, 카메라 FOV/near/far/aspect 범위 검증을 적용해야 한다. 정상 쿼터니언 Normalize의 방어는 이미 존재한다.

**23. [P2, 진단] 현재 누수 출력은 실제 누수와 생존 객체가 섞임**

위치: [Engine/FRenderer.cpp:74](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/FRenderer.cpp:74), [main.cpp:122](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/main.cpp:122), [Engine/Editor/Window/USceneWindow.cpp:120](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Editor/Window/USceneWindow.cpp:120)

D3D live report 시점에는 GDevice 소유 객체가 아직 남아 있고 ClearState도 하지 않았다. 렌더러 내부 CRT dump는 Console 삭제보다도 빠르며 main 반환 직전 dump 역시 정적 객체 소멸 전이다. 두 수동 dump와 자동 종료 dump를 구분해야 한다. UObject 통계는 STL 배열/문자열, 일반 new, ImGui, GPU 메모리를 포함하지 않는다. size_t를 %d로 출력하는 타입 불일치도 수정해야 한다(현재 Win32는 폭이 같지만 x64에서는 잘림). 최종 해제 이후 진단하고 전체 프로세스/COM/커스텀 allocator 통계를 분리한다.

**24. [P3, API 잘못된 인수] 음수 콘솔 제한이 무제한 보관으로 바뀜**

위치: [Engine/FConsole.cpp:29](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/FConsole.cpp:29)

SetMaxMessages(-1)은 size_t 변환 뒤 거대한 양수가 되어 제한을 사실상 없앤다. 실제 검증에서 -1 설정 후 메시지 1,000개가 모두 남았다. 현재 UI에는 이 setter 호출이 없으므로 기본 100개 제한은 정상이다. 음수를 거부하거나 0으로 clamp한다.

**추가로 확인한 방어 부족 — 현재 기본 흐름의 재현 결함과 구분**

- [Engine/Object/FClassRegistry.cpp:40](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Object/FClassRegistry.cpp:40): 중복 클래스 이름에서 assert에 문자열 포인터를 전달해 항상 참이 된다. 현재 중복 이름을 확인한 것은 아니다. 중복 발생 시 명시적으로 실패시켜야 한다.
- [Engine/Object/UObject.h:129](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Object/UObject.h:129): aligned new에 대응하는 aligned delete가 없다. over-aligned 파생 객체 생성자 예외의 메모리 해제 계약을 보완해야 한다. 현재 over-aligned UObject 파생 타입은 확인하지 않았다.
- [Engine/Scene/UScene.h:43](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Scene/UScene.h:43) 및 [Engine/Editor/FEditor.cpp:237](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Editor/FEditor.cpp:237): 공개 Cast/Register API가 타입 확인 없이 static_cast한다. 현재 호출은 올바른 타입이지만 확장 시 잘못된 다운캐스트 위험이다.
- [Engine/Scene/UScene.cpp:51](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Scene/UScene.cpp:51): 직접 Destroy 호출은 MainCamera/에디터 선택 포인터를 무효화하지 않는다. 현재 UI Delete는 선택을 해제하고, UI Load도 먼저 해제하므로 이 경로를 실제 use-after-free로 보고하지 않았다. 외부 API에서 Destroy/Load를 호출하는 확장에는 소멸 알림이 필요하다.
- [Engine/GSceneManager.cpp:127](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GSceneManager.cpp:127): NextScene=null 입력을 방어하지 않는다. 잘못된 non-scene 타입은 pending 값을 지우지 않아 반복 검사할 수 있다. 현재 UI는 올바른 씬 타입을 제공한다.
- [Engine/Object/UObject.h:68](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Object/UObject.h:68), [Engine/Scene/UScene.h:13](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Scene/UScene.h:13): 객체 ID/소유 컨테이너가 있는 클래스의 암시적 복사 제한이 없다. UObject 복사는 같은 InternalIndex를 복제하고 UScene 복사는 raw 소유 포인터를 복제한다. 현재 복사 호출은 확인하지 않았다. noncopyable 명시가 적절하다.
- [Engine/GEngine.cpp:99](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/GEngine.cpp:99): Destroy는 두 번 호출하거나 부분 초기화 상태에서 호출하기 안전하지 않다. Console을 delete 후 nullptr로 만들지 않으며 UE_LOG도 null/종료 상태를 방어하지 않는다. 현재 main은 정상 초기화 후 한 번만 호출한다.
- [Container/TArray.h:74](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Container/TArray.h:74), [Container/TDeque.h:38](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Container/TDeque.h:38): 인덱스/빈 컨테이너 전제조건 검사가 없다. 일반 STL과 같은 사용 계약으로 볼 수 있으나, 외부 JSON을 소비하는 부분에서는 반드시 사전 검증이 필요하다.
- [Engine/Editor/Controller/FCameraController.cpp:34](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Editor/Controller/FCameraController.cpp:34): 회전 세분화 횟수가 마우스 delta에 비례하고 상한이 없다. 큰 누적 입력은 긴 프레임을 만들 수 있다. 입력 delta와 steps 상한을 검토한다.
- 씬 파일 크기/객체 개수 제한이 없고 ReadText→JSON→FArchive 복사로 큰 파일의 일시 메모리가 증가한다. 기본 Spawn 20개 제한은 한 번의 UI 요청만 제한한다.
- [Engine/Editor/Window/UConsoleWindow.cpp:43](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/Editor/Window/UConsoleWindow.cpp:43): 매 프레임 로그 전체를 복사한다. 현재 100개 제한으로 유계지만 큰 로그 문자열에서는 불필요한 할당 비용이다.
- [Engine/FRenderer.cpp:401](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/Engine/FRenderer.cpp:401): 투영행렬을 만든 뒤 aspect ratio를 갱신하여 리사이즈 직후 일부 경로에서 한 프레임 이전 비율을 사용한다.
- [main.cpp:106](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/main.cpp:106): WM_QUIT를 받았어도 루프 아래 Engine->Tick을 한 번 실행한다. 종료 플래그 확인 후 즉시 루프를 빠져나오는 편이 안전하다.
- [main.cpp:67](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/GameTechlabWeek2/main.cpp:67): 고정 _CrtSetBreakAlloc(159)는 디버거 실행 중 무관한 할당에서 중단시킨다. 특정 누수 추적 세션에서만 사용해야 한다.

**누수로 오해하면 안 되는 정상 경로**

- 정상 UI 객체 삭제/씬 교체는 UScene 소멸자를 통해 소유 객체를 delete하며 전역 슬롯도 nullptr로 만든다. 문제는 남는 빈 슬롯이다.
- EditorCamera는 FEditor::Release에서 직접 delete하지 않지만 정상 엔진 종료의 GObjectStatics::Release에서 회수된다. 현재 종료 경로에서 카메라 누수로 분류하지 않았다.
- 메시의 VertexBuffer/IndexBuffer를 여러 Primitive가 참조하는 것은 캐시 소유 공유다. 각 Primitive 소멸 시 COM Release를 추가하면 오히려 중복 해제가 될 수 있다.
- 현재 renderer는 primitive를 UI 삭제 전에 그리며, UI Load는 다음 tick에 씬을 교체한다. 저장해 둔 RenderList 포인터가 현재 순서에서 삭제 후 다시 사용되는 경로는 확인하지 않았다.
- 기본 콘솔은 100개로 제한된다. vector/deque capacity 보유나 초기 로드한 CPU 메시 복제는 그 자체로 무한 누수는 아니다.
- 컴파일러의 일부 C26495는 singleton의 new T() 값 초기화나 사용 전 Initialize를 모델링하지 못한 경고다. 실제 즉시 미초기화 역참조로 일괄 집계하지 않았다.
- ImGui C6011 두 건은 hovered_window/root 조건과 InputText state 설정 경로를 추가 확인했다. 이번 점검만으로 외부 라이브러리 확정 결함이라고 판단하지 않았다.

**검증 결과 원문**

```text
empty_location_accepted=1
missing_location_accepted=1
editor_type_accepted=1
uuid_trailing_text_accepted=1
high_uint32_accepted=high_uint32_exception=stoi argument out of range
registry_slots_after_100000_empty_cycles=100000
max_size_allocation_returned=1, actual_accounted_bytes=26
negative_console_limit_retained=1000
nan_matrix_inverse_success=1, result_is_finite=0
```

이 결과는 의도적으로 잘못된 입력이 현재 구현에 수용되는 것을 관측한 것이다. '테스트 통과 = 코드 안전'을 뜻하지 않는다. 배열 범위 밖 접근이나 거대한 메모리 쓰기를 실제로 수행해 프로세스를 손상시키지는 않았다.

- [빌드·분석 로그](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/tmp/memory-audit-build.log)
- [검증 코드](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/tmp/memory-audit/probe.txt)
- [검증 출력](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/tmp/memory-audit/probe-results.txt)
- [모델 인덱스 검사 결과](C:/Users/JUNGLE/source/repos/GameTechLabWeek3/tmp/memory-audit/model-check.csv)

**수정·후속 검증 순서**

1. 씬 스키마/UUID 검증과 임시 씬 교체, 안전한 파일 저장으로 데이터 유실·범위 밖 접근을 차단한다.
2. renderer/device 소유권을 통일하고 세 depth state와 shader blob을 정리한다.
3. allocator overflow와 factory 슬롯 예약/예외 정리, 전역 빈 슬롯 재사용을 수정한다.
4. 초기화/리사이즈/Present 실패 처리, Scale Lock과 입력 해제, 수치 유효성 검사를 추가한다.
5. 실제 앱에서 Spawn/Delete 및 New/Load 반복, 선택/드래그 중 교체, 리사이즈 반복을 ASan/디버거로 확인하고 종료 시 COM live report를 다시 수집한다. 디스크·GPU 실패 주입은 별도 격리된 환경에서 수행한다.

