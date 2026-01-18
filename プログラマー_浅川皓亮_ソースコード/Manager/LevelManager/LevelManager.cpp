#include "Manager/LevelManager.h"
#include "Manager/ColorManager.h"
#include "Manager/SaveManager.h"
#include "Manager/WeatherEffectManager.h"
#include "Kismet/GameplayStatics.h" 
#include "UI/UIManager.h"
#include "EngineUtils.h"
#include "Engine/DataTable.h"
#include "Sound/SoundManager.h"


TWeakObjectPtr<ALevelManager> ALevelManager::Instance = nullptr;

ALevelManager::ALevelManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

// 1. 親クラスのBeginPlayを呼び出し
// 2. シングルトンインスタンスを設定
// 3. 各種マネージャーを初期化
// 4. テスト用のセーブデータを保存
void ALevelManager::BeginPlay()
{
    Super::BeginPlay();
    Instance = this;

    InitializeComponents();

    // テスト用セーブデータ
    FSaveData SaveData;
    SaveData.bCleared = true;
    SaveData.ClearRank = EStageRank::S;
    SaveData.difficultyRank = 2;
    SaveData.Title = TEXT("ステージ1");
    USaveManager::SaveStageData(TEXT("Stage1"), SaveData);
}

// 1. 初期化済みフラグをチェック
// 2. ColorManagerを生成して初期化
// 3. SoundManagerを取得してBGMを再生
// 4. UIManagerを生成して初期化
void ALevelManager::InitializeComponents()
{
    if (bInitialize)
        return;

    if (ColorManagerClass)
    {
        ColorManager = NewObject<UColorManager>(this, ColorManagerClass);
        ColorManager->Init();
    }

    SoundManager = GetComponentByClass<USoundManager>();
    if (SoundManager)
    {
        SoundManager->Init();
        SoundManager->PlaySound("BGM", "Default", true, SoundManager->GetBGMVolume());
    }

    if (UIManagerClass)
    {
        UIManager = NewObject<UUIManager>(this, UIManagerClass);
    }
    if (UIManager)
    {
        UIManager->Init(this);
    }

    bInitialize = true;
}

void ALevelManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// 1. インスタンスが有効かチェック
// 2. 無効な場合はワールドから検索
// 3. 見つからない場合は新規生成
// 4. 初期化処理を実行してインスタンスを返す
ALevelManager* ALevelManager::GetInstance(UObject* WorldContext)
{
    if (!Instance.IsValid())
    {
        UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
        if (!World)
            return nullptr;

        for (TActorIterator<ALevelManager> It(World); It; ++It)
        {
            Instance = *It;
            Instance->InitializeComponents();
            return *It;
        }

        ALevelManager* NewInstance = World->SpawnActor<ALevelManager>();
        Instance = NewInstance;
        Instance->InitializeComponents();
    }

    return Instance.Get();
}

// 1. SoundManagerの有効性を確認
// 2. サウンドを再生
void ALevelManager::PlaySound(FName Category, FName CueName)
{
    if (!SoundManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("SoundManager is null when trying to play sound."));
        return;
    }

    SoundManager->PlaySound(Category, CueName);
}

// SoundManagerをISoundableインターフェースとして返す
TScriptInterface<ISoundable> ALevelManager::GetSoundManager() const
{
    return TScriptInterface<ISoundable>(SoundManager);
}

// 1. SoundManagerの有効性を確認
// 2. BGMを停止してファンファーレを再生
void ALevelManager::HandlePlayerGoalReached()
{
    if (!SoundManager)
        return;

    SoundManager->StopBGM();
    SoundManager->PlaySound("SE", "Fanfare");
}

// 1. ゲームを一時停止
// 2. マウスカーソルを表示
// 3. 入力モードをUIのみに設定
// 4. ウィジェットにフォーカス
void ALevelManager::PauseGameAndShowUI(UUserWidget* FocusWidget)
{
    UGameplayStatics::SetGamePaused(GetWorld(), true);

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
        return;

    PC->bShowMouseCursor = true;

    FInputModeUIOnly InputMode;
    if (FocusWidget)
    {
        InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
    }
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    PC->SetInputMode(InputMode);
}