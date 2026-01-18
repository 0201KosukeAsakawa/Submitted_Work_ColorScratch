// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/MoveLogic.h"
#include "MoveComponent.generated.h"


/**
 * 移動処理を担当するアクターコンポーネント
 * 実際の移動計算は IMoveLogic に委譲し、本クラスは仲介役を担う
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PACHIO_API UMoveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * コンポーネントのデフォルトコンストラクタ
	 */
	UMoveComponent();

	/**
	 * 移動ロジックの初期化
	 * @param MoveLogic 使用する移動ロジック
	 * @param Speed 移動速度
	 * @param NewDirection 初期移動方向
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Init(
		TScriptInterface<IMoveLogic> MoveLogic,
		const float Speed = 1.f,
		const FVector NewDirection = FVector(0, 0, 0)
	);

	/**
	 * 移動処理を実行する
	 * @param DeltaTime 前フレームからの経過時間
	 * @param Owner 移動対象のアクター
	 * @param Value 入力値
	 * @return 計算された移動方向ベクトル
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	FVector Movement(
		float DeltaTime,
		AActor* Owner,
		const FInputActionValue& Value = FInputActionValue()
	);

	/**
	 * 移動方向を設定する
	 * @param NewDirection 新しい移動方向
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetDirection(FVector NewDirection);

public:
	/**
	 * 移動ロジックを差し替える
	 * @param Logic 新しい移動ロジック
	 * @return 設定に成功した場合true
	 */
	bool SetMoveLogic(TScriptInterface<IMoveLogic> Logic);

private:
	/** 移動速度 */
	float Speed;

	/** 実際の移動処理を行うロジック */
	UPROPERTY(EditAnywhere)
	TScriptInterface<IMoveLogic> MoveLogic;
};
