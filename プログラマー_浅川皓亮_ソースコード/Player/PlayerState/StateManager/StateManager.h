#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StateManager.generated.h"

// 前方宣言：プレイヤーの状態を管理する基底クラス
class UPlayerStateComponent;
class ACharacter;

/**
 * プレイヤーが取りうるステート種別
 */
UENUM(BlueprintType)
enum class EPlayerStateType : uint8
{
	Default UMETA(DisplayName = "Default"),
	Hold    UMETA(DisplayName = "Hold"),
	Climb   UMETA(DisplayName = "Climb"),
	Dead    UMETA(DisplayName = "Dead"),
	// 必要に応じて追加
};

/**
 * プレイヤーの状態（ステート）を管理・切り替えするコンポーネント
 * 各ステートは UPlayerStateComponent を継承したクラスとして管理される
 */
UCLASS(Blueprintable)
class PACHIO_API UStateManager : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * コンストラクタ
	 * コンポーネントの初期設定を行う
	 */
	UStateManager();

	/**
	 * ステートマネージャの初期化処理
	 * @param Owner ステートを管理するPawn
	 * @param World 現在のワールド
	 */
	void Init(APawn* Owner, UWorld* World);

	/**
	 * 毎フレーム呼ばれる更新処理
	 * @param DeltaTime 前フレームからの経過時間
	 */
	void Update(float DeltaTime);

	/**
	 * ステートを切り替える
	 * @param NextStateTag 遷移先ステートの種別
	 * @return 新しく生成されたステート
	 */
	UPlayerStateComponent* ChangeState(EPlayerStateType NextStateTag);

	/**
	 * 現在のステートが指定したステートかを判定
	 * @param StateTag 判定対象のステート種別
	 * @return 一致している場合true
	 */
	UFUNCTION(BlueprintCallable)
	bool IsStateMatch(EPlayerStateType StateTag);

	/**
	 * 現在アクティブなステートを取得
	 * @return 現在のステート
	 */
	UFUNCTION(BlueprintCallable)
	inline UPlayerStateComponent* GetCurrentState() const
	{
		return CurrentState;
	}

private:
	/** ステート種別とステートクラスの対応マップ */
	UPROPERTY(EditAnywhere)
	TMap<EPlayerStateType, TSubclassOf<UPlayerStateComponent>> StateClassMap;

	/** ステートの所有者となるPawn */
	UPROPERTY()
	APawn* mOwner = nullptr;

	/** 現在アクティブなステート */
	UPROPERTY()
	UPlayerStateComponent* CurrentState = nullptr;

	/** ワールド参照 */
	UPROPERTY()
	UWorld* pWorld = nullptr;
};
