// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Color/ColorReactiveObject.h"
#include "DataContainer/EffectMatchResult.h"
#include "ColorReactiveBeltConveyor.generated.h"

class UBoxComponent;
class UPhysicsCalculator;

/**
 * 色によって挙動が変化するベルトコンベアクラス
 * 色の一致・不一致に応じて進行方向や停止を切り替える
 * オーバーラップしたアクターに物理的な力を加える
 */
UCLASS()
class PACHIO_API AColorReactiveBeltConveyor : public AColorReactiveObject
{
	GENERATED_BODY()

public:
	AColorReactiveBeltConveyor();

	/**
	 * ベルトコンベアの初期化処理
	 * 親クラスの初期化とビート検出のバインドを実行
	 */
	virtual void Init() override;

	/**
	 * 毎フレーム呼ばれる処理
	 * ベルト上のオブジェクトに力を加える
	 * @param DeltaTime フレーム間の経過時間
	 */
	virtual void Tick(float DeltaTime) override;

private:
	/**
	 * 色反応処理
	 * 色の一致状態に応じてベルトの進行方向を変更する
	 * @param InColor 適用される色
	 * @param Result エフェクトマッチング結果
	 */
	virtual void ColorAction(const FLinearColor InColor, FEffectMatchResult Result) override;

	/**
	 * オーバーラップ開始時の処理
	 * ベルト上にオブジェクトが乗った際に呼ばれる
	 * @param OverlappedComp オーバーラップしたコンポーネント
	 * @param OtherActor オーバーラップした相手のアクター
	 * @param OtherComp 相手のコンポーネント
	 * @param OtherBodyIndex 相手のボディインデックス
	 * @param bFromSweep スイープによるオーバーラップか
	 * @param SweepResult スイープ結果
	 */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * オーバーラップ終了時の処理
	 * ベルト上からオブジェクトが離れた際に呼ばれる
	 * @param OverlappedComp オーバーラップしていたコンポーネント
	 * @param OtherActor オーバーラップしていた相手のアクター
	 * @param OtherComp 相手のコンポーネント
	 * @param OtherBodyIndex 相手のボディインデックス
	 */
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	// ベルトの当たり判定用Boxコリジョン
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	UBoxComponent* BoxComponent;

	// ベルトの基準進行方向(エディタで設定)
	UPROPERTY(EditAnywhere, Category = "Belt Settings")
	FVector direction;

	// ベルトの推進力の初期値(エディタで設定)
	UPROPERTY(EditAnywhere, Category = "Belt Settings")
	float DefaultPower = 0.0f;

	// 現在の推進力
	float CurrentPower = 0.0f;

	// 現在の進行方向(色に応じて動的に変化)
	FVector CurrentDirection;

	// ベルト上に存在するオブジェクトのリスト
	UPROPERTY()
	TArray<UPhysicsCalculator*> hitObject;

	// 色一致時に逆方向に動作するか
	UPROPERTY(EditAnywhere, Category = "Belt Settings")
	bool IsRevers = false;

	// ローカル座標系でオフセットを使用するか
	UPROPERTY(EditAnywhere, Category = "Belt Settings")
	bool bUseLocalOffset = false;

	// 最も近いオブジェクトのみに力を加えるか
	UPROPERTY(EditAnywhere, Category = "Belt Settings")
	bool bOnlyClosest = false;
};