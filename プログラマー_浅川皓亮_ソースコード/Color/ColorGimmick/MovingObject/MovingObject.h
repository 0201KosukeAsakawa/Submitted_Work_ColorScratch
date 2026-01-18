// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Color/ColorReactiveObject.h"
#include "DataContainer/EffectMatchResult.h"
#include "MovingObject.generated.h"

class UBoxComponent;

/**
 * 色に反応して移動するオブジェクト
 * 色の一致・不一致に応じてOnLocationとOffLocationを切り替える
 * 上に乗っているアクターや子オブジェクトも一緒に移動する
 */
UCLASS()
class PACHIO_API AMovingObject : public AColorReactiveObject
{
	GENERATED_BODY()

public:
	AMovingObject();

	/**
	 * 移動オブジェクトの初期化処理
	 */
	virtual void Init() override;

	/**
	 * 毎フレーム呼ばれる処理
	 * 移動中のアクターと子オブジェクトを追従させる
	 * @param DeltaTime フレーム間の経過時間
	 */
	virtual void Tick(float DeltaTime) override;

private:
	/**
	 * 色反応処理
	 * 色一致時はOffLocation、不一致時はOnLocationへ移動を開始
	 * @param InColor 適用される色
	 * @param Result エフェクトマッチング結果
	 */
	virtual void ColorAction(const FLinearColor InColor, FEffectMatchResult Result) override;

	/**
	 * 足元トリガーオーバーラップ開始時の処理
	 * オブジェクト上にアクターが乗った際に呼ばれる
	 * @param OverlappedComp オーバーラップしたコンポーネント
	 * @param OtherActor オーバーラップした相手のアクター
	 * @param OtherComp 相手のコンポーネント
	 * @param OtherBodyIndex 相手のボディインデックス
	 * @param bFromSweep スイープによるオーバーラップか
	 * @param SweepResult スイープ結果
	 */
	UFUNCTION()
	void OnFootBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * 足元トリガーオーバーラップ終了時の処理
	 * オブジェクト上からアクターが離れた際に呼ばれる
	 * @param OverlappedComp オーバーラップしていたコンポーネント
	 * @param OtherActor オーバーラップしていた相手のアクター
	 * @param OtherComp 相手のコンポーネント
	 * @param OtherBodyIndex 相手のボディインデックス
	 */
	UFUNCTION()
	void OnFootEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	// 移動中フラグ
	bool bIsMoving = false;

	// 色不一致時の目標位置(エディタで設定)
	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FVector OffLocation;

	// 色一致時の目標位置(エディタで設定)
	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FVector OnLocation;

	// 現在の目標位置
	FVector TargetLocation;

	// 一緒に移動する子アクターのリスト(エディタで設定)
	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	TArray<AActor*> Child;

	// 足元の判定用トリガー
	UPROPERTY(EditAnywhere, Category = "Collision")
	UBoxComponent* FootTrigger;

	// 上に乗っているアクターのリスト
	TArray<AActor*> AttachedActors;

	// 移動開始時の位置
	FVector StartLocation;

	// 移動にかける時間(秒)
	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float MoveDuration = 1.0f;

	// 移動開始からの経過時間
	float ElapsedTime = 0.0f;
};