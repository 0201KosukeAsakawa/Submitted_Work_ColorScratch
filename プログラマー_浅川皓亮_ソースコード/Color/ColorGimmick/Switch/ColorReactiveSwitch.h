// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Color/ColorReactiveObject.h"
#include "DataContainer/EffectMatchResult.h"
#include "ColorReactiveSwitch.generated.h"

class UBoxComponent;

/**
 * 色に反応するスイッチアクター
 * 2つの異なる色(メイン色とセカンド色)に反応してイベントを発火する
 */
UCLASS()
class PACHIO_API AColorReactiveSwitch : public AColorReactiveObject
{
	GENERATED_BODY()

public:
	AColorReactiveSwitch();

	/**
	 * スイッチの初期化処理
	 * セカンド色の取得を実行
	 */
	virtual void Init() override;

private:
	/**
	 * 色反応処理
	 * メイン色またはセカンド色と一致した場合にカラーイベントを発火
	 * @param InColor 適用される色
	 * @param Result エフェクトマッチング結果
	 */
	virtual void ColorAction(const FLinearColor InColor, FEffectMatchResult Result) override;

private:
	// スイッチの当たり判定用Boxコリジョン
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	UBoxComponent* BoxComponent;

	// セカンド反応色のエフェクトタイプ(エディタで設定)
	UPROPERTY(EditAnywhere, Category = "Color Settings")
	EBuffEffect Second;

	// セカンド反応色(実行時に計算)
	FLinearColor SecondColor;
};