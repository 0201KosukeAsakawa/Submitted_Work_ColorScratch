// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/DeadZone.h"
#include "Interface/StateControllable.h"
#include "Components/BoxComponent.h"
#include "Components/RespawnComponent.h"
#include "GameFramework/Actor.h"

ADeadZone::ADeadZone()
{
	// Tickを有効化
	PrimaryActorTick.bCanEverTick = true;

	// Boxコンポーネントを生成してルートに設定
	DeadArea = CreateDefaultSubobject<UBoxComponent>(TEXT("DeadArea"));
	RootComponent = DeadArea;

	// コリジョン設定: 物理処理は行わず、オーバーラップ検知のみ
	DeadArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DeadArea->SetCollisionResponseToAllChannels(ECR_Ignore);
	DeadArea->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// オーバーラップイベントを登録
	DeadArea->OnComponentBeginOverlap.AddDynamic(this, &ADeadZone::OverlapDead);
}

void ADeadZone::BeginPlay()
{
	Super::BeginPlay();
}

void ADeadZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// 1. アクターの有効性を確認
// 2. RespawnComponentを持っている場合はリスポーン処理を実行
// 3. RespawnComponentがない場合はDead状態に遷移
void ADeadZone::OverlapDead(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor)
		return;

	if (URespawnComponent* RespawnComp = OtherActor->FindComponentByClass<URespawnComponent>())
	{
		RespawnComp->RespawnOwnerAtInitialLocation();
		return;
	}

	if (IStateControllable* StateInterface = Cast<IStateControllable>(OtherActor))
	{
		StateInterface->ChangeState(EPlayerStateType::Dead);
	}
}