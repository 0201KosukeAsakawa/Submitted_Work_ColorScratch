// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeadZone.generated.h"

class UBoxComponent;

/**
 * デッドゾーン(即死エリア)
 * プレイヤーが侵入するとリスポーンまたはDead状態に遷移する
 */
UCLASS()
class PACHIO_API ADeadZone : public AActor
{
	GENERATED_BODY()

public:
	ADeadZone();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	/**
	 * オーバーラップ時の処理
	 * RespawnComponentがあればリスポーン、なければDead状態に遷移
	 * @param OverlappedComponent オーバーラップしたコンポーネント
	 * @param OtherActor オーバーラップした相手のアクター
	 * @param OtherComponent 相手のコンポーネント
	 * @param OtherBodyIndex 相手のボディインデックス
	 * @param bFromSweep スイープによるオーバーラップか
	 * @param SweepResult スイープ結果
	 */
	UFUNCTION()
	void OverlapDead(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

protected:
	// デッドゾーンの判定用Boxコリジョン
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	UBoxComponent* DeadArea;
};