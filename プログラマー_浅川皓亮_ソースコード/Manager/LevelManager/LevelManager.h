#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataContainer/UIStruct.h"
#include "LevelManager.generated.h"

// 前方宣言（依存クラスの参照を軽量化）
class USoundManager;
class USaveManager;
class UColorManager;
class UUIManager;

class UWeatherEffectManager;
class UDataTable;
class ISoundable;
class UBlockDataContainer;
class UAttackDataContainer;
class UItemDataContainer;
/**
 * ステージ全体の生成・管理・シングルトンアクセスを提供するマネージャー
 * 各種マネージャー(サウンド、スコア、UI、カラー等)を統括する
 */
UCLASS()
class PACHIO_API ALevelManager : public AActor
{
    GENERATED_BODY()

public:
    ALevelManager();

protected:
    virtual void BeginPlay() override;

public:
    /**
     * 次のステージ名を取得する
     * @return 次のステージ名
     */
    UFUNCTION(BlueprintCallable, Category = "LevelManager")
    FString GetNextStageName() const { return NextStageName; }

    virtual void Tick(float DeltaTime) override;

    /**
     * プレイヤーがゴールに到達した際の処理
     * リザルト画面の表示、サウンド再生、ランク評価を実行
     */
    void HandlePlayerGoalReached();

    /**
     * サウンドの再生を指示する
     * @param Category サウンドカテゴリ(BGM/SE)
     * @param CueName 再生するサウンドの識別名
     */
    void PlaySound(FName Category, FName CueName);

    /**
     * サウンドマネージャーのインターフェースを取得する
     * @return ISoundableインターフェース
     */
    UFUNCTION(BlueprintCallable, Category = "LevelManager")
    TScriptInterface<ISoundable> GetSoundManager() const;

    /**
     * グローバルから取得できるLevelManagerのシングルトンインスタンス
     * @param WorldContext ワールドコンテキスト
     * @return LevelManagerのインスタンス
     */
    UFUNCTION(BlueprintCallable, Category = "LevelManager")
    static ALevelManager* GetInstance(UObject* WorldContext);

    /**
     * カラーマネージャーを取得する
     * @return ColorManagerへのポインタ
     */
    UFUNCTION(BlueprintCallable, Category = "LevelManager")
    inline UColorManager* GetColorManager() const { return ColorManager; }

    /**
     * UIマネージャーを取得する
     * @return UIManagerへのポインタ
     */
    UFUNCTION(BlueprintCallable, Category = "LevelManager")
    inline UUIManager* GetUIManager() const { return UIManager; }

private:
    /**
     * 各種コンポーネント(マネージャー)を初期化する
     */
    UFUNCTION(BlueprintCallable, Category = "LevelManager")
    void InitializeComponents();

    /**
     * ゲームを一時停止しUIにフォーカスを移す
     * @param FocusWidget フォーカスするウィジェット
     */
    void PauseGameAndShowUI(UUserWidget* FocusWidget);

private:
    // 初期化済みフラグ
    bool bInitialize;

    // 現在のステージ名
    UPROPERTY(EditAnywhere, Category = "Stage")
    FString StageName;

    // 次のステージ名
    UPROPERTY(EditAnywhere, Category = "Stage")
    FString NextStageName;

    // UIManagerのクラス参照(エディタで設定)
    UPROPERTY(EditAnywhere, Category = "Manager Classes")
    TSubclassOf<UUIManager> UIManagerClass;

    // ColorManagerのクラス参照(エディタで設定)
    UPROPERTY(EditAnywhere, Category = "Manager Classes")
    TSubclassOf<UColorManager> ColorManagerClass;

    // サウンドマネージャーのインスタンス
    UPROPERTY()
    TObjectPtr<USoundManager> SoundManager;

    // UIマネージャーのインスタンス
    UPROPERTY()
    TObjectPtr<UUIManager> UIManager;

    // カラーマネージャーのインスタンス
    UPROPERTY()
    TObjectPtr<UColorManager> ColorManager;

    // シングルトンアクセス用の静的インスタンス
    static TWeakObjectPtr<ALevelManager> Instance;
};