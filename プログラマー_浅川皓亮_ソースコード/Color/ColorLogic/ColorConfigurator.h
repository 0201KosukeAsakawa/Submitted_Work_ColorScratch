#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataContainer/EffectMatchResult.h"
#include "ColorConfigurator.generated.h"

class ALevelManager;
class ANiagaraActor;
class UBeatScalerComponent;
class UColorReactiveComponent;
class UColorManager;



/**
 * 色の設定と管理を行うコンポーネント
 * オブジェクトの色変更、色一致判定、エフェクト連動などを担当
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PACHIO_API UColorConfigurator : public UActorComponent
{
	GENERATED_BODY()
public:
	UColorConfigurator();

	// =======================
	// 初期化処理
	// =======================

	/**
	 * コンポーネントの初期化
	 * ColorLogicの初期化、ColorManagerへの登録、マテリアル設定を実行
	 */
	virtual void Init();

	/**
	 * カラーロジックの初期化
	 * ColorReactiveComponentの生成と設定
	 */
	virtual void InitializeColorLogic();

	/**
	 * ColorManagerへの登録
	 * ColorTargetTypeに基づいて登録
	 */
	virtual void RegisterToColorManager();

	/**
	 * マテリアルの設定
	 * 初期色の設定とカスタムデプスの有効化
	 */
	virtual void SetupMaterial();

	// =======================
	// 色に応じたアクション
	// =======================

	/**
	 * 色に応じたアクション実行
	 * @param InColor 適用する色
	 * @param result エフェクトマッチング結果
	 */
	virtual void ColorAction(FLinearColor InColor, FEffectMatchResult result);

	// =======================
	// 色操作
	// =======================

	/**
	 * 色を初期状態にリセット
	 * @param result エフェクトマッチング結果
	 */
	virtual void ResetColor(FEffectMatchResult result);

	/**
	 * 新しい色を設定
	 * @param InColor 設定する色
	 * @param result エフェクトマッチング結果
	 */
	virtual void SetColor(FLinearColor InColor, FEffectMatchResult result);

	/**
	 * マテリアルに色を適用
	 * @param InColor 適用する色
	 */
	virtual void ApplyColorToMaterial(FLinearColor InColor);

	/**
	 * 現在の色を設定（マテリアル更新なし）
	 * @param InColor 設定する色
	 */
	void SetCurrentColor(FLinearColor InColor);

	/**
	 * 色一致状態を設定
	 * @param bInColorMuch 一致しているか
	 */
	void SetColorMatch(bool bInColorMuch);

	/**
	 * 選択モードを設定
	 * @param bIsSelect 選択されているか
	 */
	void SetSelectMode(bool bIsSelect);

	/**
	 * 色変更のロック状態を変更
	 * @param bLock ロックするか
	 */
	void ChangeLock(bool bLock) { bColorVariable = bLock; }

	// =======================
	// 色判定・一致確認
	// =======================

	/**
	 * 初期色から変更されているか確認
	 * @return 変更されている場合true
	 */
	virtual bool IsColorChange() const;

	/**
	 * 指定色から変更されているか確認
	 * @param InColor 比較する色
	 * @return 変更されている場合true
	 */
	virtual bool IsColorChange(FLinearColor InColor) const;

	/**
	 * 現在の色一致状態を取得
	 * @return 一致している場合true
	 */
	bool IsColorMatch() const;

	/**
	 * 2色の一致判定
	 * @param FilterColor フィルター色
	 * @param TargetColor 対象色
	 * @param Tolerance 許容誤差（デフォルト0.08）
	 * @return 一致している場合true
	 */
	bool IsColorMatch(const FLinearColor& FilterColor, const FLinearColor& TargetColor, float Tolerance = 0.08f) const;

	/**
	 * フィルター色との一致判定
	 * @param FilterColor フィルター色
	 * @param Tolerance 許容誤差（デフォルト0.08）
	 * @return 一致している場合true
	 */
	bool IsColorMatch(const FLinearColor& FilterColor, float Tolerance = 0.08f) const;

	/**
	 * 色のマッチング確認（補色対応）
	 * @param result エフェクトマッチング結果
	 * @param FilterColor フィルター色
	 * @param bUseComplementaryColor 補色を使用するか
	 * @return マッチしている場合true
	 */
	bool CheckColorMatch(FEffectMatchResult result, const FLinearColor& FilterColor, bool bUseComplementaryColor = false) const;

	/**
	 * 色変更可能か確認
	 * @return 変更可能な場合true
	 */
	bool IsChangeable() const;

	/**
	 * 非表示状態か確認
	 * @return 非表示の場合true
	 */
	bool IsHidden() const;

	// =======================
	// Getter
	// =======================

	/**
	 * 色変更が有効か取得
	 * @return 有効な場合true
	 */
	bool IsColorModifiable() const { return bSetColor; }

	/**
	 * 現在の色を取得
	 * @return 現在の色
	 */
	FLinearColor GetCurrentColor() const { return CurrentColor; }

	/**
	 * カラーイベントIDを取得
	 * @return イベントID
	 */
	FName GetColorEventID() const { return EventID; }

	/**
	 * カラーターゲットタイプを取得
	 * @return ターゲットタイプ
	 */
	EColorTargetType GetColorTargetType() const { return ColorTargetType; }

private:
	// =======================
	// 状態取得（内部用）
	// =======================

	/**
	 * StaticMeshコンポーネントを取得
	 * @return StaticMeshコンポーネント
	 */
	USkeletalMeshComponent* GetStaticMesh() const;

	/**
	 * LevelManagerを取得
	 * @return LevelManager
	 */
	ALevelManager* GetLevelManager() const;

	/**
	 * ColorManagerを取得
	 * @return ColorManager
	 */
	UColorManager* GetColorManager() const;

protected:
	// --- Component References ---
	UPROPERTY(EditAnywhere, Category = "Reactive")
	TSubclassOf<UColorReactiveComponent> ReactiveComponentClass;

	UPROPERTY()
	UColorReactiveComponent* ColorReactiveComponent;

	// --- Color Data ---
	FLinearColor StartColor;

	UPROPERTY(VisibleAnywhere, Category = "Color")
	FLinearColor CurrentColor;

	// --- Settings ---
	UPROPERTY(EditAnywhere, Category = "Color")
	EColorTargetType ColorTargetType;

	UPROPERTY(EditAnywhere, Category = "Color")
	FName EventID;

	UPROPERTY(EditAnywhere, Category = "Color")
	bool bColorVariable = false;

	UPROPERTY(EditAnywhere, Category = "Color")
	bool bColorChangeable = true;

	UPROPERTY(EditAnywhere, Category = "Color")
	bool bSetColor = true;

	UPROPERTY(EditAnywhere, Category = "Color")
	bool bPlayColorAction = true;

	UPROPERTY(EditAnywhere, Category = "Color")
	bool bUseComplementaryColor = false;

	UPROPERTY(VisibleAnywhere, Category = "Color")
	bool bColorMuch = false;

	UPROPERTY(EditAnywhere, Category = "Color")
	EBuffEffect Effect;

	UPROPERTY(EditAnywhere)
	TArray<ANiagaraActor*> Niagaras;

private:
	bool bIsSelected = false;

	UPROPERTY(EditAnywhere)
	bool bIsPlayBeat = true;
};