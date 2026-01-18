// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Player/PlayerStateComponent.h"
#include "PlayerHoldState.generated.h"

class UMoveComponent;
class UColorReactiveComponent;

/**
 * プレイヤーがオブジェクトを掴んでいる状態
 * オブジェクトとの距離維持、移動同期、解除判定を行う
 */
UCLASS()
class PACHIO_API UPlayerHoldState : public UPlayerStateComponent
{
	GENERATED_BODY()

public:
	UPlayerHoldState();

	/**
	 * 掴む対象の初期設定
	 * @param Target 掴むアクター
	 * @param bGrabDirection 掴んだ向き（true: 正方向 / false: 逆方向）
	 */
	void SetUp(AActor* Target, bool bGrabDirection);

private:
	/**
	 * ステートに入った際の初期化処理
	 * @param Owner ステート所有者
	 * @param World ワールド
	 * @return 正常に処理できた場合true
	 */
	virtual bool OnEnter(APawn* Owner, UWorld* World) override;

	/**
	 * 毎フレーム呼ばれる更新処理
	 * @param DeltaTime 前フレームからの経過時間
	 * @return ステート継続中の場合true
	 */
	virtual bool OnUpdate(float DeltaTime) override;

	/**
	 * ステート終了時の後処理
	 * @param Owner ステート所有者
	 * @return 正常に処理できた場合true
	 */
	virtual bool OnExit(APawn* Owner) override;

	/**
	 * スキル入力時の処理（掴み解除）
	 * @param Value 入力値
	 * @return 処理成功時true
	 */
	virtual bool OnSkill(const FInputActionValue& Value) override;

	/**
	 * 移動入力処理
	 * プレイヤーと掴み対象を同時に移動させる
	 * @param Value 入力値
	 */
	virtual void Movement(const FInputActionValue& Value) override;

private:
	/** 掴んでいる対象アクター */
	UPROPERTY()
	AActor* HoldTarget = nullptr;

	/** 移動処理用コンポーネント */
	UPROPERTY()
	UMoveComponent* MoveComp = nullptr;

	/** 色反応コンポーネント（非表示時の解除判定用） */
	UPROPERTY()
	UColorReactiveComponent* TargetComp = nullptr;

	/** 掴み開始時のプレイヤーとの距離 */
	float InitialHoldDistance = 0.f;

	/** 掴んだ向き（1 or -1） */
	int32 GrabDirection = 1;
};