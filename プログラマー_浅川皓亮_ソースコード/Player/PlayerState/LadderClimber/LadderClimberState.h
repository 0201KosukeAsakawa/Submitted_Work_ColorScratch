// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Player/PlayerStateComponent.h"
#include "LadderClimberState.generated.h"

class ALadderActor;
class UMoveComponent;
class UColorConfigurator;

/**
 * 梯子昇降中のプレイヤーステート
 * 梯子に固定された状態で上下移動を行い、条件に応じて通常ステートへ遷移する
 */
UCLASS()
class PACHIO_API ULadderClimberState : public UPlayerStateComponent
{
	GENERATED_BODY()

public:
	/**
	 * 対象となる梯子を設定する
	 * @param LadderActor 昇降対象の梯子アクター
	 */
	void SetTargetLadder(ALadderActor* LadderActor);

	/**
	 * 梯子昇降中の移動処理
	 * @param Value 入力値
	 */
	void Movement(const FInputActionValue& Value) override;

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
	 * スキル入力時の処理
	 * @param Input 入力値
	 * @return 処理成功時true
	 */
	virtual bool OnSkill(const FInputActionValue& Input) override;

private:
	/** 現在昇降中の梯子 */
	UPROPERTY()
	ALadderActor* Ladder;

	/** 梯子移動用の移動コンポーネント */
	UPROPERTY()
	UMoveComponent* MoveComp;

	/** 梯子側の色制御コンポーネント */
	UPROPERTY()
	UColorConfigurator* TargetComp;

	/** 梯子に対して固定される位置 */
	FVector FixedPosition;
};
