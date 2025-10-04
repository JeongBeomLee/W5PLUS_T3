# Week 5+ Task 3 구현 문서

## 📋 학습 목표

* Editor 모드와 PIE (Play In Editor) 모드의 역할과 차이점 이해
* 다중 World 생성, 소멸, 실행에 대한 이해
* Component Pattern을 이용한 Actor system 구축
* UWorld, ULevel, AActor, UActorComponent 클래스의 관계 이해

---

## 🎯 구현 내용

### 1\. Editor \& Rendering (눈에 보이는 세상)

#### PIE (Play In Editor) 구현

**파일 위치**: `EditorEngine.cpp`, `SViewportWindow.cpp`

PIE는 에디터 내에서 게임을 실행하는 기능으로, 다음과 같이 구현되었습니다:

##### EditorEngine의 World Context 관리

```cpp
// EditorEngine.cpp
void UEditorEngine::Tick(float DeltaSeconds)
{
    // PIE 월드가 있으면 PIE만 틱, 없으면 Editor만 틱
    UWorld\* PIEWorld = GetPIEWorld();

    if (PIEWorld)
    {
        // PIE 모드: PIE 월드만 틱
        TickPIEWorld(PIEWorld, DeltaSeconds);
    }
    else
    {
        // Editor 모드: Editor 월드만 틱
        UWorld\* EditorWorld = GetEditorWorld();
        if (EditorWorld)
        {
            TickEditorWorld(EditorWorld, DeltaSeconds);
        }
    }
}
```

**특징**

* PIE 실행 중에는 Editor 월드의 Tick을 중단하여 GameTime 중복 증가 방지
* 각 World Type에 맞는 Tick 함수 분리 (`TickEditorWorld`, `TickPIEWorld`)

##### PIE 시작 프로세스

```cpp
// SViewportWindow.cpp:337
void SViewportWindow::StartPIE()
{
    // 1. Editor 월드 복제
    PIEWorld = UWorld::DuplicateWorldForPIE(EditorWorld);

    // 2. PIE 월드를 WorldContext로 등록
    GEditor->CreateWorldContext(PIEWorld, EWorldType::PIE);

    // 3. GWorld를 PIE 월드로 전환
    GWorld = PIEWorld;

    // 4. ViewportClient를 PIE 월드로 전환
    ViewportClient->SetWorld(PIEWorld);

    // 5. Lit 모드로 강제 전환
    ViewportClient->SetViewModeIndex(EViewModeIndex::VMI\_Lit);

    // 6. 모든 액터 BeginPlay 호출
    PIEWorld->InitializeActorsForPlay();

    // 7. GameTime 초기화
    UUIManager::GetInstance().ResetFPSWidgetGameTime();
}
```

##### PIE 종료 프로세스

```cpp
// SViewportWindow.cpp:390
void SViewportWindow::EndPIE()
{
    // 1. ViewportClient를 Editor 월드로 복원
    ViewportClient->SetWorld(EditorWorld);
    ViewportClient->SetViewModeIndex(SavedViewModeIndex);

    // 2. GameTime 초기화
    UUIManager::GetInstance().ResetFPSWidgetGameTime();

    // 3. PIE 종료 요청 (다음 프레임에서 정리)
    GEditor->RequestEndPIE();
}
```

#### TextRenderComponent 구현

**파일 위치**: `TextRenderComponent.h`, `TextRenderComponent.cpp`

Billboard 방식으로 항상 카메라를 향하는 텍스트 렌더링 컴포넌트입니다.

**주요 기능**:

* UUID를 포함한 임의의 텍스트 렌더링
* 카메라 방향을 향하도록 자동 회전 (Billboard)
* WorldSpace, ScreenSpace 모드 지원
* 색상, 크기 커스터마이징

**Show Flag 통합**:

```cpp
// World.cpp:393
if (Cast<UTextRenderComponent>(Component) \&\& !IsShowFlagEnabled(EEngineShowFlags::SF\_BillboardText))
    continue;
```

#### Actor에 Component 추가

다양한 Component를 Actor에 동적으로 추가할 수 있도록 구현되었습니다:

```cpp
// Actor의 Component 관리
TArray<UActorComponent\*> Components;

// Component 추가 예시
UTextRenderComponent\* TextComp = NewObject<UTextRenderComponent>();
Actor->AddComponent(TextComp);
```

---

### 2\. Engine Core (눈에 안 보이는 세상)

#### UWorld 클래스

**파일 위치**: `World.h`, `World.cpp`

게임 세계를 나타내는 최상위 컨테이너 클래스입니다.

**주요 역할**:

* Level 관리 (Actor들의 컨테이너)
* Engine Actors 관리 (Camera, Grid, Gizmo 등)
* WorldType 구분 (Editor, PIE, Game)
* Rendering 및 Tick 관리
* Show Flags 관리

**WorldType 구분**:

```cpp
enum class EWorldType
{
    Editor,    // 에디터 모드
    PIE,       // Play In Editor 모드
    Game       // 독립 실행 게임 모드
};
```

**핵심 메서드**:

* `Initialize()`: 월드 초기화 (Camera, Grid, Gizmo 생성)
* `Tick(float DeltaSeconds)`: 모든 Actor들의 Tick 호출
* `Render()`: 렌더링 수행
* `DuplicateWorldForPIE()`: PIE용 월드 복제 (정적 메서드)

**PIE 월드 복제**:

```cpp
// World.cpp:897
UWorld\* UWorld::DuplicateWorldForPIE(UWorld\* EditorWorld)
{
    UWorld\* PIEWorld = NewObject<UWorld>();
    PIEWorld->SetWorldType(EWorldType::PIE);

    // Renderer, Camera 공유 (얕은 복사)
    PIEWorld->Renderer = EditorWorld->Renderer;
    PIEWorld->MainCameraActor = EditorWorld->MainCameraActor;

    // Level의 Actor들은 깊은 복사
    for (AActor\* EditorActor : EditorLevel->GetActors())
    {
        AActor\* PIEActor = Cast<AActor>(EditorActor->Duplicate());
        PIELevel->AddActor(PIEActor);
        PIEActor->SetWorld(PIEWorld);
    }

    return PIEWorld;
}
```

#### ULevel 클래스

**파일 위치**: `Level.h`, `Level.cpp`

Actor들의 컨테이너 역할을 하는 클래스입니다.

**주요 역할**:

* Actor 배열 관리
* Actor 추가/제거 인터페이스 제공

**핵심 메서드**:

```cpp
void AddActor(AActor\* Actor);
void RemoveActor(AActor\* Actor);
const TArray<AActor\*>\& GetActors() const;
```

**UWorld와의 관계**:

* UWorld는 하나의 ULevel을 소유
* ULevel은 여러 AActor를 소유
* 계층 구조: `UWorld -> ULevel -> AActor`

#### AActor 클래스

**파일 위치**: `Actor.h`, `Actor.cpp`

게임 세계에 배치할 수 있는 모든 오브젝트의 기본 클래스입니다.

**주요 역할**:

* Transform 관리 (위치, 회전, 크기)
* Component 소유 및 관리
* Tick 활성화/비활성화
* BeginPlay, Tick, EndPlay 라이프사이클

**Component Pattern 구현**:

```cpp
class AActor : public UObject
{
private:
    TArray<UActorComponent\*> Components;
    USceneComponent\* RootComponent;

public:
    void AddComponent(UActorComponent\* Component);
    void RemoveComponent(UActorComponent\* Component);
    TArray<UActorComponent\*> GetAllComponents() const;
};
```

**라이프사이클 메서드**:

* `BeginPlay()`: PIE 시작 시 호출
* `Tick(float DeltaTime)`: 매 프레임 호출
* `EndPlay(EEndPlayReason Reason)`: PIE 종료 시 호출

**Tick 제어**:

```cpp
bool ShouldTickInEditor() const;     // Editor에서 Tick 여부
bool CanTickInPlayMode() const;      // PIE/Game에서 Tick 여부
```

#### UActorComponent 클래스

**파일 위치**: `ActorComponent.h`, `ActorComponent.cpp`

Actor에 추가 기능을 부여하는 모듈형 컴포넌트의 기본 클래스입니다.

**주요 파생 클래스**:

* `USceneComponent`: Transform을 가진 컴포넌트

  * `UPrimitiveComponent`: 렌더링 가능한 컴포넌트

    * `UStaticMeshComponent`: 정적 메시 렌더링
    * `UTextRenderComponent`: 텍스트 렌더링

  * `UCameraComponent`: 카메라

* `UMovementComponent`: 이동 로직

  * `USimpleRotatingMovementComponent`: 자동 회전

**Component Pattern의 장점**:

1. **재사용성**: 같은 컴포넌트를 여러 Actor에 추가 가능
2. **유연성**: 런타임에 컴포넌트 추가/제거 가능
3. **모듈성**: 기능을 독립적인 단위로 분리
4. **확장성**: 새로운 컴포넌트 타입 쉽게 추가 가능

---

## 🔧 주요 클래스 관계도

```
UWorld (게임 세계)
  ├── ULevel (Actor 컨테이너)
  │     └── AActor\[] (배치된 오브젝트들)
  │           └── UActorComponent\[] (기능 모듈)
  │                 ├── USceneComponent (Transform)
  │                 │     ├── UPrimitiveComponent (렌더링)
  │                 │     │     ├── UStaticMeshComponent
  │                 │     │     └── UTextRenderComponent
  │                 │     └── UCameraComponent
  │                 └── UMovementComponent
  │
  ├── EngineActors\[] (엔진 전용)
  │     ├── ACameraActor
  │     ├── AGridActor
  │     └── AGizmoActor
  │
  └── WorldType (Editor | PIE | Game)
```

---

## 🎮 Editor vs PIE 모드 비교

| 특징 | Editor 모드 | PIE 모드 |
|------|------------|----------|
| \*\*World Type\*\* | `EWorldType::Editor` | `EWorldType::PIE` |
| \*\*목적\*\* | 레벨 편집, 배치 | 게임 플레이 테스트 |
| \*\*Actor Tick\*\* | `ShouldTickInEditor()` 체크 | `CanTickInPlayMode()` 체크 |
| \*\*BeginPlay\*\* | 호출 안 됨 | PIE 시작 시 호출 |
| \*\*GameTime\*\* | 증가 안 됨 | 증가함 |
| \*\*View Mode\*\* | 사용자 선택 | Lit 모드 강제 |
| \*\*Gizmo\*\* | 활성화 | 비활성화 |
| \*\*World 공유\*\* | - | Renderer, Camera 공유 |

---

## 💡 구현 세부 사항

### GameTime 관리

**파일 위치**: `FPSWidget.cpp`, `UIManager.cpp`

PIE 모드에서만 GameTime이 증가하도록 구현:

```cpp
// FPSWidget.cpp:43
void UFPSWidget::Update()
{
    if (GWorld->GetWorldType() == EWorldType::PIE ||
        GWorld->GetWorldType() == EWorldType::Game)
    {
        TotalGameTime += CurrentDeltaTime;
        // FPS 계산 등...
    }
}
```

**GameTime 초기화 타이밍**:

* PIE 시작 시: `SViewportWindow::StartPIE()` → `ResetFPSWidgetGameTime()`
* PIE 종료 시: `SViewportWindow::EndPIE()` → `ResetFPSWidgetGameTime()`

```

## 🏗️ 설계 패턴

### 1\. Component Pattern

Actor의 기능을 독립적인 컴포넌트로 분리하여 조합하는 패턴

**장점**:

* 코드 재사용성 향상
* 런타임 동적 구성 가능
* 단일 책임 원칙 준수



---

## 📁 주요 파일 구조

```
TL2/
├── World.h, World.cpp                    # World 클래스
├── Level.h, Level.cpp                    # Level 클래스
├── Actor.h, Actor.cpp                    # Actor 기본 클래스
├── ActorComponent.h, ActorComponent.cpp  # Component 기본 클래스
├── TextRenderComponent.h, .cpp           # 텍스트 렌더링 컴포넌트
├── EditorEngine.h, EditorEngine.cpp      # Editor 엔진 (World 관리)
├── SViewportWindow.h, .cpp               # Viewport (PIE 시작/종료)
├── UI/
│   ├── UIManager.h, .cpp                 # UI 관리자
│   └── Widget/
│       ├── FPSWidget.h, .cpp             # FPS \& GameTime 위젯
│       └── ShowFlagWidget.h, .cpp        # Show Flag 제어 위젯
└── WorldContext.h                        # World Context 구조체
```

---

## 🚀 실행 방법

1. **에디터 모드**:

   * 기본 실행 시 Editor 모드로 시작
   * Actor 배치, 편집 가능
   * GameTime 증가하지 않음

2. **PIE 모드 시작**:

   * Viewport 상단의 "Play (PIE)" 버튼 클릭
   * 또는 단축키 사용 (구현된 경우)
   * GameTime이 0부터 증가 시작

3. **PIE 모드 종료**:

   * Viewport 상단의 "Stop (PIE)" 버튼 클릭
   * GameTime이 0으로 리셋
   * Editor 모드로 복귀

---

## 📊 성능 고려사항

### Frustum Culling

* 카메라 시야 밖 오브젝트 렌더링 스킵
* 현재는 비활성화되어 있음 (`World.cpp:373-385`)
* 필요 시 주석 해제하여 활성화 가능

### Tick 최적화

* `ShouldTickInEditor()`, `CanTickInPlayMode()`로 불필요한 Tick 방지
* PIE 중에는 Editor 월드 Tick 스킵

### 메모리 관리

* PIE 종료 시 복제된 Actor들 자동 정리
* Renderer, Camera는 공유하여 메모리 절약

---

## 🔍 참고 자료

* Unreal Engine 공식 문서: World, Level, Actor 구조
* Component Pattern: Game Programming Patterns
* PIE 구현: Unreal Engine Editor 소스 코드 참고

---

## ✅ 구현 완료 체크리스트

* \[x] PIE (Play In Editor) 구현
* \[x] UWorld 클래스 구현
* \[x] ULevel 클래스 구현
* \[x] AActor 클래스 구현
* \[x] UActorComponent 클래스 구현
* \[x] TextRenderComponent 구현
* \[x] Component Pattern 적용
* \[x] Editor/PIE 모드 분리
* \[x] GameTime 관리 (PIE에서만 증가)
* \[x] Show Flag 시스템 구현
* \[x] World Context 관리
* \[x] Actor 라이프사이클 구현

---

**작성일**: 2025-10-02
**버전**: 1.0

