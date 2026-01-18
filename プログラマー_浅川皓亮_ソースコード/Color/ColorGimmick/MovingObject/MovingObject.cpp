// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/MovingObject.h"
#include "Sound/SoundManager.h"
#include "Components/BoxComponent.h"
#include "Components/Color/ColorConfigurator.h"
#include"Manager/LevelManager.h"

AMovingObject::AMovingObject()
{
	// Tickを有効化
	PrimaryActorTick.bCanEverTick = true;

	// 足元トリガーを作成してルートに設定
	FootTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("FootTrigger"));
	RootComponent = FootTrigger;

	// オーバーラップイベントを有効化
	FootTrigger->SetGenerateOverlapEvents(true);
	FootTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FootTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	FootTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// オーバーラップイベントをバインド
	FootTrigger->OnComponentBeginOverlap.AddDynamic(this, &AMovingObject::OnFootBeginOverlap);
	FootTrigger->OnComponentEndOverlap.AddDynamic(this, &AMovingObject::OnFootEndOverlap);
}

// 1. 親クラスの初期化を実行
// 2. 初期ターゲット位置をOffLocationに設定
void AMovingObject::Init()
{
	AColorReactiveObject::Init();
	TargetLocation = OffLocation;
}

// 1. 親クラスのTickを呼び出し
// 2. 移動中の場合:
//    - 経過時間を更新しAlpha値を計算
//    - 開始位置と目標位置を線形補間
//    - 移動量を計算して自身の位置を更新
//    - 上に乗っているアクターに同じ移動量を適用
//    - 子アクターに同じ移動量を適用
//    - Alpha値が1.0以上になったら移動完了
void AMovingObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsMoving)
	{
		ElapsedTime += DeltaTime;
		float Alpha = FMath::Clamp(ElapsedTime / MoveDuration, 0.0f, 1.0f);

		FVector NewLocation = FMath::Lerp(StartLocation, TargetLocation, Alpha);
		FVector DeltaMove = NewLocation - GetActorLocation();

		SetActorLocation(NewLocation);

		TArray<AActor*> Actors = AttachedActors;
		for (AActor* ActorOnTop : Actors)
		{
			if (ActorOnTop)
			{
				ActorOnTop->AddActorWorldOffset(DeltaMove, true);
			}
		}

		for (AActor* ChildActor : Child)
		{
			if (ChildActor)
			{
				ChildActor->AddActorWorldOffset(DeltaMove, true);
			}
		}

		if (Alpha >= 1.0f)
		{
			bIsMoving = false;
		}
	}
}

// 1. 親クラスのColorActionを実行
// 2. 移動開始位置を現在位置に設定
// 3. 経過時間をリセット
// 4. 色一致時はOffLocationへ、不一致時はOnLocationへ移動
// 5. 移動開始フラグを立てる
void AMovingObject::ColorAction(FLinearColor InColor, FEffectMatchResult Result)
{
	AColorReactiveObject::ColorAction(InColor, Result);

	StartLocation = GetActorLocation();
	ElapsedTime = 0.0f;

	if (ColorConfigurator->IsColorMatch())
	{
		TargetLocation = OffLocation;
	}
	else
	{
		TargetLocation = OnLocation;
	}

	bIsMoving = true;
}

// 1. Interactionタグを持つコンポーネントは除外
// 2. アクターの有効性を確認し、自分自身は除外
// 3. まだリストに含まれていなければ追加
// 4. ログに追加を記録
void AMovingObject::OnFootBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherComp && OtherComp->ComponentHasTag(TEXT("Interaction")))
		return;

	if (OtherActor && OtherActor != this && !AttachedActors.Contains(OtherActor))
	{
		AttachedActors.Add(OtherActor);
		UE_LOG(LogTemp, Log, TEXT("Added actor on top: %s"), *OtherActor->GetName());
	}
}

// 1. Interactionタグを持つコンポーネントは除外
// 2. アクターの有効性を確認
// 3. リストに含まれていれば削除
// 4. ログに削除を記録
void AMovingObject::OnFootEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherComp && OtherComp->ComponentHasTag(TEXT("Interaction")))
		return;

	if (OtherActor && AttachedActors.Contains(OtherActor))
	{
		AttachedActors.Remove(OtherActor);
		UE_LOG(LogTemp, Log, TEXT("Removed actor from top: %s"), *OtherActor->GetName());
	}
}