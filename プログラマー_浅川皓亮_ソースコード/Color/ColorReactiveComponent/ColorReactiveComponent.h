// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataContainer/EffectMatchResult.h"
#include "ColorReactiveComponent.generated.h"


class ANiagaraActor;
class UNiagaraSystem;
class UNiagaraComponent;
/**
 * HSL色空間の構造体
 */
struct FHSLColor
{
	float H; // 色相 (0.0~1.0)
	float S; // 彩度 (0.0~1.0)
	float L; // 輝度 (0.0~1.0)
};

/**
 * 色に反応して視覚効果を制御するコンポーネント
 * マテリアルの色変更、エフェクト再生、色の一致判定などを管理
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PACHIO_API UColorReactiveComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	/**
	 * コンストラクタ
	 * Niagaraシステムの読み込みを行う
	 */
	UColorReactiveComponent();

public:
	/**
	 * コンポーネントの初期化
	 * メッシュとマテリアルの取得、初期色の設定
	 * @param bIsColorVariable 色が可変かどうか
	 */
	virtual void Init(bool bIsColorVariable);

	/**
	 * 色、エフェクト、Niagaraの初期化
	 * @param FilterColor 設定する色
	 * @param NewEffect エフェクトタイプ
	 * @param NiagaraComponents Niagaraアクターの配列
	 */
	void InitColorEffectAndNiagara(const FLinearColor& FilterColor, EBuffEffect NewEffect, TArray<ANiagaraActor*> NiagaraComponents);

	/**
	 * マテリアルに色を適用
	 * @param InColor 適用する色
	 */
	void ApplyColorToMaterial(FLinearColor InColor);

	/**
	 * 色の一致をチェック（補色対応）
	 * @param MatchResult エフェクトマッチング結果
	 * @param FilterColor フィルター色
	 * @param bUseComplementaryColor 補色を使用するか
	 * @return 一致している場合true
	 */
	bool CheckColorMatch(FEffectMatchResult MatchResult, const FLinearColor& FilterColor, const bool bUseComplementaryColor = false);

	/**
	 * 現在の色とフィルター色の一致判定
	 * @param FilterColor フィルター色
	 * @param Tolerance 許容誤差（デフォルト0.08）
	 * @return 一致している場合true
	 */
	UFUNCTION(BlueprintCallable)
	virtual bool IsColorMatch(const FLinearColor& FilterColor, const float Tolerance = 0.08f) const;

	/**
	 * 2色の一致判定
	 * @param FilterColor フィルター色
	 * @param TargetColor 対象色
	 * @param Tolerance 許容誤差（デフォルト0.08）
	 * @return 一致している場合true
	 */
	bool IsColorMatch(const FLinearColor& FilterColor, const FLinearColor& TargetColor, const float Tolerance = 0.08f) const;

	/**
	 * 毎フレーム呼ばれる更新処理
	 */
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

	/**
	 * 補色を取得（パステル調に調整）
	 * @param InColor 元の色
	 * @return 補色
	 */
	FLinearColor GetComplementaryColor(const FLinearColor& InColor);

	/**
	 * 選択モードを設定
	 * @param bIsSelected 選択されているか
	 */
	void SetSelectMode(bool bIsSelected);

	/**
	 * 非表示状態かどうかを取得
	 * @return 非表示の場合true
	 */
	inline bool IsHidden() const { return bIsHidden; }

	/**
	 * エフェクトの有効/無効を切り替え
	 * @param bActivate 有効にするか
	 */
	void ActiveEffect(bool bActivate);

protected:
	/**
	 * Niagaraの表示状態を切り替え
	 * @param bVisible 表示するか
	 */
	void ToggleNiagaraActiveState(bool bVisible);

	/**
	 * 出現エフェクトを再生
	 */
	void PlayAppearEffect();

	/**
	 * 消滅エフェクトを再生
	 */
	void PlayDisappearEffect();

	/**
	 * Niagaraエフェクトをアクティブ化
	 * @param NiagaraSystem 再生するNiagaraシステム
	 */
	void ActiveNiagaraEffect(UNiagaraSystem* NiagaraSystem);

	/**
	 * 全てのアクティブなエフェクトを無効化
	 */
	void DeactivateAllEffects();

private:
	/**
	 * 色が一致したときの処理
	 * @param FilterColor フィルター色
	 * @return 処理が成功した場合true
	 */
	UFUNCTION(BlueprintCallable)
	virtual bool OnColorMatched(const FLinearColor& FilterColor);

	/**
	 * 色が一致しなかったときの処理
	 * @param FilterColor フィルター色
	 * @return 処理が成功した場合false
	 */
	virtual bool OnColorMismatched(const FLinearColor& FilterColor);

protected:
	/** 初期色を設定するか */
	UPROPERTY(EditAnywhere)
	bool bSetStartColor = true;

	/** 現在のエフェクトタイプ */
	EBuffEffect Effect;

	/** 現在の色 */
	UPROPERTY(EditAnywhere)
	FLinearColor CurrentColor;

	/** Niagaraアクターの配列 */
	UPROPERTY()
	TArray<ANiagaraActor*> Niagaras;

	/** 動的マテリアルインスタンス（メッシュ用） */
	UPROPERTY()
	UMaterialInstanceDynamic* DynMesh;

	/** FireflyBurst Niagaraシステム */
	UPROPERTY()
	UNiagaraSystem* FireflyBurstNiagaraSystem = nullptr;

	/** ParticlesOfLight Niagaraシステム */
	UPROPERTY()
	UNiagaraSystem* ParticlesOfLightNiagaraSystem = nullptr;

	/** LightCube Niagaraシステム */
	UPROPERTY()
	UNiagaraSystem* LightCubeNiagaraSystem = nullptr;

	/** アクティブなNiagaraコンポーネントの配列 */
	UPROPERTY()
	TArray<UNiagaraComponent*> ActiveNiagaraComponent;

	/** 選択されているか */
	bool bIsSelected = false;

	/** 非表示状態か */
	bool bIsHidden = false;
};