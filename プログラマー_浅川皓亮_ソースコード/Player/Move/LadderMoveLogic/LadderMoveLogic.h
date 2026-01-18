// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Interface/MoveLogic.h"
#include "LadderMoveLogic.generated.h"

/**
 * はしご移動ロジック
 * はしごに掴まっている状態での上下移動を制御する
 */
UCLASS()
class PACHIO_API ULadderMoveLogic : public UObject, public IMoveLogic
{
	GENERATED_BODY()

private:
	/**
	 * はしご上での移動ベクトルを計算する
	 * @param DeltaTime フレーム間の経過時間
	 * @param Owner 移動するアクター
	 * @param Value 入力値(X軸で上下移動)
	 * @return 移動方向ベクトル
	 */
	FVector Movement(float DeltaTime, AActor* Owner, const FInputActionValue& Value = FInputActionValue()) override;

	/**
	 * 移動ロジックを初期化する
	 * @param Speed 移動速度
	 * @param Direction 初期移動方向(未使用)
	 */
	void Init(float Speed, const FVector Direction) override;
};