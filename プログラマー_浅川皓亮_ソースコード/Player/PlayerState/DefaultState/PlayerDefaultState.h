// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Player/PlayerStateComponent.h"
#include "PlayerDefaultState.generated.h"
class UMoveComponent;
class UBoxComponent;


/**
 * プレイヤーの通常状態（デフォルトステート）
 * 移動・ジャンプ・梯子遷移・着地アニメーション制御を行う
 */
UCLASS(Blueprintable)
class PACHIO_API UPlayerDefaultState : public UPlayerStateComponent
{
	GENERATED_BODY()

public:
	UPlayerDefaultState();

public:
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
	 * @param Value 入力値
	 * @return 処理成功時true
	 */
	virtual bool OnSkill(const FInputActionValue& Value) override;

	/**
	 * 移動入力処理
	 * @param Value 入力値
	 */
	virtual void Movement(const FInputActionValue& Value) override;

	/**
	 * ジャンプ処理
	 * @param jumpForce ジャンプに使用する力
	 * @return ジャンプできた場合true
	 */
	virtual bool Jump(float jumpForce) override;

public:
	/**
	 * 着地アニメーションが終了したかを取得
	 * @return 終了したフレームの場合true
	 */
	UFUNCTION(BlueprintCallable, Category = "Animation")
	bool HasLandingAnimationEnded() const { return bLandingAnimationJustEnded; }

	/**
	 * 着地アニメーション再生中かを取得
	 * @return 再生中の場合true
	 */
	UFUNCTION(BlueprintCallable, Category = "Animation")
	bool IsPlayingLandingAnimation() const { return bIsPlayingLandingAnimation; }

private:
	/**
	 * ジャンプ入力時に梯子へ遷移できるかを判定する
	 * @return 梯子遷移した場合true
	 */
	bool TryEnterLadderOnJump() const;

	/**
	 * 着地アニメーション終了時のコールバック
	 * @param Montage 再生されていたモンタージュ
	 * @param bInterrupted 中断されたか
	 */
	UFUNCTION()
	void OnLandingMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** 着地アニメーションを再生する */
	void PlayLandingAnimation();

private:
	/** ホールド可能判定用タイマー */
	FTimerHandle CheckHoldableHandle;

	/** 移動処理を司るコンポーネント */
	UPROPERTY()
	UMoveComponent* MoveComp = nullptr;

	/** 当たり判定用Boxコンポーネント */
	UPROPERTY()
	UBoxComponent* BoxComp = nullptr;

	/** 向き判定用の方向値 */
	float Direction = 0.f;

	/** 移動速度 */
	float MoveSpeed = 10.f;

	/** 初期回転設定済みか */
	bool InitialRotationSet = false;

	/** 前フレームでホールド可能だったか */
	bool bPrevCanHold = false;

	/** 前フレームで梯子に登れたか */
	bool bPrevCanClim = false;

	/** 着地アニメーション用モンタージュ */
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* LandingMontage = nullptr;

	/** 着地アニメーション再生中フラグ */
	bool bIsPlayingLandingAnimation = false;

	/** 初期回転値 */
	FRotator InitialRotation;

	/** 現在の入力方向 */
	FVector CurrentDirection = FVector::ZeroVector;

	/** 物理計算用コンポーネント */
	UPROPERTY()
	UPhysicsCalculator* Physics = nullptr;

	/** 梯子・地面判定用カプセル */
	UPROPERTY()
	UCapsuleComponent* HitBox = nullptr;

	/** 着地アニメーションが終了した瞬間のフラグ */
	bool bLandingAnimationJustEnded = false;


};
