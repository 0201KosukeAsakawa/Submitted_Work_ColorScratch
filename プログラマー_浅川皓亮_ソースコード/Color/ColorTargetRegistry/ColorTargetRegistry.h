// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DataContainer/EffectMatchResult.h"
#include "ColorTargetRegistry.generated.h"

class IColorReactiveInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnColorAppliedDelegate, EColorTargetType, Mode, FLinearColor, NewColor);

/**
 * 色の変化をゲーム内の様々なオブジェクトに伝達するレジストリクラス
 * ポストプロセスエフェクトやオブジェクトの色変更を一元管理する
 */
UCLASS(Blueprintable)
class PACHIO_API UColorTargetRegistry : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * 指定モードに応じて色を適用する
	 * @param NewColor 適用する色
	 * @param Mode 色の適用モード(ワールド全体/特定オブジェクト等)
	 * @param Effect エフェクトのマッチング結果情報
	 */
	void ApplyColor(FLinearColor NewColor, EColorTargetType Mode, FEffectMatchResult Effect);

	/**
	 * イベントIDに基づいて特定のターゲットに色を適用する
	 * @param EventID 発火するイベントの識別子
	 * @param NewColor 適用する色
	 * @param Effect エフェクトのマッチング結果情報
	 */
	void ColorEvent(FName EventID, FLinearColor NewColor, FEffectMatchResult Effect);

	/**
	 * 色変更の対象オブジェクトを設定する
	 * @param InInterface 色反応インターフェースを実装したオブジェクト
	 */
	void SetColorTarget(IColorReactiveInterface* InInterface);

	/**
	 * 現在の色変更対象をリセットする
	 */
	void ResetColorTarget();

	/**
	 * 色変更の通知を受け取るターゲットを登録する
	 * @param Mode 登録するターゲットのタイプ
	 * @param Target 登録するターゲットオブジェクト
	 */
	void RegisterTarget(EColorTargetType Mode, TScriptInterface<IColorReactiveInterface> Target);

	/**
	 * ポストプロセスエフェクトを初期化する
	 * ワールド内のPostProcessVolumeを検索してマテリアルを適用
	 */
	void InitializePostEffect();

	/**
	 * 現在のポストプロセスに適用されている色を取得する
	 * @return 現在のフィルター色(取得失敗時は黒)
	 */
	FLinearColor GetPostProcessColor() const;

	// 色が適用された際に発火するデリゲート
	UPROPERTY(BlueprintAssignable, Category = "Color")
	FOnColorAppliedDelegate OnColorApplied;

private:
	/**
	 * 登録されたターゲットに色変更を通知する
	 * @param Mode 通知対象のターゲットタイプ
	 * @param Color 適用する色
	 * @param Effect エフェクトのマッチング結果情報
	 */
	void NotifyTargets(EColorTargetType Mode, const FLinearColor& Color, FEffectMatchResult Effect);

private:
	// 各モードごとに登録された色反応オブジェクトのマップ
	UPROPERTY()
	TMap<EColorTargetType, FColorTargetInstanceArray> ColorResponseTargets;

	// 現在選択中の色変更対象オブジェクト
	UPROPERTY()
	TScriptInterface<IColorReactiveInterface> TargetObject;

	// ポストプロセスマテリアルの動的インスタンス
	UPROPERTY()
	UMaterialInstanceDynamic* PostProcessMID;

	// ポストプロセスに使用するマテリアル(エディタで設定)
	UPROPERTY(EditAnywhere, Category = "PostProcess")
	UMaterialInterface* PostProcessMaterial;
};