// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DataContainer/EffectMatchResult.h"
#include "EffectColorMatcher.generated.h"

/**
 * 入力された色からバフエフェクトタイプを判定するマッチャークラス
 * 色相(Hue)と明度(Value)を考慮して最も近いエフェクトを特定する
 */
UCLASS()
class PACHIO_API UEffectColorMatcher : public UObject
{
	GENERATED_BODY()

public:
	UEffectColorMatcher();

	/**
	 * 入力色に最も近いエフェクトタイプを色相ベースで判定する
	 * @param InputColor 判定対象の色
	 * @return マッチング結果(エフェクトタイプ、距離、強度比率)
	 */
	FEffectMatchResult GetClosestEffectByHue(const FLinearColor& InputColor);

	/**
	 * 2色間の色相角度の差分を計算する(0〜180度)
	 * @param ColorA 比較する色1
	 * @param ColorB 比較する色2
	 * @return 色相角度の差(度数法)
	 */
	float GetHueAngleDistance(const FLinearColor& ColorA, const FLinearColor& ColorB);

	/**
	 * 指定されたエフェクトタイプに対応する色を取得する
	 * @param Effect エフェクトタイプ
	 * @return 対応する色(見つからない場合は白)
	 */
	FLinearColor GetEffectColor(EBuffEffect Effect) const;

private:
	// 各エフェクトタイプに対応する基準色のマップ
	TMap<EBuffEffect, FLinearColor> EffectColorMap;
};