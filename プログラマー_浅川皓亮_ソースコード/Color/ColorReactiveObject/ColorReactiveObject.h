#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/ColorFilterInterface.h"
#include "Interface/ColorReactionConfigInterface.h"
#include "DataContainer/EffectMatchResult.h"
#include "ColorReactiveObject.generated.h"

class UColorReactiveComponent;
class UBeatScalerComponent;
class UColorConfigurator;

/**
 * 色に反応するアクターの基底クラス
 * 色の変化に応じてアクションを実行し、ColorConfiguratorを使用して色管理を行う
 */
UCLASS()
class PACHIO_API AColorReactiveObject : public AActor, public IColorReactiveInterface, public IColorReactionConfigInterface
{
	GENERATED_BODY()

public:
	AColorReactiveObject();

protected:
	virtual void BeginPlay() override;

public:
	/**
	 * 色アクション実行時の処理
	 * @param InColor 適用される色
	 * @param Result エフェクトマッチング結果
	 */
	virtual void ColorAction(FLinearColor InColor, FEffectMatchResult Result) override;

	/**
	 * 色を設定する
	 * @param NewColor 設定する色
	 * @param Result エフェクトマッチング結果
	 */
	virtual void SetColor(FLinearColor NewColor, FEffectMatchResult Result) override;

	/**
	 * 選択モードを設定する
	 * @param bIsSelected 選択状態か
	 */
	virtual void SetSelectMode(bool bIsSelected) override;

	/**
	 * ロック状態を変更する
	 * @param bLock ロックするか
	 */
	virtual void ChangeLock(bool bLock) override;

	/**
	 * 色が変更されているかを取得する
	 * @return 色が変更されている場合true
	 */
	virtual bool IsColorChange() const override;

	/**
	 * 色変更が可能かを取得する
	 * @return 色変更可能な場合true
	 */
	virtual bool IsChangeable() const override;

	/**
	 * 色の編集が可能かを取得する
	 * @return 色編集可能な場合true
	 */
	virtual bool IsColorModifiable() const override;

	/**
	 * 色が一致しているかを取得する
	 * @return 色が一致している場合true
	 */
	virtual bool IsColorMatch() const override;

	/**
	 * カラーイベントIDを取得する
	 * @return イベントID
	 */
	FName GetColorEventID() const override;

protected:
	/**
	 * 色反応オブジェクトの初期化処理
	 */
	virtual void Init();

	/**
	 * 色反応ロジックの初期化
	 */
	virtual void InitializeColorLogic();

	/**
	 * ColorManagerへの登録
	 */
	virtual void RegisterToColorManager();

	/**
	 * マテリアルのセットアップ
	 */
	virtual void SetupMaterial();

	/**
	 * マテリアルに色を適用する
	 * @param InColor 適用する色
	 */
	virtual void ApplyColorToMaterial(FLinearColor InColor);

protected:
	// 色設定を管理するコンフィギュレーター
	UPROPERTY(EditAnywhere, Category = "Color")
	UColorConfigurator* ColorConfigurator;
};