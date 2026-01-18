// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RespawnComponent.generated.h"


/**
 * リスポーン機能を提供するコンポーネント
 * ゲーム開始時の位置を記録し、任意のタイミングで初期位置に戻す
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PACHIO_API URespawnComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	/**
	 * コンストラクタ
	 * Tick設定を有効化
	 */
	URespawnComponent();

protected:
	/**
	 * ゲーム開始時の初期化処理
	 * Ownerの初期位置を記録
	 */
	virtual void BeginPlay() override;

public:
	/**
	 * 毎フレーム呼ばれる更新処理
	 */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Ownerを初期位置にリスポーン
	 * ゲーム開始時に記録した位置にアクターを移動
	 */
	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void RespawnOwnerAtInitialLocation();

private:
	/** ゲーム開始時のOwnerの位置 */
	FVector InitialLocation;
};