// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Color/ColorReactiveObject.h"
#include "LadderActor.generated.h"

class UBoxComponent;

/**
 * はしごアクター
 * プレイヤーが掴んで登れるはしごを表現する
 * 位置補正と回転補正の機能を持つ
 */
UCLASS()
class PACHIO_API ALadderActor : public AColorReactiveObject
{
	GENERATED_BODY()

public:
	ALadderActor();

protected:
	/**
	 * はしごアクターの初期化処理
	 */
	virtual void Init() override;

public:
	/**
	 * 毎フレーム呼ばれる処理
	 * @param DeltaTime フレーム間の経過時間
	 */
	virtual void Tick(float DeltaTime) override;

	/**
	 * はしごの最上部のワールド座標を取得する
	 * @return 最上部の座標
	 */
	FVector GetTopWorldPosition() const;

	/**
	 * はしごの最下部のワールド座標を取得する
	 * @return 最下部の座標
	 */
	FVector GetBottomWorldPosition() const;

	/**
	 * アクターをはしごに合わせて補正した位置を取得する
	 * はしごの左右どちらにいるかを判定し、適切なオフセットを適用
	 * @param OtherActor 補正対象のアクター
	 * @return 補正された座標
	 */
	FVector GetFixedPositionForActor(AActor* OtherActor) const;

	/**
	 * アクターがはしごに向くべき回転を取得する
	 * はしごの前後どちらにいるかを判定し、必要に応じて180度回転
	 * @param OtherActor 回転を計算する対象のアクター
	 * @return 補正された回転値
	 */
	FRotator GetRotationForActorToFaceThis(AActor* OtherActor) const;

private:
	// はしごからの左右オフセット距離
	UPROPERTY(EditAnywhere, Category = "Ladder Settings")
	float Offset;

	// はしごのボリューム(当たり判定)
	UPROPERTY(EditAnywhere, Category = "Ladder Settings")
	UBoxComponent* LadderVolume;
};