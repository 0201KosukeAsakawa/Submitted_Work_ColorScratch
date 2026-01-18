// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Player/PlayerStateComponent.h"
#include "DeadPlayerState.generated.h"

class APlayerCharacter;

/**
 * プレイヤー死亡時のステート
 * 一定時間後にリスポーン処理を行い、通常ステートへ遷移する
 */
UCLASS()
class PACHIO_API UDeadPlayerState : public UPlayerStateComponent
{
	GENERATED_BODY()

public:
	/**
	 * ステートに入った際の初期化処理
	 * @param Owner ステートを所有するPawn
	 * @param World 現在のワールド
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
	 * @param Owner ステートを所有するPawn
	 * @return 正常に処理できた場合true
	 */
	virtual bool OnExit(APawn* Owner) override;

private:
	/** 死亡後の経過時間 */
	float ElapsedTime = 0.f;

	/** ステート遷移までの待機時間[秒] */
	float RespawnDelay = 2.0f;

	/** リスポーン処理を実行済みか */
	bool bIsRespawn = false;

	/** 操作対象のプレイヤーキャラクター */
	UPROPERTY()
	APlayerCharacter* Player;
};
