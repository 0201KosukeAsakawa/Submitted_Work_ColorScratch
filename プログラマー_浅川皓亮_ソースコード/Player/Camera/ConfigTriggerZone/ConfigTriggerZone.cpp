// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/ConfigTriggerZone.h"
#include "Player/PlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/CameraHandlerComponent.h"

AConfigTriggerZone::AConfigTriggerZone()
{
	// Tickを有効化
	PrimaryActorTick.bCanEverTick = true;

	// Boxコリジョンを作成してルートに設定
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;

	// オーバーラップイベントをバインド
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AConfigTriggerZone::OnOverlapBegin);
}

void AConfigTriggerZone::BeginPlay()
{
	Super::BeginPlay();
}

// 1. アクターの有効性を確認
// 2. プレイヤーキャラクターにキャスト
// 3. CameraHandlerComponentを取得
// 4. 現在のパラメータと異なる場合のみカメラ設定を適用
void AConfigTriggerZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr)
		return;

	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (Player == nullptr)
		return;

	UCameraHandlerComponent* CameraHandle = Player->GetComponentByClass<UCameraHandlerComponent>();
	if (CameraHandle == nullptr)
		return;

	if (!CameraHandle->IsParameterMatch(GridSize, ZBaffer))
	{
		CameraHandle->ApplyCameraSettings(GridSize, ZBaffer, CameraViewType);
	}
}