// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/CameraHandlerComponent.h"
#include "ConfigTriggerZone.generated.h"


class UBoxComponent;
/**
 * カメラ設定トリガーゾーン
 * プレイヤーが侵入するとカメラの設定(グリッドサイズ、Zバッファ、視点タイプ)を変更する
 */
UCLASS()
class PACHIO_API AConfigTriggerZone : public AActor
{
	GENERATED_BODY()

public:
	AConfigTriggerZone();

protected:
	virtual void BeginPlay() override;

private:
	/**
	 * オーバーラップ開始時の処理
	 * プレイヤーが侵入した際にカメラ設定を適用する
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

private:
	// トリガー判定用のBoxコリジョン
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	UBoxComponent* CollisionBox;

	// カメラの視点タイプ(エディタで設定)
	UPROPERTY(EditAnywhere, Category = "Camera Settings")
	ECameraViewType CameraViewType = ECameraViewType::GridView;

	// カメラのグリッドサイズ(エディタで設定)
	UPROPERTY(EditAnywhere, Category = "Camera Settings")
	FVector2D GridSize;

	// カメラのZバッファ値(エディタで設定)
	UPROPERTY(EditAnywhere, Category = "Camera Settings")
	float ZBaffer = 9000.0f;
};